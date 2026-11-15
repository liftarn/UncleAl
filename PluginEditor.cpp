#include "PluginEditor.h"

UncleAlsGateAudioProcessorEditor::UncleAlsGateAudioProcessorEditor(UncleAlsGateAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Disable accessibility tree to prevent Windows UI Automation COM lingering references on exit
    setAccessible(false);

    setLookAndFeel(&lnf);

    // 0. Preset Selector Dropdown (Upper Right Header)
    presetLabel.setText("PRESET:", juce::dontSendNotification);
    presetLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(INDUSTRIAL_AMBER));
    addAndMakeVisible(presetLabel);

    addAndMakeVisible(presetCombo);
    presetCombo.addItemList({
        "1. Just One Fix (Classic Stutter)",
        "2. Psalm 69 Machine Gun",
        "3. Thieves Slap Gate",
        "4. Strobe Squelch & Static",
        "5. Burning Inside Snare Choke",
        "6. Clean Precision Expander"
        }, 1);
    presetCombo.setSelectedId(1, juce::dontSendNotification);
    presetCombo.onChange = [this] {
        loadPreset(presetCombo.getSelectedId());
        };

    auto setupKnob = [this](juce::Slider& s, juce::Label& lbl, const juce::String& labelText,
        const juce::String& paramId, const juce::String& unit, int numDecimals = 1,
        bool isFrequency = false) {
            s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
            addAndMakeVisible(s);

            lbl.setText(labelText, juce::dontSendNotification);
            lbl.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            lbl.setJustificationType(juce::Justification::centred);
            lbl.setColour(juce::Label::textColourId, juce::Colour(TEXT_MUTED_SLATE));
            addAndMakeVisible(lbl);

            s.setTextValueSuffix(" " + unit);

            if (isFrequency)
            {
                s.textFromValueFunction = [](double v) {
                    if (v >= 1000.0)
                        return juce::String(v / 1000.0, 1) + " kHz";
                    return juce::String(static_cast<int> (std::round(v))) + " Hz";
                    };
                s.valueFromTextFunction = [](const juce::String& t) {
                    auto trimmed = t.trim().toLowerCase();
                    if (trimmed.endsWith("khz"))
                        return trimmed.upToFirstOccurrenceOf("khz", false, false).getDoubleValue() * 1000.0;
                    if (trimmed.endsWith("hz"))
                        return trimmed.upToFirstOccurrenceOf("hz", false, false).getDoubleValue();
                    return trimmed.getDoubleValue();
                    };
            }
            else if (paramId == "output_gain")
            {
                s.textFromValueFunction = [](double v) {
                    juce::String prefix = (v > 0.05) ? "+" : "";
                    return prefix + juce::String(v, 1) + " dB";
                    };
                s.valueFromTextFunction = [](const juce::String& t) {
                    return t.getDoubleValue();
                    };
            }
            else
            {
                s.textFromValueFunction = [unit, numDecimals](double v) {
                    if (numDecimals == 0)
                        return juce::String(static_cast<int> (std::round(v))) + " " + unit;
                    return juce::String(v, numDecimals) + " " + unit;
                    };
                s.valueFromTextFunction = [](const juce::String& t) {
                    return t.getDoubleValue();
                    };
            }

            sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, s));
            s.updateText();
        };

    // 1. Dynamics Knobs & Labels
    setupKnob(threshSlider, threshLabel, "THRESH", "threshold", "dB", 1);
    setupKnob(rangeSlider, rangeLabel, "RANGE", "range", "dB", 0);
    setupKnob(attackSlider, attackLabel, "ATTACK", "attack", "ms", 2);
    setupKnob(holdSlider, holdLabel, "HOLD", "hold", "ms", 1);
    setupKnob(releaseSlider, releaseLabel, "RELEASE", "release", "ms", 0);

    addAndMakeVisible(hardSquelchBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "hard_squelch", hardSquelchBtn));

    // 2. Rhythmic Chopper Controls
    // [ CHOPPER ON / OFF ]
    addAndMakeVisible(chopperOnBtn);
    chopperOnBtn.setClickingTogglesState(true);
    if (auto* p = audioProcessor.apvts.getParameter("chopper_enabled"))
    {
        const bool on = p->getValue() > 0.5f;
        chopperOnBtn.setToggleState(on, juce::dontSendNotification);
        chopperOnBtn.setButtonText(on ? "CHOPPER ON" : "CHOPPER OFF");
    }
    chopperOnBtn.onClick = [this] {
        const bool on = chopperOnBtn.getToggleState();
        chopperOnBtn.setButtonText(on ? "CHOPPER ON" : "CHOPPER OFF");
        if (auto* p = audioProcessor.apvts.getParameter("chopper_enabled"))
            p->setValueNotifyingHost(on ? 1.0f : 0.0f);
        };

    // [ HYBRID | CHOPPER | GATE ONLY ] Mode Selector
    addChildComponent(chopperModeCombo);
    chopperModeCombo.addItemList({ "HYBRID", "PURE CHOPPER", "GATE ONLY" }, 1);
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "chopper_mode", chopperModeCombo));

    auto setupModeBtn = [this](juce::TextButton& btn, int modeIndex) {
        btn.setClickingTogglesState(false);
        addAndMakeVisible(btn);
        btn.onClick = [this, modeIndex] {
            chopperModeCombo.setSelectedId(modeIndex + 1, juce::sendNotificationAsync);
            };
        };
    setupModeBtn(modeHybridBtn, 0);
    setupModeBtn(modeChopperBtn, 1);
    setupModeBtn(modeGateOnlyBtn, 2);

    addAndMakeVisible(rateCombo);
    rateCombo.addItemList({ "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/64" }, 1);
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "chopper_rate", rateCombo));

    addAndMakeVisible(hostSyncBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "host_sync", hostSyncBtn));

    addAndMakeVisible(slopeCombo);
    slopeCombo.addItemList({ "Sharp", "Exponential", "S-Curve" }, 1);
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "slope", slopeCombo));

    slopeLabel.setText("SLOPE:", juce::dontSendNotification);
    slopeLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    slopeLabel.setJustificationType(juce::Justification::centredRight);
    slopeLabel.setColour(juce::Label::textColourId, juce::Colour(TEXT_MUTED_SLATE));
    addAndMakeVisible(slopeLabel);

    setupKnob(swingSlider, swingLabel, "SWING", "swing", "%", 0);

    // Quick Pattern Preset Buttons (ALL, 1/2, BURST, CLR)
    auto setupPatBtn = [this](juce::TextButton& btn) {
        addAndMakeVisible(btn);
        };
    setupPatBtn(patAllBtn);
    setupPatBtn(patHalfBtn);
    setupPatBtn(patBurstBtn);
    setupPatBtn(patClearBtn);

    patAllBtn.onClick = [this] {
        for (int i = 0; i < 16; ++i)
            if (auto* p = audioProcessor.apvts.getParameter("step_" + juce::String(i)))
                p->setValueNotifyingHost(1.0f);
        };
    patHalfBtn.onClick = [this] {
        for (int i = 0; i < 16; ++i)
            if (auto* p = audioProcessor.apvts.getParameter("step_" + juce::String(i)))
                p->setValueNotifyingHost(i % 2 == 0 ? 1.0f : 0.0f);
        };
    patBurstBtn.onClick = [this] {
        for (int i = 0; i < 16; ++i)
            if (auto* p = audioProcessor.apvts.getParameter("step_" + juce::String(i)))
                p->setValueNotifyingHost((i < 4 || (i >= 8 && i < 12)) ? 1.0f : 0.0f);
        };
    patClearBtn.onClick = [this] {
        for (int i = 0; i < 16; ++i)
            if (auto* p = audioProcessor.apvts.getParameter("step_" + juce::String(i)))
                p->setValueNotifyingHost(0.0f);
        };

    // 16 Step Sequencer Pad Buttons & Velocity Micro-Sliders
    for (int i = 0; i < 16; ++i)
    {
        auto btn = std::make_unique<juce::ToggleButton>(juce::String(i + 1));
        addAndMakeVisible(*btn);
        buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "step_" + juce::String(i), *btn));
        stepButtons.push_back(std::move(btn));

        auto sld = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
        sld->setName("step_vel_" + juce::String(i));
        sld->setRange(0.1, 1.0, 0.05);
        sld->setTooltip("Step " + juce::String(i + 1) + " Velocity");
        addAndMakeVisible(*sld);
        sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "step_vel_" + juce::String(i), *sld));
        stepVelSliders.push_back(std::move(sld));
    }

    // 3. Dirt Blender
    addAndMakeVisible(dirtEnabledBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "dirt_enabled", dirtEnabledBtn));

    addAndMakeVisible(dirtSourceCombo);
    dirtSourceCombo.addItemList({ "Vinyl Crackle", "White Noise", "Pink Noise", "Industrial Hum", "Radio Static" }, 1);
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "dirt_source", dirtSourceCombo));

    addAndMakeVisible(dirtColorCombo);
    dirtColorCombo.addItemList({ "Dark", "Bright", "Band", "Harsh" }, 1);
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "dirt_color", dirtColorCombo));

    setupKnob(dirtMixSlider, dirtMixLabel, "DIRT MIX", "dirt_mix", "%", 0);
    setupKnob(dirtFreqSlider, dirtFreqLabel, "TONE FREQ", "dirt_freq", "Hz", 0, true);

    // 4. Sidechain Filter
    addAndMakeVisible(scEnabledBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "sc_enabled", scEnabledBtn));

    addAndMakeVisible(scModeCombo);
    scModeCombo.addItemList({ "Bandpass", "Highpass", "Lowpass", "Peaking" }, 1);
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "sc_mode", scModeCombo));

    addAndMakeVisible(scAuditionBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "sc_audition", scAuditionBtn));

    setupKnob(scFreqSlider, scFreqLabel, "SC FREQ", "sc_freq", "Hz", 0, true);
    setupKnob(scQSlider, scQLabel, "RESONANCE", "sc_q", "Q", 1);

    // 5. Output Master
    setupKnob(dryWetSlider, dryWetLabel, "DRY / WET", "dry_wet", "%", 0);
    setupKnob(outGainSlider, outGainLabel, "OUT GAIN", "output_gain", "dB", 1);

    addAndMakeVisible(limiterBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "limiter", limiterBtn));

    addAndMakeVisible(bypassBtn);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "bypass", bypassBtn));

    setSize(960, 672);
    startTimerHz(30); // 30fps meter telemetry refresh
}

UncleAlsGateAudioProcessorEditor::~UncleAlsGateAudioProcessorEditor()
{
    // 1. Stop high-priority GUI refresh timer immediately to prevent callbacks during teardown
    stopTimer();

    // 2. Explicitly detach APVTS parameter listeners before controls are destroyed
    comboAttachments.clear();
    buttonAttachments.clear();
    sliderAttachments.clear();

    // 3. Clear dynamically allocated sequencer pads and micro-sliders
    stepButtons.clear();
    stepVelSliders.clear();

    // 4. Reset LookAndFeel associations
    setLookAndFeel(nullptr);
}

void UncleAlsGateAudioProcessorEditor::parentHierarchyChanged()
{
    juce::AudioProcessorEditor::parentHierarchyChanged();
    setAccessible(false);
    if (auto* top = getTopLevelComponent())
        top->setAccessible(false);
}

void UncleAlsGateAudioProcessorEditor::timerCallback()
{
    // Sync Chopper ON button state from APVTS
    if (auto* p = audioProcessor.apvts.getParameter("chopper_enabled"))
    {
        const bool on = p->getValue() > 0.5f;
        if (chopperOnBtn.getToggleState() != on)
        {
            chopperOnBtn.setToggleState(on, juce::dontSendNotification);
            chopperOnBtn.setButtonText(on ? "CHOPPER ON" : "CHOPPER OFF");
        }
    }

    // Sync Mode segmented buttons
    const int currentMode = chopperModeCombo.getSelectedItemIndex();
    modeHybridBtn.setToggleState(currentMode == 0, juce::dontSendNotification);
    modeChopperBtn.setToggleState(currentMode == 1, juce::dontSendNotification);
    modeGateOnlyBtn.setToggleState(currentMode == 2, juce::dontSendNotification);

    // Realistic analog ballistics smoothing for gain reduction meter
    const float rawDb = audioProcessor.gainReductionDb.load();
    const float clampedDb = juce::jlimit(-60.0f, 0.0f, rawDb);
    // When gate opens (clampedDb rises towards 0 dB): fast responsive opening
    // When gate closes (clampedDb falls towards -60 dB): smooth analog ballistics
    if (clampedDb > smoothedReductionDb)
        smoothedReductionDb += (clampedDb - smoothedReductionDb) * 0.45f;
    else
        smoothedReductionDb += (clampedDb - smoothedReductionDb) * 0.16f;

    if (smoothedReductionDb < peakReductionDb)
    {
        peakReductionDb = smoothedReductionDb;
        peakHoldTimer = 20; // Hold peak reduction for ~600ms
    }
    else if (peakHoldTimer > 0)
    {
        --peakHoldTimer;
    }
    else
    {
        peakReductionDb += (0.0f - peakReductionDb) * 0.10f;
    }

    repaint(); // Repaint meters and reduction bar
}

void UncleAlsGateAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 19" Rack industrial brushed faceplate
    g.fillAll(juce::Colour(CHASSIS_BG));

    // Top Header Banner
    g.setColour(juce::Colour(HEADER_BANNER_BG));
    g.fillRect(0, 0, getWidth(), 46);

    // Hazard orange industrial stripe
    g.setColour(juce::Colour(INDUSTRIAL_AMBER));
    g.fillRect(0, 44, getWidth(), 2);

    // Glowing amber LED lamp
    g.setColour(juce::Colour(INDUSTRIAL_AMBER));
    g.fillRoundedRectangle(20.0f, 17.0f, 10.0f, 10.0f, 2.0f);

    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colour(TEXT_PRIMARY_LIGHT));
    g.drawText("UNCLE AL'S GATE", 36, 11, 160, 24, juce::Justification::left);

    // Subtitle badge matching web version
    g.setColour(juce::Colour(BADGE_AMBER_BG));
    g.fillRoundedRectangle(200.0f, 13.0f, 226.0f, 20.0f, 3.0f);
    g.setColour(juce::Colour(BADGE_AMBER_BORDER));
    g.drawRoundedRectangle(200.0f, 13.0f, 226.0f, 20.0f, 3.0f, 1.0f);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colour(GLOWING_AMBER));
    g.drawText("[RHYTHMIC NOISE GATE & SQUELCHER]", 200, 13, 226, 20, juce::Justification::centred);

    // Section panel frames
    auto drawRackSection = [&g](const juce::String& title, float x, float y, float w, float h) {
        g.setColour(juce::Colour(PANEL_SURFACE_BG));
        g.fillRoundedRectangle(x, y, w, h, 6.0f);
        g.setColour(juce::Colour(PANEL_BORDER));
        g.drawRoundedRectangle(x, y, w, h, 6.0f, 1.0f);

        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(juce::Colour(GLOWING_AMBER));
        g.drawText(title, static_cast<int> (x) + 14, static_cast<int> (y) + 8, static_cast<int> (w) - 28, 16, juce::Justification::left);
        };

    drawRackSection("GATE DYNAMICS", 16.0f, 54.0f, 380.0f, 216.0f);
    drawRackSection("RHYTHMIC CHOPPER / SYNC", 412.0f, 54.0f, 532.0f, 216.0f);
    drawRackSection("DIRT / NOISE BLENDER (TRIGGERED BY GATE)", 16.0f, 284.0f, 536.0f, 134.0f);
    drawRackSection("SIDECHAIN FILTER", 568.0f, 284.0f, 376.0f, 134.0f);
    drawRackSection("GATE REDUCTION METER & OSCILLOSCOPE", 16.0f, 432.0f, 456.0f, 224.0f);
    drawRackSection("OUTPUT MASTER", 488.0f, 432.0f, 456.0f, 224.0f);

    // Segmented Mode Box Bezel (Matching [ HYBRID | CHOPPER | GATE ONLY ])
    g.setColour(juce::Colour(SEGMENTED_BEZEL_BG));
    g.fillRoundedRectangle(534.0f, 78.0f, 208.0f, 26.0f, 4.0f);
    g.setColour(juce::Colour(BEZEL_BORDER));
    g.drawRoundedRectangle(534.0f, 78.0f, 208.0f, 26.0f, 4.0f, 1.0f);

    // Step Sequencer Sub-header
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colour(TEXT_SUBHEADER_SLATE));
    g.drawText("PATTERN STEP GRID (16-STEP)", 428, 110, 150, 16, juce::Justification::left);

    // --------------------------------------------------------------------------
    // GATE REDUCTION METER: Detector Input Bar, 24-Segment LED Ladder & Scale
    // --------------------------------------------------------------------------
    // 1. Status Readout Row (GAIN REDUCTION, DET INPUT, STATE)
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(TEXT_MUTED_SLATE));
    g.drawText("GAIN REDUCTION:", 34, 456, 98, 18, juce::Justification::left);

    // Reduction readout digital badge
    g.setColour(juce::Colour(DEEP_RECESS_BG));
    g.fillRoundedRectangle(132.0f, 456.0f, 94.0f, 18.0f, 3.0f);
    g.setColour(juce::Colour(BEZEL_BORDER));
    g.drawRoundedRectangle(132.0f, 456.0f, 94.0f, 18.0f, 3.0f, 1.0f);

    juce::String dbText = (smoothedReductionDb <= -59.0f) ? "-60 dB (MUTE)" : juce::String(smoothedReductionDb, 1) + " dB";
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(smoothedReductionDb < -1.0f ? juce::Colour(REDUCTION_ACTIVE_RED) : juce::Colour(STATE_OPEN_EMERALD));
    g.drawText(dbText, 132, 456, 94, 18, juce::Justification::centred);

    // Detector input dB badge
    const float detDb = audioProcessor.detectorLevelDb.load();
    const float threshDb = *audioProcessor.apvts.getRawParameterValue("threshold");
    const bool detAboveThresh = (detDb >= threshDb);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(TEXT_MUTED_SLATE));
    g.drawText("DET:", 234, 456, 28, 18, juce::Justification::left);

    g.setColour(juce::Colour(DEEP_RECESS_BG));
    g.fillRoundedRectangle(262.0f, 456.0f, 68.0f, 18.0f, 3.0f);
    g.setColour(juce::Colour(BEZEL_BORDER));
    g.drawRoundedRectangle(262.0f, 456.0f, 68.0f, 18.0f, 3.0f, 1.0f);

    juce::String detText = (detDb <= -59.5f) ? "-60.0 dB" : juce::String(detDb, 1) + " dB";
    g.setColour(detAboveThresh ? juce::Colour(STATE_OPEN_EMERALD) : juce::Colour(TEXT_MUTED_SLATE));
    g.drawText(detText, 262, 456, 68, 18, juce::Justification::centred);

    // 2. Gate State Indicator Lamp (STATE: OPEN / HOLD / CLOSED)
    const int stateCode = audioProcessor.gateState.load();
    juce::String stateName = (stateCode == 0) ? "OPEN" : (stateCode == 1) ? "HOLD" : "CLOSED";
    juce::Colour stateCol = (stateCode == 0) ? juce::Colour(STATE_OPEN_EMERALD) : (stateCode == 1) ? juce::Colour(INDUSTRIAL_AMBER) : juce::Colour(STATE_CLOSED_RED);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(TEXT_MUTED_SLATE));
    g.drawText("STATE:", 342, 456, 42, 18, juce::Justification::left);

    g.setColour(stateCol);
    g.fillEllipse(386.0f, 460.0f, 9.0f, 9.0f);
    g.setColour(juce::Colour(PURE_WHITE));
    g.fillEllipse(388.0f, 461.5f, 2.5f, 2.5f);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(stateCol);
    g.drawText(stateName, 400, 456, 52, 18, juce::Justification::left);

    // 2b. Detector Input Bar + Threshold Target Needle (matches 2.2 skew taper)
    const int detBarX = 34;
    const int detBarY = 478;
    const int detBarW = 420;
    const int detBarH = 10;

    g.setColour(juce::Colour(METER_TROUGH_BG));
    g.fillRoundedRectangle(static_cast<float> (detBarX), static_cast<float> (detBarY),
        static_cast<float> (detBarW), static_cast<float> (detBarH), 2.0f);
    g.setColour(juce::Colour(BEZEL_BORDER));
    g.drawRoundedRectangle(static_cast<float> (detBarX), static_cast<float> (detBarY),
        static_cast<float> (detBarW), static_cast<float> (detBarH), 2.0f, 1.0f);

    const float detNorm = std::pow(juce::jlimit(0.0f, 1.0f, (detDb + 60.0f) / 60.0f), 2.2f);
    const float threshNorm = std::pow(juce::jlimit(0.0f, 1.0f, (threshDb + 60.0f) / 60.0f), 2.2f);

    const float detFillW = detNorm * static_cast<float> (detBarW - 2);
    if (detFillW > 0.5f)
    {
        g.setColour(detAboveThresh ? juce::Colour(STATE_OPEN_EMERALD) : juce::Colour(INDUSTRIAL_AMBER));
        g.fillRoundedRectangle(static_cast<float> (detBarX + 1), static_cast<float> (detBarY + 1),
            detFillW, static_cast<float> (detBarH - 2), 1.5f);
    }

    const float needleX = static_cast<float> (detBarX + 1) + threshNorm * static_cast<float> (detBarW - 4);
    g.setColour(juce::Colour(GLOWING_AMBER));
    g.fillRect(needleX - 0.5f, static_cast<float> (detBarY - 1), 3.0f, static_cast<float> (detBarH + 2));
    g.setColour(juce::Colour(PURE_WHITE));
    g.fillRect(needleX, static_cast<float> (detBarY), 2.0f, static_cast<float> (detBarH));

    // 3. 24-Segment LED Ladder Bar
    const int meterX = 34;
    const int meterY = 493;
    const int meterW = 420;
    const int meterH = 18;

    g.setColour(juce::Colour(METER_TROUGH_BG));
    g.fillRoundedRectangle(static_cast<float> (meterX - 2), static_cast<float> (meterY - 2),
        static_cast<float> (meterW + 4), static_cast<float> (meterH + 4), 3.0f);
    g.setColour(juce::Colour(BEZEL_BORDER));
    g.drawRoundedRectangle(static_cast<float> (meterX - 2), static_cast<float> (meterY - 2),
        static_cast<float> (meterW + 4), static_cast<float> (meterH + 4), 3.0f, 1.0f);

    const int totalSegments = 24;
    const float segGap = 2.0f;
    const float segW = (static_cast<float> (meterW) - (totalSegments - 1) * segGap) / totalSegments;

    const float attenuation = juce::jlimit(0.0f, 60.0f, -smoothedReductionDb);
    const int activeSegments = static_cast<int> (std::round((attenuation / 60.0f) * totalSegments));
    const float peakAtt = juce::jlimit(0.0f, 60.0f, -peakReductionDb);
    const int peakSeg = juce::jmin(totalSegments - 1, static_cast<int> (std::round((peakAtt / 60.0f) * totalSegments)));

    for (int i = 0; i < totalSegments; ++i)
    {
        const float sx = meterX + i * (segW + segGap);
        auto segRect = juce::Rectangle<float>(sx, static_cast<float>(meterY), segW, static_cast<float>(meterH));

        const bool isLit = (i < activeSegments);
        const bool isPeak = (i == peakSeg && activeSegments > 0);

        if (isLit || isPeak)
        {
            juce::Colour segCol = juce::Colour(INDUSTRIAL_AMBER); // 0 to -15 dB (Amber)
            if (i >= 8 && i < 16)
                segCol = juce::Colour(METER_ORANGE); // -15 to -35 dB (Orange)
            else if (i >= 16)
                segCol = juce::Colour(METER_RED); // -35 to -60 dB (Hot Industrial Red)

            if (isPeak && !isLit)
                segCol = juce::Colour(METER_PEAK_RED);

            g.setColour(segCol);
            g.fillRoundedRectangle(segRect, 1.5f);

            // Subtle top gloss
            g.setColour(juce::Colour(TRANSLUCENT_WHITE_GLOSS));
            g.fillRect(segRect.getX(), segRect.getY(), segRect.getWidth(), 2.0f);
        }
        else
        {
            g.setColour(juce::Colour(CONTROL_DARK_BG));
            g.fillRoundedRectangle(segRect, 1.5f);
            g.setColour(juce::Colour(METER_SEG_OFF_BORDER));
            g.drawRoundedRectangle(segRect, 1.5f, 0.5f);
        }
    }

    // 4. Calibrated dB Scale Ticks
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    g.setColour(juce::Colour(TEXT_DIM_SLATE));
    const char* ticks[] = { "0dB", "-3", "-6", "-10", "-15", "-20", "-30", "-45", "-60dB" };
    const float tickDbs[] = { 0.0f, 3.0f, 6.0f, 10.0f, 15.0f, 20.0f, 30.0f, 45.0f, 60.0f };
    for (int t = 0; t < 9; ++t)
    {
        const float fraction = tickDbs[t] / 60.0f;
        const int tx = static_cast<int>(meterX + fraction * (meterW - 20));
        g.drawText(ticks[t], tx - 12, meterY + meterH + 3, 30, 12, juce::Justification::centred);
    }

    // --------------------------------------------------------------------------
    // 5. REAL-TIME CRT OSCILLOSCOPE (Dual Trace: Pre-Gate Input & Gated Output)
    // --------------------------------------------------------------------------
    const int scopeX = 34;
    const int scopeY = 530;
    const int scopeW = 420;
    const int scopeH = 114;
    const float centerY = static_cast<float> (scopeY) + static_cast<float> (scopeH) * 0.5f;

    // Dark CRT phosphor glass screen
    g.setColour(juce::Colour(CRT_SCREEN_BG));
    g.fillRoundedRectangle(static_cast<float> (scopeX), static_cast<float> (scopeY),
        static_cast<float> (scopeW), static_cast<float> (scopeH), 4.0f);
    g.setColour(juce::Colour(CRT_SCREEN_BORDER));
    g.drawRoundedRectangle(static_cast<float> (scopeX), static_cast<float> (scopeY),
        static_cast<float> (scopeW), static_cast<float> (scopeH), 4.0f, 1.0f);

    // Subtle CRT Graticule Grid
    g.setColour(juce::Colour(CRT_GRID_LINE));
    for (int x = scopeX + 35; x < scopeX + scopeW; x += 35)
        g.drawVerticalLine(x, static_cast<float>(scopeY + 2), static_cast<float>(scopeY + scopeH - 2));
    for (int y = scopeY + 20; y < scopeY + scopeH; y += 20)
        g.drawHorizontalLine(y, static_cast<float>(scopeX + 2), static_cast<float>(scopeX + scopeW - 2));

    // Center Reference Baseline
    g.setColour(juce::Colour(CRT_CENTER_LINE));
    g.drawHorizontalLine(static_cast<int>(centerY), static_cast<float>(scopeX + 2), static_cast<float>(scopeX + scopeW - 2));

    // Sample from lock-free dual-trace ring buffers starting from oldest sample
    const int readHead = audioProcessor.scopeWriteIndex.load(std::memory_order_relaxed);
    constexpr int numScopePoints = UncleAlsGateAudioProcessor::scopeBufferSize;
    const float dx = (static_cast<float>(scopeW) - 2.0f) / static_cast<float>(numScopePoints);
    const float minPlotY = static_cast<float> (scopeY + 2);
    const float maxPlotY = static_cast<float> (scopeY + scopeH - 2);

    juce::Path inPath, outPath;
    bool inStarted = false, outStarted = false;

    for (int i = 0; i < numScopePoints; ++i)
    {
        const int idx = (readHead + i) % numScopePoints;
        const float inVal = juce::jlimit(-1.0f, 1.0f, audioProcessor.inputScopeBuffer[static_cast<size_t>(idx)]);
        const float outVal = juce::jlimit(-1.0f, 1.0f, audioProcessor.outputScopeBuffer[static_cast<size_t>(idx)]);

        const float px = static_cast<float>(scopeX + 1) + static_cast<float>(i) * dx;
        const float pyIn = juce::jlimit(minPlotY, maxPlotY, centerY - inVal * (static_cast<float>(scopeH) * 0.40f));
        const float pyOut = juce::jlimit(minPlotY, maxPlotY, centerY - outVal * (static_cast<float>(scopeH) * 0.40f));

        if (!inStarted)
        {
            inPath.startNewSubPath(px, pyIn);
            inStarted = true;
        }
        else
        {
            inPath.lineTo(px, pyIn);
        }

        if (!outStarted)
        {
            outPath.startNewSubPath(px, pyOut);
            outStarted = true;
        }
        else
        {
            outPath.lineTo(px, pyOut);
        }
    }

    // Strictly clip all trace rendering inside the bezel so strokes never touch or bleed outside the screen
    const bool isBypassed = *audioProcessor.apvts.getRawParameterValue("bypass") > 0.5f;
    {
        juce::Graphics::ScopedSaveState sss(g);
        g.reduceClipRegion(juce::Rectangle<int>(scopeX + 1, scopeY + 1, scopeW - 2, scopeH - 2));

        // 1. Draw Pre-Gate Input Trace (Ghost Sky Blue)
        g.setColour(juce::Colour(SCOPE_INPUT_TRACE));
        g.strokePath(inPath, juce::PathStrokeType(1.2f));

        // 2. Draw Post-Gate Output Trace (Glowing Industrial Amber)
        g.setColour(isBypassed ? juce::Colour(SCOPE_SKY_BLUE) : juce::Colour(INDUSTRIAL_AMBER));
        g.strokePath(outPath, juce::PathStrokeType(1.8f));
    }

    // Scope Top Badges: [● INPUT TRACE] [● GATED OUTPUT]
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    // Sky blue dot
    g.setColour(juce::Colour(SCOPE_SKY_BLUE));
    g.fillEllipse(static_cast<float> (scopeX + 8), static_cast<float> (scopeY + 6), 6.0f, 6.0f);
    g.setColour(juce::Colour(SCOPE_INPUT_LABEL));
    g.drawText("INPUT TRACE", scopeX + 18, scopeY + 2, 70, 14, juce::Justification::left);

    // Amber dot
    g.setColour(juce::Colour(INDUSTRIAL_AMBER));
    g.fillEllipse(static_cast<float> (scopeX + 96), static_cast<float> (scopeY + 6), 6.0f, 6.0f);
    g.setColour(juce::Colour(GLOWING_AMBER));
    g.drawText("GATED OUTPUT", scopeX + 106, scopeY + 2, 80, 14, juce::Justification::left);

    // Scope Bottom-Right Status Badge (Matching web version)
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    if (isBypassed)
    {
        g.setColour(juce::Colour(SCOPE_SKY_BLUE));
        g.drawText("BYPASSED", scopeX + scopeW - 90, scopeY + scopeH - 16, 82, 12, juce::Justification::right);
    }
    else
    {
        const bool squelchOn = *audioProcessor.apvts.getRawParameterValue("hard_squelch") > 0.5f;
        if (squelchOn)
        {
            g.setColour(juce::Colour(SQUELCH_ROSE_TEXT));
            g.drawText("SQUELCH ACTIVE", scopeX + scopeW - 100, scopeY + scopeH - 16, 92, 12, juce::Justification::right);
        }
        else
        {
            g.setColour(juce::Colour(TEXT_DIM_SLATE));
            g.drawText("SOFT EXPANDER", scopeX + scopeW - 100, scopeY + scopeH - 16, 92, 12, juce::Justification::right);
        }
    }

    // --------------------------------------------------------------------------
    // 6. OUTPUT MASTER METADATA PLATE
    // --------------------------------------------------------------------------
    const int plateX = 506;
    const int plateY = 566;
    const int plateW = 420;
    const int plateH = 78;

    g.setColour(juce::Colour(PLATE_DARK_BG));
    g.fillRoundedRectangle(static_cast<float> (plateX), static_cast<float> (plateY),
        static_cast<float> (plateW), static_cast<float> (plateH), 4.0f);
    g.setColour(juce::Colour(PLATE_BORDER));
    g.drawRoundedRectangle(static_cast<float> (plateX), static_cast<float> (plateY),
        static_cast<float> (plateW), static_cast<float> (plateH), 4.0f, 1.0f);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(GLOWING_AMBER));
    g.drawText("QUICK GUIDE: THRESHOLD, CHOPPER & GATED DIRT", plateX + 12, plateY + 8, plateW - 24, 16, juce::Justification::left);

    g.setFont(juce::FontOptions(8.5f));
    g.setColour(juce::Colour(TEXT_PRIMARY_LIGHT));
    g.drawText("1. SET THRESH TO DETECTOR BAR NEEDLE (GREEN = OPEN, RED = SHUT).", plateX + 12, plateY + 26, plateW - 24, 13, juce::Justification::left);
    g.setColour(juce::Colour(TEXT_MUTED_SLATE));
    g.drawText("2. USE SC FILTER TO TRIGGER ON SNARE/KICK; OFF READS FULL MIX.", plateX + 12, plateY + 41, plateW - 24, 13, juce::Justification::left);
    g.setColour(juce::Colour(TEXT_DIM_SLATE));
    g.drawText("3. CHOPPER STUTTERS TO TEMPO; DIRT BURSTS ONLY WHEN GATE OPENS.", plateX + 12, plateY + 56, plateW - 24, 13, juce::Justification::left);
}

void UncleAlsGateAudioProcessorEditor::resized()
{
    // ==============================================================================
    // 0. TOP BANNER: PRESET DROPDOWN (UPPER RIGHT)
    // ==============================================================================
    presetLabel.setBounds(getWidth() - 325, 11, 60, 24);
    presetCombo.setBounds(getWidth() - 262, 10, 246, 26);

    // ==============================================================================
    // 1. GATE DYNAMICS SECTION (x = 16..396, y = 54..270)
    // ==============================================================================
    // Row 1: THRESH, RANGE, ATTACK
    threshLabel.setBounds(32, 74, 68, 16);
    threshSlider.setBounds(32, 92, 68, 70);

    rangeLabel.setBounds(142, 74, 68, 16);
    rangeSlider.setBounds(142, 92, 68, 70);

    attackLabel.setBounds(252, 74, 68, 16);
    attackSlider.setBounds(252, 92, 68, 70);

    // Row 2: HOLD, RELEASE, HARD SQUELCH (Selectable toggle button in Gate Dynamics)
    holdLabel.setBounds(32, 168, 68, 16);
    holdSlider.setBounds(32, 186, 68, 70);

    releaseLabel.setBounds(142, 168, 68, 16);
    releaseSlider.setBounds(142, 186, 68, 70);

    hardSquelchBtn.setBounds(240, 196, 136, 30);

    // ==============================================================================
    // 2. RHYTHMIC CHOPPER / SYNC (x = 412..944, y = 54..270)
    // ==============================================================================
    // Top Row: [ CHOPPER ON ]  [ HYBRID | CHOPPER | GATE ONLY ]  RATE: [ 1/16 v ]  [✓] HOST SYNC
    chopperOnBtn.setBounds(428, 78, 96, 26);

    modeHybridBtn.setBounds(534, 80, 62, 22);
    modeChopperBtn.setBounds(600, 80, 66, 22);
    modeGateOnlyBtn.setBounds(670, 80, 70, 22);

    rateCombo.setBounds(752, 78, 72, 26);
    hostSyncBtn.setBounds(832, 78, 102, 26);

    // Row 2: Pattern Grid Sub-header & Quick presets (ALL, 1/2, BURST, CLR)
    patAllBtn.setBounds(582, 110, 32, 16);
    patHalfBtn.setBounds(618, 110, 32, 16);
    patBurstBtn.setBounds(654, 110, 44, 16);
    patClearBtn.setBounds(702, 110, 32, 16);

    // Dedicated Right Column for Slope & Swing - ZERO collision with dropdown arrow!
    // Vertically stacked: slopeCombo is at y = 110..132, swingLabel is at y = 140..154, swingSlider is at y = 154..220
    slopeLabel.setBounds(768, 112, 46, 18);
    slopeCombo.setBounds(818, 110, 114, 22);

    swingLabel.setBounds(840, 140, 72, 14);
    swingSlider.setBounds(840, 154, 72, 66);

    // 16 Step Sequencer Pads & Micro-Velocity Sliders: 2 rows of 8 pads (width: 38, gap: 6 -> ends at x = 774)
    const int stepW = 38;
    const int stepH = 24;
    const int velH = 8;
    const int startX = 428;
    const int gap = 6;
    for (int i = 0; i < 16; ++i)
    {
        const int row = i / 8;
        const int col = i % 8;
        const int padX = startX + col * (stepW + gap);
        const int padY = (row == 0) ? 132 : 176;
        stepButtons[i]->setBounds(padX, padY, stepW, stepH);
        stepVelSliders[i]->setBounds(padX, padY + stepH + 2, stepW, velH);
    }

    // ==============================================================================
    // 3. DIRT BLENDER SECTION (x = 16..552, y = 284..418)
    // ==============================================================================
    dirtEnabledBtn.setBounds(32, 314, 96, 26);
    dirtSourceCombo.setBounds(32, 354, 126, 26);
    dirtColorCombo.setBounds(166, 354, 90, 26);

    dirtMixSlider.setBounds(280, 332, 66, 72);
    dirtMixLabel.setBounds(280, 312, 66, 16);

    dirtFreqSlider.setBounds(368, 332, 66, 72);
    dirtFreqLabel.setBounds(368, 312, 66, 16);

    // ==============================================================================
    // 4. SIDECHAIN FILTER SECTION (x = 568..944, y = 284..418)
    // ==============================================================================
    scEnabledBtn.setBounds(584, 312, 108, 24);
    scModeCombo.setBounds(584, 342, 108, 24);
    scAuditionBtn.setBounds(584, 372, 108, 24);

    scFreqSlider.setBounds(714, 332, 66, 72);
    scFreqLabel.setBounds(714, 312, 66, 16);

    scQSlider.setBounds(802, 332, 66, 72);
    scQLabel.setBounds(802, 312, 66, 16);

    // ==============================================================================
    // 5. OUTPUT MASTER SECTION (x = 488..944, y = 432..656)
    // ==============================================================================
    dryWetLabel.setBounds(514, 460, 72, 16);
    dryWetSlider.setBounds(514, 478, 72, 74);

    outGainLabel.setBounds(614, 460, 72, 16);
    outGainSlider.setBounds(614, 478, 72, 74);

    limiterBtn.setBounds(726, 476, 110, 26);
    bypassBtn.setBounds(726, 514, 110, 26);
}

void UncleAlsGateAudioProcessorEditor::loadPreset(int presetIndex)
{
    auto setP = [this](const juce::String& paramId, float value) {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter(paramId)))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(value));
        };

    switch (presetIndex)
    {
    case 1: // Just One Fix (Classic Stutter)
    {
        setP("threshold", -28.0f);
        setP("range", -65.0f);
        setP("attack", 0.3f);
        setP("hold", 1.5f);
        setP("release", 12.0f);
        setP("hard_squelch", 1.0f);
        setP("chopper_enabled", 1.0f);
        setP("chopper_mode", 0.0f); // HYBRID
        setP("chopper_rate", 3.0f); // 1/16
        setP("host_sync", 1.0f);
        setP("swing", 8.0f);
        setP("slope", 0.0f); // Sharp
        setP("dirt_enabled", 1.0f);
        setP("dirt_source", 0.0f); // Vinyl crackle
        setP("dirt_mix", 32.0f);
        setP("dirt_freq", 2400.0f);
        setP("sc_enabled", 1.0f);
        setP("sc_mode", 0.0f); // Bandpass
        setP("sc_freq", 1100.0f);
        setP("sc_q", 2.8f);
        setP("dry_wet", 100.0f);
        setP("output_gain", 1.5f);
        const int steps[16] = { 1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1 };
        const float vels[16] = { 1.0f,0.1f,1.0f,0.1f,0.9f,0.1f,1.0f,0.1f,1.0f,0.1f,0.9f,0.1f,1.0f,0.8f,0.1f,1.0f };
        for (int i = 0; i < 16; ++i) {
            setP("step_" + juce::String(i), static_cast<float>(steps[i]));
            setP("step_vel_" + juce::String(i), vels[i]);
        }
        break;
    }
    case 2: // Psalm 69 Machine Gun
    {
        setP("threshold", -36.0f);
        setP("range", -75.0f);
        setP("attack", 0.1f);
        setP("hold", 0.8f);
        setP("release", 8.0f);
        setP("hard_squelch", 1.0f);
        setP("chopper_enabled", 1.0f);
        setP("chopper_mode", 0.0f);
        setP("chopper_rate", 5.0f); // 1/32
        setP("host_sync", 1.0f);
        setP("swing", 0.0f);
        setP("slope", 0.0f);
        setP("dirt_enabled", 1.0f);
        setP("dirt_source", 1.0f); // White noise
        setP("dirt_mix", 42.0f);
        setP("dirt_freq", 4500.0f);
        setP("sc_enabled", 1.0f);
        setP("sc_mode", 0.0f);
        setP("sc_freq", 1800.0f);
        setP("sc_q", 3.2f);
        setP("dry_wet", 100.0f);
        setP("output_gain", 2.0f);
        const int steps[16] = { 1,1,1,0,1,1,0,0,1,1,1,0,1,1,1,1 };
        const float vels[16] = { 1.0f,0.9f,1.0f,0.1f,1.0f,0.8f,0.1f,0.1f,1.0f,0.9f,1.0f,0.1f,1.0f,0.9f,1.0f,1.0f };
        for (int i = 0; i < 16; ++i) {
            setP("step_" + juce::String(i), static_cast<float>(steps[i]));
            setP("step_vel_" + juce::String(i), vels[i]);
        }
        break;
    }
    case 3: // Thieves Slap Gate
    {
        setP("threshold", -30.0f);
        setP("range", -60.0f);
        setP("attack", 0.5f);
        setP("hold", 3.0f);
        setP("release", 18.0f);
        setP("hard_squelch", 1.0f);
        setP("chopper_enabled", 1.0f);
        setP("chopper_mode", 0.0f);
        setP("chopper_rate", 3.0f); // 1/16
        setP("host_sync", 1.0f);
        setP("swing", 14.0f);
        setP("slope", 0.0f);
        setP("dirt_enabled", 1.0f);
        setP("dirt_source", 3.0f); // Industrial hum
        setP("dirt_mix", 38.0f);
        setP("dirt_freq", 1600.0f);
        setP("sc_enabled", 1.0f);
        setP("sc_mode", 1.0f); // Highpass
        setP("sc_freq", 180.0f);
        setP("sc_q", 1.5f);
        setP("dry_wet", 100.0f);
        setP("output_gain", 0.0f);
        const int steps[16] = { 1,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1 };
        const float vels[16] = { 1.0f,0.7f,0.1f,1.0f,0.1f,0.8f,1.0f,0.1f,1.0f,0.7f,0.1f,1.0f,0.1f,0.9f,0.1f,1.0f };
        for (int i = 0; i < 16; ++i) {
            setP("step_" + juce::String(i), static_cast<float>(steps[i]));
            setP("step_vel_" + juce::String(i), vels[i]);
        }
        break;
    }
    case 4: // Strobe Squelch & Static
    {
        setP("threshold", -34.0f);
        setP("range", -80.0f);
        setP("attack", 0.2f);
        setP("hold", 1.0f);
        setP("release", 15.0f);
        setP("hard_squelch", 1.0f);
        setP("chopper_enabled", 1.0f);
        setP("chopper_mode", 0.0f);
        setP("chopper_rate", 3.0f);
        setP("host_sync", 1.0f);
        setP("swing", 10.0f);
        setP("slope", 1.0f); // Exponential
        setP("dirt_enabled", 1.0f);
        setP("dirt_source", 4.0f); // Radio static
        setP("dirt_mix", 45.0f);
        setP("dirt_freq", 3200.0f);
        setP("sc_enabled", 1.0f);
        setP("sc_mode", 0.0f);
        setP("sc_freq", 2200.0f);
        setP("sc_q", 3.0f);
        setP("dry_wet", 100.0f);
        setP("output_gain", 0.5f);
        const int steps[16] = { 1,0,0,1,1,0,1,0,1,0,0,1,1,0,1,1 };
        const float vels[16] = { 1.0f,0.1f,0.1f,0.9f,1.0f,0.1f,0.8f,0.1f,1.0f,0.1f,0.1f,0.9f,1.0f,0.1f,0.9f,1.0f };
        for (int i = 0; i < 16; ++i) {
            setP("step_" + juce::String(i), static_cast<float>(steps[i]));
            setP("step_vel_" + juce::String(i), vels[i]);
        }
        break;
    }
    case 5: // Burning Inside Snare Choke
    {
        setP("threshold", -24.0f);
        setP("range", -70.0f);
        setP("attack", 0.1f);
        setP("hold", 4.0f);
        setP("release", 25.0f);
        setP("hard_squelch", 0.0f);
        setP("chopper_enabled", 0.0f);
        setP("chopper_mode", 2.0f); // Gate only
        setP("dirt_enabled", 1.0f);
        setP("dirt_source", 2.0f); // Pink noise
        setP("dirt_mix", 18.0f);
        setP("dirt_freq", 3800.0f);
        setP("sc_enabled", 1.0f);
        setP("sc_mode", 3.0f); // Peaking
        setP("sc_freq", 1950.0f);
        setP("sc_q", 4.5f);
        setP("dry_wet", 100.0f);
        setP("output_gain", 1.0f);
        for (int i = 0; i < 16; ++i) {
            setP("step_" + juce::String(i), 1.0f);
            setP("step_vel_" + juce::String(i), 1.0f);
        }
        break;
    }
    case 6: // Clean Precision Expander
    {
        setP("threshold", -42.0f);
        setP("range", -50.0f);
        setP("attack", 1.2f);
        setP("hold", 10.0f);
        setP("release", 80.0f);
        setP("hard_squelch", 0.0f);
        setP("chopper_enabled", 0.0f);
        setP("chopper_mode", 2.0f);
        setP("dirt_enabled", 0.0f);
        setP("dirt_mix", 0.0f);
        setP("sc_enabled", 0.0f);
        setP("sc_mode", 1.0f); // Highpass
        setP("sc_freq", 80.0f);
        setP("sc_q", 0.7f);
        setP("dry_wet", 100.0f);
        setP("output_gain", 0.0f);
        for (int i = 0; i < 16; ++i) {
            setP("step_" + juce::String(i), 1.0f);
            setP("step_vel_" + juce::String(i), 1.0f);
        }
        break;
    }
    default:
        break;
    }
}
