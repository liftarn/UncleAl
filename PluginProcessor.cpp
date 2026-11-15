#include "PluginProcessor.h"
#include "PluginEditor.h"

UncleAlsGateAudioProcessor::UncleAlsGateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#else
    :
#endif
apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

UncleAlsGateAudioProcessor::~UncleAlsGateAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout UncleAlsGateAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // 1. Gate Dynamics
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("threshold", 1), "Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f, 2.2f), -32.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("range", 1), "Range", juce::NormalisableRange<float>(-90.0f, 0.0f, 0.5f), -60.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("attack", 1), "Attack", juce::NormalisableRange<float>(0.05f, 50.0f, 0.05f, 0.4f), 0.5f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hold", 1), "Hold", juce::NormalisableRange<float>(0.0f, 200.0f, 0.5f, 0.4f), 2.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("release", 1), "Release", juce::NormalisableRange<float>(1.0f, 500.0f, 0.5f, 0.4f), 15.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hard_squelch", 1), "Hard Squelch", true));

    // 2. Rhythmic Chopper & Sync
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("chopper_enabled", 1), "Chopper Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("chopper_mode", 1), "Chopper Mode", juce::StringArray{ "HYBRID", "PURE CHOPPER", "GATE ONLY" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("chopper_rate", 1), "Rate", juce::StringArray{ "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/64" }, 3));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("host_sync", 1), "Host Sync", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("free_bpm", 1), "BPM", juce::NormalisableRange<float>(40.0f, 260.0f, 1.0f), 128.0f, "BPM"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("swing", 1), "Swing", juce::NormalisableRange<float>(0.0f, 75.0f, 1.0f), 12.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("slope", 1), "Slope", juce::StringArray{ "Sharp", "Exponential", "S-Curve" }, 0));

    // 16 Step grid buttons & per-step velocity sliders
    for (int i = 0; i < 16; ++i)
    {
        const bool defaultActive = (i % 2 == 0);
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("step_" + juce::String(i), 1), "Step " + juce::String(i + 1), defaultActive));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("step_vel_" + juce::String(i), 1), "Step " + juce::String(i + 1) + " Velocity",
            juce::NormalisableRange<float>(0.1f, 1.0f, 0.05f), 1.0f));
    }

    // 3. Dirt / Noise Blender
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("dirt_enabled", 1), "Dirt Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("dirt_source", 1), "Source", juce::StringArray{ "Vinyl Crackle", "White Noise", "Pink Noise", "Industrial Hum", "Radio Static" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dirt_mix", 1), "Dirt Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 30.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dirt_freq", 1), "Tone Freq", juce::NormalisableRange<float>(200.0f, 16000.0f, 1.0f, 0.3f), 2500.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("dirt_color", 1), "Color", juce::StringArray{ "Dark", "Bright", "Band", "Harsh" }, 0));

    // 4. Sidechain Filter
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("sc_enabled", 1), "SC Filter Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("sc_mode", 1), "SC Mode", juce::StringArray{ "Bandpass", "Highpass", "Lowpass", "Peaking" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_freq", 1), "SC Freq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1200.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_q", 1), "SC Q", juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f), 2.5f, "Q"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("sc_audition", 1), "Audition Sidechain", false));

    // 5. Output Master
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dry_wet", 1), "Dry/Wet", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("output_gain", 1), "Output Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1), "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("limiter", 1), "Brickwall Limiter", true));

    return { params.begin(), params.end() };
}

void UncleAlsGateAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;

    for (auto& filter : sidechainFilters)
        filter.reset();

    dirtToneFilter.reset();
    for (auto& lim : outputLimiters)
        lim.reset();

    detectorMeanSquare = 0.0f;
    currentGainLinear = 1.0f;
    smoothedChopper = 1.0f;
    holdCounterSeconds = 0.0;
    isGateOpen = true;
    freeRunningSamplePosition = 0.0;

    gainReductionDb.store(0.0f);
    detectorLevelDb.store(-60.0f);
    gateState.store(0); // 0: OPEN
    gateEnvelope.store(1.0f);

    inputScopeBuffer.fill(0.0f);
    outputScopeBuffer.fill(0.0f);
    scopeWriteIndex.store(0);
}

void UncleAlsGateAudioProcessor::releaseResources()
{
}

bool UncleAlsGateAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn != juce::AudioChannelSet::mono() && mainIn != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn == juce::AudioChannelSet::stereo() && mainOut == juce::AudioChannelSet::mono())
        return false;

    return true;
}

void UncleAlsGateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // If input is mono and output is stereo (e.g. mono track, guitar, or 1-ch interface),
    // copy channel 0 into channel 1 so both left and right channels are populated
    if (totalNumInputChannels == 1 && totalNumOutputChannels >= 2)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    }

    const int activeChannels = juce::jmin(totalNumOutputChannels, juce::jmax(totalNumInputChannels, (totalNumInputChannels == 1 && totalNumOutputChannels >= 2) ? 2 : 1));

    for (int i = activeChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    const bool bypass = *apvts.getRawParameterValue("bypass") > 0.5f;
    if (bypass)
    {
        gainReductionDb.store(0.0f);
        detectorLevelDb.store(-60.0f);
        gateEnvelope.store(1.0f);
        return;
    }

    // Read parameter values
    const float thresholdDb = *apvts.getRawParameterValue("threshold");
    const float rangeDb = *apvts.getRawParameterValue("range");
    const float attackMs = *apvts.getRawParameterValue("attack");
    const float holdMs = *apvts.getRawParameterValue("hold");
    const float releaseMs = *apvts.getRawParameterValue("release");
    const bool  hardSquelch = *apvts.getRawParameterValue("hard_squelch") > 0.5f;

    const bool  chopperOn = *apvts.getRawParameterValue("chopper_enabled") > 0.5f;
    const int   chopperMode = static_cast<int> (*apvts.getRawParameterValue("chopper_mode"));
    const int   chopperRate = static_cast<int> (*apvts.getRawParameterValue("chopper_rate"));
    const bool  hostSync = *apvts.getRawParameterValue("host_sync") > 0.5f;
    const float freeBpm = *apvts.getRawParameterValue("free_bpm");
    const float swingPct = *apvts.getRawParameterValue("swing");
    const int   slope = static_cast<int> (*apvts.getRawParameterValue("slope"));

    const bool  dirtOn = *apvts.getRawParameterValue("dirt_enabled") > 0.5f;
    const int   dirtSource = static_cast<int> (*apvts.getRawParameterValue("dirt_source"));
    const float dirtMix = *apvts.getRawParameterValue("dirt_mix") / 100.0f;
    const float dirtFreq = *apvts.getRawParameterValue("dirt_freq");
    const int   dirtColor = static_cast<int> (*apvts.getRawParameterValue("dirt_color"));

    const bool  scEnabled = *apvts.getRawParameterValue("sc_enabled") > 0.5f;
    const int   scMode = static_cast<int> (*apvts.getRawParameterValue("sc_mode"));
    const float scFreq = *apvts.getRawParameterValue("sc_freq");
    const float scQ = *apvts.getRawParameterValue("sc_q");
    const bool  scAudition = *apvts.getRawParameterValue("sc_audition") > 0.5f;

    const float dryWet = *apvts.getRawParameterValue("dry_wet") / 100.0f;
    const float outputGainDb = *apvts.getRawParameterValue("output_gain");
    const float outGainLinear = juce::Decibels::decibelsToGain(outputGainDb);
    const bool  useLimiter = *apvts.getRawParameterValue("limiter") > 0.5f;

    // Determine current DAW tempo
    double bpm = freeBpm;
    double ppqPosition = 0.0;
    bool isHostPlaying = false;

    if (auto* ph = getPlayHead())
    {
        if (auto posOpt = ph->getPosition())
        {
            if (hostSync && posOpt->getBpm().hasValue())
                bpm = *posOpt->getBpm();
            if (posOpt->getPpqPosition().hasValue())
                ppqPosition = *posOpt->getPpqPosition();
            isHostPlaying = posOpt->getIsPlaying();
        }
    }

    // Step rate subdivision
    double stepsPerQuarter = 4.0; // 1/16 default
    switch (chopperRate)
    {
    case 0: stepsPerQuarter = 1.0; break; // 1/4
    case 1: stepsPerQuarter = 2.0; break; // 1/8
    case 2: stepsPerQuarter = 3.0; break; // 1/8T
    case 3: stepsPerQuarter = 4.0; break; // 1/16
    case 4: stepsPerQuarter = 6.0; break; // 1/16T
    case 5: stepsPerQuarter = 8.0; break; // 1/32
    case 6: stepsPerQuarter = 16.0; break;// 1/64
    }

    // Copy dry audio buffer for mixing later
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Filter sidechain detector signal when SC Filter or Audition is enabled
    juce::AudioBuffer<float> scBuffer;
    if (scEnabled || scAudition)
    {
        scBuffer.makeCopyOf(buffer);

        for (auto& filter : sidechainFilters)
        {
            if (scMode == 0) // Bandpass
                filter.makeBandPass(currentSampleRate, scFreq, scQ);
            else if (scMode == 1) // Highpass
                filter.makeHighPass(currentSampleRate, scFreq, scQ);
            else if (scMode == 2) // Lowpass
                filter.makeLowPass(currentSampleRate, scFreq, scQ);
            else // Peaking
                filter.makePeak(currentSampleRate, scFreq, scQ, 1.6f);
        }

        for (int ch = 0; ch < activeChannels && ch < 2; ++ch)
        {
            auto* chData = scBuffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                chData[i] = sidechainFilters[static_cast<size_t>(ch)].processSample(chData[i]);
        }
    }

    // If Audition Sidechain is ON, update detector telemetry and route filtered signal directly to output
    if (scAudition)
    {
        float sumAudEnergy = 0.0f;
        for (int ch = 0; ch < activeChannels; ++ch)
        {
            const auto* chData = scBuffer.getReadPointer(ch);
            for (int i = 0; i < numSamples; ++i)
                sumAudEnergy += chData[i] * chData[i];
        }
        const float audRms = std::sqrt(sumAudEnergy / static_cast<float>(juce::jmax(1, activeChannels * numSamples)));
        detectorLevelDb.store(juce::jlimit(-60.0f, 0.0f, juce::Decibels::gainToDecibels(audRms + 1e-6f)));
        gainReductionDb.store(0.0f);
        gateState.store(0);
        buffer.makeCopyOf(scBuffer);
        return;
    }

    // Dirt Tone filter coefficients
    if (dirtOn && dirtMix > 0.001f)
    {
        if (dirtColor == 0) // Dark
            dirtToneFilter.makeLowPass(currentSampleRate, dirtFreq);
        else if (dirtColor == 1) // Bright
            dirtToneFilter.makeHighPass(currentSampleRate, dirtFreq);
        else if (dirtColor == 2) // Band
            dirtToneFilter.makeBandPass(currentSampleRate, dirtFreq, 2.0f);
        else // Harsh
            dirtToneFilter.makePeak(currentSampleRate, dirtFreq, 3.0f, 2.5f);
    }

    // Ballistics coefficients (explicit static_cast to avoid C4244)
    const float attackCoeff = static_cast<float> (1.0 - std::exp(-1.0 / (static_cast<double> (attackMs) * 0.001 * currentSampleRate)));
    const double holdSamples = (static_cast<double> (holdMs) * 0.001) * currentSampleRate;
    const float effRelMs = hardSquelch ? std::min(releaseMs, 6.0f) : releaseMs;
    const float effRelCoeff = static_cast<float> (1.0 - std::exp(-1.0 / (static_cast<double> (effRelMs) * 0.001 * currentSampleRate)));

    // Tight RMS energy follower for detector input (1ms attack / 18ms release)
    // Tracks rhythmic valleys cleanly without bass zero-crossing drops or smearing dense mixes
    const float detAttackCoeff = static_cast<float> (1.0 - std::exp(-1.0 / (0.0010 * currentSampleRate)));
    const float detReleaseCoeff = static_cast<float> (1.0 - std::exp(-1.0 / (0.0180 * currentSampleRate)));
    const float chopperSmoothCoeff = static_cast<float> (1.0 - std::exp(-1.0 / (0.0015 * currentSampleRate)));
    const double swingDelay = (static_cast<double> (juce::jlimit(0.0f, 75.0f, swingPct)) / 100.0) * 0.5;

    // Dual-Threshold Hysteresis (Schmitt Trigger)
    // Tight 0.8 dB hysteresis prevents chatter while keeping threshold responsive on compressed mixes
    const float openThreshDb = thresholdDb;
    const float closeThreshDb = thresholdDb - 0.8f;

    float sumGateLinear = 0.0f;
    float lastScDb = -60.0f;

    // Process sample by sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1. True RMS detector energy (pure filtered SC when enabled, pure fullband dry when bypassed)
        float sumEnergy = 0.0f;
        for (int ch = 0; ch < activeChannels; ++ch)
        {
            const float detSample = scEnabled ? scBuffer.getSample(ch, sample) : dryBuffer.getSample(ch, sample);
            sumEnergy += detSample * detSample;
        }
        const float meanSquare = sumEnergy / static_cast<float>(juce::jmax(1, activeChannels));

        if (meanSquare > detectorMeanSquare)
            detectorMeanSquare += (meanSquare - detectorMeanSquare) * detAttackCoeff;
        else
            detectorMeanSquare += (meanSquare - detectorMeanSquare) * detReleaseCoeff;

        const float detRms = std::sqrt(juce::jmax(0.0f, detectorMeanSquare));
        const float scDb = juce::Decibels::gainToDecibels(detRms + 1e-6f);
        lastScDb = scDb;

        // Gate state with Dual-Threshold Hysteresis
        if (!isGateOpen)
        {
            if (scDb >= openThreshDb)
            {
                isGateOpen = true;
                holdCounterSeconds = holdSamples;
            }
        }
        else
        {
            if (scDb >= closeThreshDb)
            {
                holdCounterSeconds = holdSamples;
            }
            else if (holdCounterSeconds > 0)
            {
                holdCounterSeconds -= 1.0;
            }
            else
            {
                isGateOpen = false;
            }
        }

        // Target downward expander gain
        float targetGain = 1.0f;
        if (!isGateOpen)
        {
            if (hardSquelch)
            {
                targetGain = 0.0f; // Snaps to absolute silence
            }
            else
            {
                const float diff = openThreshDb - scDb;
                const float attDb = std::min(-rangeDb, diff * 2.0f);
                targetGain = juce::Decibels::decibelsToGain(-attDb);
            }
        }

        // Smooth with ballistics (ramps cleanly all the way to 0.0 without hard click steps)
        const float coeff = (targetGain > currentGainLinear) ? attackCoeff : effRelCoeff;
        currentGainLinear += (targetGain - currentGainLinear) * coeff;

        // 2. Rhythmic Chopper Calculation (with Swing & 1.5ms De-clicker)
        float chopperMultiplier = 1.0f;
        int activeStep = 0;

        if (chopperOn)
        {
            double stepLoc = 0.0;
            if (hostSync && isHostPlaying)
            {
                const double currentPpq = ppqPosition + (static_cast<double> (sample) / currentSampleRate) * (bpm / 60.0);
                stepLoc = currentPpq * stepsPerQuarter;
            }
            else if (hostSync && !isHostPlaying)
            {
                chopperMultiplier = 1.0f;
            }
            else
            {
                const double currentPpq = (freeRunningSamplePosition / currentSampleRate) * (bpm / 60.0);
                stepLoc = currentPpq * stepsPerQuarter;
            }

            if (!(hostSync && !isHostPlaying))
            {
                const double pairIdx = std::floor(stepLoc / 2.0);
                const double pairFrac = stepLoc - pairIdx * 2.0;
                const double splitPoint = 1.0 + swingDelay;

                double fract = 0.0;
                if (pairFrac < splitPoint)
                {
                    activeStep = static_cast<int>(pairIdx * 2.0) % 16;
                    fract = pairFrac / splitPoint;
                }
                else
                {
                    activeStep = (static_cast<int>(pairIdx * 2.0) + 1) % 16;
                    fract = (pairFrac - splitPoint) / std::max(0.05, 1.0 - swingDelay);
                }
                if (activeStep < 0) activeStep += 16;
                currentChopperStep.store(activeStep);

                const bool isStepOn = *apvts.getRawParameterValue("step_" + juce::String(activeStep)) > 0.5f;
                const float stepVel = *apvts.getRawParameterValue("step_vel_" + juce::String(activeStep));
                if (!isStepOn)
                {
                    chopperMultiplier = 0.0f;
                }
                else
                {
                    if (slope == 1) // Exponential decay
                        chopperMultiplier = stepVel * static_cast<float> (std::exp(-fract * 4.0));
                    else if (slope == 2) // S-Curve
                        chopperMultiplier = stepVel * static_cast<float> (std::sin(fract * juce::MathConstants<double>::pi));
                    else
                        chopperMultiplier = stepVel; // Sharp
                }
            }
        }
        freeRunningSamplePosition += 1.0;
        smoothedChopper += (chopperMultiplier - smoothedChopper) * chopperSmoothCoeff;

        // Combined final gate multiplier based on Chopper Mode
        float finalGate = currentGainLinear;
        if (chopperOn)
        {
            if (chopperMode == 0) // HYBRID: Gate dynamics multiplied by chopper rhythm
                finalGate = currentGainLinear * smoothedChopper;
            else if (chopperMode == 1) // PURE CHOPPER: Rhythmic chopper overrides incoming dynamics
                finalGate = smoothedChopper;
            else // GATE ONLY: Pure downward expander gate
                finalGate = currentGainLinear;
        }

        // 3. Dirt / Noise synthesis triggered ONLY when gate is open!
        float dirtSample = 0.0f;
        if (dirtOn && dirtMix > 0.001f && finalGate > 0.001f)
        {
            float rawNoise = 0.0f;
            if (dirtSource == 0) // Vinyl crackle
            {
                rawNoise = (random.nextFloat() * 2.0f - 1.0f) * 0.1f;
                if (random.nextFloat() < 0.004f)
                    rawNoise += (random.nextBool() ? 0.7f : -0.7f);
            }
            else if (dirtSource == 1) // White noise
            {
                rawNoise = (random.nextFloat() * 2.0f - 1.0f) * 0.5f;
            }
            else if (dirtSource == 2) // Pink noise
            {
                const float white = random.nextFloat() * 2.0f - 1.0f;
                pinkFilterB0 = 0.99886f * pinkFilterB0 + white * 0.0555179f;
                pinkFilterB1 = 0.99332f * pinkFilterB1 + white * 0.0750759f;
                pinkFilterB2 = 0.96900f * pinkFilterB2 + white * 0.1538520f;
                rawNoise = (pinkFilterB0 + pinkFilterB1 + pinkFilterB2) * 0.15f;
            }
            else if (dirtSource == 3) // Industrial hum
            {
                const double t = static_cast<double> (sample) / currentSampleRate;
                rawNoise = std::sin(2.0 * juce::MathConstants<double>::pi * 60.0 * t) * 0.4f
                    + (random.nextFloat() * 2.0f - 1.0f) * 0.1f;
            }
            else // Radio static
            {
                rawNoise = std::tanh((random.nextFloat() * 2.0f - 1.0f) * 2.5f) * 0.4f;
            }

            // Filter noise through tone filter
            rawNoise = dirtToneFilter.processSample(rawNoise);

            // Gated dirt injection
            dirtSample = rawNoise * dirtMix * finalGate;
        }

        // Apply to channel samples (Left & Right)
        for (int ch = 0; ch < activeChannels; ++ch)
        {
            const float wetSignal = (dryBuffer.getSample(ch, sample) * finalGate) + dirtSample;
            const float drySignal = dryBuffer.getSample(ch, sample);
            const float mixed = (drySignal * (1.0f - dryWet)) + (wetSignal * dryWet);
            buffer.setSample(ch, sample, mixed * outGainLinear);
        }

        sumGateLinear += finalGate;
    }

    // Optional Brickwall Limiter (stereo, self-contained, no juce_dsp link required)
    if (useLimiter)
    {
        for (int ch = 0; ch < activeChannels && ch < 2; ++ch)
        {
            auto* chData = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                chData[i] = outputLimiters[static_cast<size_t>(ch)].processSample(chData[i]);
        }
    }

    const float avgGateGain = sumGateLinear / static_cast<float>(numSamples);
    const float blockReductionDb = juce::Decibels::gainToDecibels(juce::jmax(1e-4f, avgGateGain));
    const int currentGateStateCode = (avgGateGain > 0.7f) ? 0 : (avgGateGain > 0.05f ? 1 : 2); // 0: OPEN, 1: HOLD/CHATTER, 2: CLOSED
    gateState.store(currentGateStateCode);
    gainReductionDb.store(juce::jlimit(-60.0f, 0.0f, blockReductionDb));
    detectorLevelDb.store(juce::jlimit(-60.0f, 0.0f, lastScDb));
    gateEnvelope.store(avgGateGain);

    // Push audio snapshot to oscilloscope circular ring buffers (lock-free)
    int currentWrite = scopeWriteIndex.load(std::memory_order_relaxed);
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inSamp = 0.0f;
        float outSamp = 0.0f;
        for (int ch = 0; ch < activeChannels; ++ch)
        {
            inSamp += dryBuffer.getSample(ch, sample);
            outSamp += buffer.getSample(ch, sample);
        }
        inSamp /= static_cast<float>(juce::jmax(1, activeChannels));
        outSamp /= static_cast<float>(juce::jmax(1, activeChannels));

        inputScopeBuffer[static_cast<size_t>(currentWrite)] = inSamp;
        outputScopeBuffer[static_cast<size_t>(currentWrite)] = outSamp;
        currentWrite = (currentWrite + 1) % scopeBufferSize;
    }
    scopeWriteIndex.store(currentWrite, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* UncleAlsGateAudioProcessor::createEditor()
{
    return new UncleAlsGateAudioProcessorEditor(*this);
}

void UncleAlsGateAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void UncleAlsGateAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UncleAlsGateAudioProcessor();
}
