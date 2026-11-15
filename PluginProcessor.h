#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <cmath>

// ==============================================================================
// Self-contained, header-only Biquad filter (no juce_dsp link dependency required)
// ==============================================================================
class UncleAlBiquad
{
public:
    UncleAlBiquad() noexcept { reset(); }

    void reset() noexcept
    {
        z1 = 0.0f;
        z2 = 0.0f;
    }

    void setCoefficients(float b0_, float b1_, float b2_, float a1_, float a2_) noexcept
    {
        b0 = b0_; b1 = b1_; b2 = b2_; a1 = a1_; a2 = a2_;
    }

    void makeLowPass(double sampleRate, float frequency, float q = 0.7071f) noexcept
    {
        const double w0 = 2.0 * juce::MathConstants<double>::pi * static_cast<double> (frequency) / sampleRate;
        const double cosw0 = std::cos(w0);
        const double alpha = std::sin(w0) / (2.0 * static_cast<double> (juce::jmax(0.05f, q)));
        const double a0 = 1.0 + alpha;
        setCoefficients(static_cast<float> (((1.0 - cosw0) * 0.5) / a0),
            static_cast<float> ((1.0 - cosw0) / a0),
            static_cast<float> (((1.0 - cosw0) * 0.5) / a0),
            static_cast<float> ((-2.0 * cosw0) / a0),
            static_cast<float> ((1.0 - alpha) / a0));
    }

    void makeHighPass(double sampleRate, float frequency, float q = 0.7071f) noexcept
    {
        const double w0 = 2.0 * juce::MathConstants<double>::pi * static_cast<double> (frequency) / sampleRate;
        const double cosw0 = std::cos(w0);
        const double alpha = std::sin(w0) / (2.0 * static_cast<double> (juce::jmax(0.05f, q)));
        const double a0 = 1.0 + alpha;
        setCoefficients(static_cast<float> (((1.0 + cosw0) * 0.5) / a0),
            static_cast<float> ((-(1.0 + cosw0)) / a0),
            static_cast<float> (((1.0 + cosw0) * 0.5) / a0),
            static_cast<float> ((-2.0 * cosw0) / a0),
            static_cast<float> ((1.0 - alpha) / a0));
    }

    void makeBandPass(double sampleRate, float frequency, float q = 1.0f) noexcept
    {
        const double w0 = 2.0 * juce::MathConstants<double>::pi * static_cast<double> (frequency) / sampleRate;
        const double cosw0 = std::cos(w0);
        const double alpha = std::sin(w0) / (2.0 * static_cast<double> (juce::jmax(0.05f, q)));
        const double a0 = 1.0 + alpha;
        setCoefficients(static_cast<float> (alpha / a0),
            0.0f,
            static_cast<float> (-alpha / a0),
            static_cast<float> ((-2.0 * cosw0) / a0),
            static_cast<float> ((1.0 - alpha) / a0));
    }

    void makePeak(double sampleRate, float frequency, float q = 1.0f, float gainFactor = 1.6f) noexcept
    {
        const double w0 = 2.0 * juce::MathConstants<double>::pi * static_cast<double> (frequency) / sampleRate;
        const double cosw0 = std::cos(w0);
        const double alpha = std::sin(w0) / (2.0 * static_cast<double> (juce::jmax(0.05f, q)));
        const double A = static_cast<double> (gainFactor);
        const double a0 = 1.0 + alpha / A;
        setCoefficients(static_cast<float> ((1.0 + alpha * A) / a0),
            static_cast<float> ((-2.0 * cosw0) / a0),
            static_cast<float> ((1.0 - alpha * A) / a0),
            static_cast<float> ((-2.0 * cosw0) / a0),
            static_cast<float> ((1.0 - alpha / A) / a0));
    }

    inline float processSample(float in) noexcept
    {
        const float out = in * b0 + z1;
        z1 = in * b1 - out * a1 + z2;
        z2 = in * b2 - out * a2;
        return out;
    }

private:
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;
};

// Fast brickwall limiter (-0.5 dBFS ceiling)
class UncleAlLimiter
{
public:
    void reset() noexcept { envelope = 0.0f; }

    inline float processSample(float in) noexcept
    {
        const float absVal = std::abs(in);
        if (absVal > envelope)
            envelope = absVal;
        else
            envelope += 0.0005f * (absVal - envelope);

        if (envelope > thresholdLinear)
            return (in / envelope) * thresholdLinear;
        return in;
    }

private:
    const float thresholdLinear = 0.944f; // -0.5 dBFS
    float envelope = 0.0f;
};

class UncleAlsGateAudioProcessor : public juce::AudioProcessor
{
public:
    UncleAlsGateAudioProcessor();
    ~UncleAlsGateAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Uncle Al's Gate"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS for thread-safe parameter management
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Metering & visualizer telemetry for GUI
    std::atomic<float> gainReductionDb{ 0.0f };
    std::atomic<float> detectorLevelDb{ -60.0f };
    std::atomic<float> gateEnvelope{ 1.0f };
    std::atomic<int> currentChopperStep{ 0 };
    std::atomic<int> gateState{ 0 }; // 0: OPEN, 1: HOLD, 2: CLOSED

    // Real-Time Oscilloscope dual-trace circular buffers
    static constexpr int scopeBufferSize = 512;
    std::array<float, scopeBufferSize> inputScopeBuffer{};
    std::array<float, scopeBufferSize> outputScopeBuffer{};
    std::atomic<int> scopeWriteIndex{ 0 };

private:
    // Sidechain detector filters (per channel, self-contained)
    std::array<UncleAlBiquad, 2> sidechainFilters;

    // Output limiters (stereo)
    std::array<UncleAlLimiter, 2> outputLimiters;

    // Noise Generator state for Dirt blender
    juce::Random random;
    float pinkFilterB0 = 0.0f, pinkFilterB1 = 0.0f, pinkFilterB2 = 0.0f;
    UncleAlBiquad dirtToneFilter;

    // Gate ballistics state
    float detectorMeanSquare = 0.0f;
    float currentGainLinear = 1.0f;
    float smoothedChopper = 1.0f;
    double holdCounterSeconds = 0.0;
    bool isGateOpen = false;

    // Host sync calculation
    double currentSampleRate = 44100.0;
    double freeRunningSamplePosition = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UncleAlsGateAudioProcessor)
};
