#pragma once

#include "PluginProcessor.h"
#include "UncleAlLookAndFeel.h"

class UncleAlsGateAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit UncleAlsGateAudioProcessorEditor(UncleAlsGateAudioProcessor&);
    ~UncleAlsGateAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;

private:
    void timerCallback() override;

    UncleAlsGateAudioProcessor& audioProcessor;
    UncleAlLookAndFeel lnf;

    // Preset Selector Dropdown (Upper Right Header)
    juce::ComboBox presetCombo;
    juce::Label presetLabel;
    void loadPreset(int presetIndex);

    // Dynamics Controls & Labels
    juce::Slider threshSlider, rangeSlider, attackSlider, holdSlider, releaseSlider;
    juce::Label threshLabel, rangeLabel, attackLabel, holdLabel, releaseLabel;
    juce::ToggleButton hardSquelchBtn{ "HARD SQUELCH" };

    // Chopper Controls & Labels
    juce::TextButton chopperOnBtn{ "CHOPPER ON" };
    juce::TextButton modeHybridBtn{ "HYBRID" }, modeChopperBtn{ "CHOPPER" }, modeGateOnlyBtn{ "GATE ONLY" };
    juce::ComboBox chopperModeCombo;
    juce::ComboBox rateCombo, slopeCombo;
    juce::Label slopeLabel;
    juce::ToggleButton hostSyncBtn{ "HOST SYNC" };
    juce::Slider swingSlider;
    juce::Label swingLabel;

    // Pattern Quick Presets
    juce::TextButton patAllBtn{ "ALL" }, patHalfBtn{ "1/2" }, patBurstBtn{ "BURST" }, patClearBtn{ "CLR" };

    // 16 Step grid buttons & velocity micro-sliders
    std::vector<std::unique_ptr<juce::ToggleButton>> stepButtons;
    std::vector<std::unique_ptr<juce::Slider>> stepVelSliders;

    // Dirt Blender Controls & Labels
    juce::ComboBox dirtSourceCombo, dirtColorCombo;
    juce::ToggleButton dirtEnabledBtn{ "DIRT ON" };
    juce::Slider dirtMixSlider, dirtFreqSlider;
    juce::Label dirtMixLabel, dirtFreqLabel;

    // Sidechain Controls & Labels
    juce::ToggleButton scEnabledBtn{ "SC FILTER" };
    juce::ComboBox scModeCombo;
    juce::ToggleButton scAuditionBtn{ "AUDITION" };
    juce::Slider scFreqSlider, scQSlider;
    juce::Label scFreqLabel, scQLabel;

    // Output Master Controls & Labels
    juce::Slider dryWetSlider, outGainSlider;
    juce::Label dryWetLabel, outGainLabel;
    juce::ToggleButton bypassBtn{ "BYPASS" }, limiterBtn{ "LIMITER" };

    // Meter telemetry smoothing
    float smoothedReductionDb = 0.0f;
    float peakReductionDb = 0.0f;
    int peakHoldTimer = 0;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> comboAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UncleAlsGateAudioProcessorEditor)
};
