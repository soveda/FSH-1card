#include "ComputerCard.h"

#include <stdint.h>

class F1shnet : public ComputerCard
{
public:
    void ProcessSample() override
    {
        int32_t input = AudioIn1();
        int32_t auxAudio = AudioIn2();
        int32_t absInput = input >= 0 ? input : -input;
        int32_t absAux = auxAudio >= 0 ? auxAudio : -auxAudio;

        int32_t rawMainKnob = KnobVal(Knob::Main);
        int32_t rawXKnob = KnobVal(Knob::X);
        int32_t rawYKnob = KnobVal(Knob::Y);

        int32_t cv1 = CVIn1();
        int32_t cv2 = CVIn2();

        bool clockIn = PulseIn1();
        bool shGateIn = PulseIn2();
        bool clockEdge = clockIn && !lastClockIn_;
        lastClockIn_ = clockIn;

        Switch switchPos = SwitchVal();
        bool altPage = (switchPos == Switch::Up);
        bool shGesture = (switchPos == Switch::Down) || shGateIn;

        if (!controlsInitialised_)
        {
            pageMain_[0] = rawMainKnob;
            pageX_[0] = rawXKnob;
            pageY_[0] = rawYKnob;
            pageMain_[1] = rawMainKnob;
            pageX_[1] = rawXKnob;
            pageY_[1] = rawYKnob;
            controlsInitialised_ = true;
            previousAltPage_ = altPage;
        }

        if (previousAltPage_ != altPage)
        {
            ArmPickup(mainPickup_, pageMain_[altPage ? 1 : 0], rawMainKnob);
            ArmPickup(xPickup_, pageX_[altPage ? 1 : 0], rawXKnob);
            ArmPickup(yPickup_, pageY_[altPage ? 1 : 0], rawYKnob);
            previousAltPage_ = altPage;
        }

        const int pageIndex = altPage ? 1 : 0;
        const int32_t mainKnob = ApplyPickup(mainPickup_, pageMain_[pageIndex], rawMainKnob);
        const int32_t xKnob = ApplyPickup(xPickup_, pageX_[pageIndex], rawXKnob);
        const int32_t yKnob = ApplyPickup(yPickup_, pageY_[pageIndex], rawYKnob);

        Parameters params = ComputeParameters(altPage, mainKnob, xKnob, yKnob, cv1, cv2, auxAudio, absAux);

        UpdateEnvelope(absInput, params.envelopeSensitivity, params.envelopeRelease, clockEdge);
        UpdateSampleHold(params.sampleRate, clockEdge, shGateIn);

        int32_t modSource = envelope_;
        if (shGesture)
        {
            modSource = heldValue_;
        }

        int32_t cutoff = params.rangeBase + ((modSource * params.depth) >> 12);
        cutoff = Clamp(cutoff, 64, 3900);

        int32_t filtered = ProcessFilter(input, cutoff, params.resonance);
        int32_t wet = (filtered * params.outputGain) >> 11;
        int32_t dry = input >> 2;
        int32_t wetMixed = (wet * 3072) >> 12;
        int32_t output = SoftClip12(dry + wetMixed);

        AudioOut1(output);
        AudioOut2(output);

        CVOut1(heldValue_);
        CVOut2(envelope_);

        PulseOut1(clockIn);
        PulseOut2(gateOutCounter_ > 0);
        if (gateOutCounter_ > 0)
        {
            --gateOutCounter_;
        }

        UpdateLeds(altPage, shGesture, mainKnob, xKnob, yKnob);
    }

private:
    struct Parameters
    {
        int32_t rangeBase;
        int32_t depth;
        int32_t resonance;
        int32_t envelopeSensitivity;
        int32_t envelopeRelease;
        int32_t sampleRate;
        int32_t outputGain;
    };

    int32_t envelope_ = 0;
    int32_t heldValue_ = 0;
    int32_t sampleCounter_ = 0;
    int32_t samplePeriod_ = 1200;
    int32_t gateOutCounter_ = 0;
    int32_t rng_ = 0x13579BDF;
    bool lastClockIn_ = false;
    bool controlsInitialised_ = false;
    bool previousAltPage_ = false;

    int32_t low_ = 0;
    int32_t band_ = 0;

    static constexpr int32_t kGateLength = 1600;
    static constexpr int32_t kPickupThreshold = 96;

    struct SoftPickup
    {
        bool pickedUp = true;
    };

    SoftPickup mainPickup_ {};
    SoftPickup xPickup_ {};
    SoftPickup yPickup_ {};
    int32_t pageMain_[2] {};
    int32_t pageX_[2] {};
    int32_t pageY_[2] {};

    static void ArmPickup(SoftPickup &pickup, int32_t target, int32_t raw)
    {
        int32_t diff = raw - target;
        if (diff < 0)
        {
            diff = -diff;
        }
        pickup.pickedUp = (diff <= kPickupThreshold);
    }

    static int32_t ApplyPickup(SoftPickup &pickup, int32_t &stored, int32_t raw)
    {
        if (pickup.pickedUp)
        {
            stored = raw;
            return stored;
        }

        int32_t diff = raw - stored;
        if (diff < 0)
        {
            diff = -diff;
        }
        if (diff <= kPickupThreshold)
        {
            pickup.pickedUp = true;
            stored = raw;
        }

        return stored;
    }

    Parameters ComputeParameters(bool altPage,
        int32_t mainKnob,
        int32_t xKnob,
        int32_t yKnob,
        int32_t cv1,
        int32_t cv2,
        int32_t auxAudio,
        int32_t absAux) const
    {
        Parameters params {};

        if (!altPage)
        {
            params.rangeBase = 192 + ((mainKnob * 3328) >> 12) + (cv1 >> 2) + (auxAudio >> 3);
            params.depth = 256 + ((xKnob * 3200) >> 12) + (cv2 >> 3);
            params.resonance = 128 + ((yKnob * 1500) >> 12) + (absAux >> 4);
            params.envelopeSensitivity = 1792 + (cv1 >> 1);
            params.envelopeRelease = 1024;
            params.sampleRate = 24000;
            params.outputGain = 3072;
        }
        else
        {
            params.rangeBase = 192 + ((xKnob * 3328) >> 12) + (cv1 >> 2) + (auxAudio >> 3);
            params.depth = 256 + ((yKnob * 3200) >> 12) + (cv2 >> 3);
            params.resonance = 128 + ((yKnob * 1500) >> 12) + (absAux >> 4);
            params.envelopeSensitivity = 768 + ((xKnob * 3000) >> 12) + (cv1 >> 1);
            params.envelopeRelease = 128 + (((4095 - yKnob) * 3600) >> 12);
            params.sampleRate = 480 + (((4095 - yKnob) * 47520) >> 12);
            params.outputGain = 2048 + ((mainKnob * 2047) >> 12);
        }

        params.rangeBase = Clamp(params.rangeBase, 64, 3900);
        params.depth = Clamp(params.depth, 64, 4095);
        params.resonance = Clamp(params.resonance, 64, 1800);
        params.envelopeSensitivity = Clamp(params.envelopeSensitivity, 128, 4095);
        params.envelopeRelease = Clamp(params.envelopeRelease, 64, 4095);
        params.sampleRate = Clamp(params.sampleRate, 48, 48000);
        params.outputGain = Clamp(params.outputGain, 1024, 4095);

        return params;
    }

    static int32_t Clamp(int32_t value, int32_t minValue, int32_t maxValue)
    {
        if (value < minValue)
        {
            return minValue;
        }
        if (value > maxValue)
        {
            return maxValue;
        }
        return value;
    }

    static int16_t SoftClip12(int32_t sample)
    {
        if (sample > 3072)
        {
            sample = 3072 + ((sample - 3072) >> 2);
        }
        if (sample < -3072)
        {
            sample = -3072 + ((sample + 3072) >> 2);
        }

        if (sample > 2047)
        {
            sample = 2047;
        }
        if (sample < -2048)
        {
            sample = -2048;
        }

        return static_cast<int16_t>(sample);
    }

    void UpdateEnvelope(int32_t absInput, int32_t sensitivityControl, int32_t releaseControl, bool clockEdge)
    {
        int32_t driven = (absInput * sensitivityControl) >> 11;
        driven = Clamp(driven, 0, 4095);

        if (driven > envelope_)
        {
            envelope_ += (driven - envelope_) >> 1;
        }
        else
        {
            int32_t release = 2 + ((4095 - releaseControl) >> 10);
            envelope_ -= (envelope_ - driven) >> release;
        }

        if (clockEdge && envelope_ < 512)
        {
            envelope_ = 512;
        }
    }

    void UpdateSampleHold(int32_t sampleRateControl, bool clockEdge, bool shGateIn)
    {
        samplePeriod_ = sampleRateControl;
        if (shGateIn && !clockEdge)
        {
            samplePeriod_ >>= 1;
        }
        if (samplePeriod_ < 48)
        {
            samplePeriod_ = 48;
        }

        ++sampleCounter_;
        if (clockEdge || sampleCounter_ >= samplePeriod_)
        {
            sampleCounter_ = 0;
            heldValue_ = NextRandomSigned();
            gateOutCounter_ = kGateLength;
        }
    }

    int32_t NextRandomSigned()
    {
        rng_ ^= rng_ << 13;
        rng_ ^= rng_ >> 17;
        rng_ ^= rng_ << 5;
        return static_cast<int32_t>((rng_ >> 20) & 4095) - 2048;
    }

    int32_t ProcessFilter(int32_t input, int32_t cutoff, int32_t resonance)
    {
        int32_t notch = input - low_ - ((band_ * resonance) >> 12);
        band_ += (cutoff * notch) >> 12;
        band_ = Clamp(band_, -32768, 32767);

        low_ += (cutoff * band_) >> 12;
        low_ = Clamp(low_, -32768, 32767);

        return low_;
    }

    void UpdateLeds(bool altPage, bool shGesture, int32_t mainKnob, int32_t xKnob, int32_t yKnob)
    {
        LedOn(0, altPage);
        LedOn(2, shGesture);
        LedOn(4, !altPage);

        LedBrightness(1, mainKnob);
        LedBrightness(3, xKnob);
        LedBrightness(5, yKnob);
    }
};

int main()
{
    set_sys_clock_khz(192000, true);

    F1shnet card;
    card.Run();
}
