#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// ==============================================================================
// UNCLE AL'S GATE - INDUSTRIAL RACK COLOUR PALETTE
// ==============================================================================

// Amber & Orange Accents
#define GLOWING_AMBER           0xffffb300
#define INDUSTRIAL_AMBER        0xffffa000
#define DARK_AMBER_FILL         0xffff9900
#define TRANSLUCENT_AMBER_BG    0x28ffa000
#define BADGE_AMBER_BG          0x22ffa000
#define BADGE_AMBER_BORDER      0x55ffa000
#define PAD_AMBER_TOP           0xffffa726
#define PAD_ORANGE_BOTTOM       0xffe65100
#define PAD_AMBER_BORDER        0xffffe082
#define LED_YELLOW_TOP          0xffffcc00
#define LED_ORANGE_BOTTOM       0xffff8000
#define METER_ORANGE            0xffff6f00

// Rose, Crimson & Squelch Status
#define METER_RED               0xffff2828
#define METER_PEAK_RED          0xffff6666
#define REDUCTION_ACTIVE_RED    0xffff5252
#define STATE_CLOSED_RED        0xffef4444
#define SQUELCH_ROSE_BG         0x38e11d48
#define SQUELCH_ROSE_BORDER     0xfff43f5e
#define SQUELCH_ROSE_BOX        0xffe11d48
#define SQUELCH_ROSE_TEXT       0xfffda4af

// Emerald & Scope Sky Blue
#define STATE_OPEN_EMERALD      0xff10b981
#define SCOPE_SKY_BLUE          0xff38bdf8
#define SCOPE_INPUT_TRACE       0x5538bdf8
#define SCOPE_INPUT_LABEL       0xcc38bdf8

// Whites, Text & Neutral Slates
#define PURE_WHITE              0xffffffff
#define TRANSLUCENT_WHITE_GLOSS 0x55ffffff
#define TEXT_BRIGHT_WHITE       0xfff8fafc
#define TEXT_PRIMARY_LIGHT      0xfff1f5f9
#define KNOB_THUMB_LIGHT        0xffe6e6e6
#define TEXT_HOVER_SLATE        0xffcbd5e1
#define TEXT_MUTED_SLATE        0xff94a3b8
#define TEXT_SUBHEADER_SLATE    0xff8492a6
#define TEXT_DIM_SLATE          0xff64748b
#define BORDER_HOVER_SLATE      0xff475569

// Chassis, Panels & Dark Metallic Surfaces
#define CHASSIS_BG              0xff14171a
#define HEADER_BANNER_BG        0xff0d0f12
#define PANEL_SURFACE_BG        0xff1e2228
#define PANEL_BORDER            0xff323a46
#define CONTROL_DARK_BG         0xff14181d
#define POPUP_MENU_BG           0xff171b21
#define TEXTBOX_BG              0xff0f1216
#define TEXTBOX_OUTLINE         0xff2d3540
#define COMBOBOX_OUTLINE        0xff374151
#define DEEP_RECESS_BG          0xff0a0d10
#define METER_TROUGH_BG         0xff0a0c10
#define SEGMENTED_BEZEL_BG      0xff0c0e12
#define PLATE_DARK_BG           0xff0d1014
#define CRT_SCREEN_BG           0xff07090c
#define CHECKBOX_EMPTY_BG       0xff090c0f
#define KNOB_BEZEL_BG           0xff111417
#define KNOB_BEZEL_BORDER       0xff3a4149
#define KNOB_DIAL_TOP           0xff2b3138
#define KNOB_OUTLINE_DARK       0xff1f2429
#define LED_OFF_BG              0xff1f242b
#define BTN_HOVER_BG            0xff232932
#define GROOVE_BORDER           0xff252c36
#define BEZEL_BORDER            0xff262e38
#define MODE_PILL_BG            0xff272d36
#define MODE_PILL_BORDER        0xff3b4452
#define MODE_HOVER_BG           0xff191d23
#define QUICK_BTN_BG            0xff15181d
#define QUICK_BTN_BORDER        0xff2c3440
#define STEP_PAD_OFF_BG         0xff181d23
#define STEP_PAD_OFF_BORDER     0xff2b3440
#define SQUELCH_OFF_BORDER      0xff2e3745
#define METER_SEG_OFF_BORDER    0xff1e252e
#define CRT_SCREEN_BORDER       0xff1e2632
#define CRT_GRID_LINE           0xff111720
#define CRT_CENTER_LINE         0xff1c2430
#define PLATE_BORDER            0xff242c38

class UncleAlLookAndFeel : public juce::LookAndFeel_V4
{
public:
    UncleAlLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(DARK_AMBER_FILL));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(KNOB_OUTLINE_DARK));
        setColour(juce::Slider::thumbColourId, juce::Colour(KNOB_THUMB_LIGHT));

        setColour(juce::Slider::textBoxTextColourId, juce::Colour(TEXT_PRIMARY_LIGHT));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(TEXTBOX_BG));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(TEXTBOX_OUTLINE));
        setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(INDUSTRIAL_AMBER));

        setColour(juce::ComboBox::backgroundColourId, juce::Colour(CONTROL_DARK_BG));
        setColour(juce::ComboBox::textColourId, juce::Colour(GLOWING_AMBER));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(COMBOBOX_OUTLINE));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(INDUSTRIAL_AMBER));

        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(POPUP_MENU_BG));
        setColour(juce::PopupMenu::textColourId, juce::Colour(TEXT_PRIMARY_LIGHT));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(DARK_AMBER_FILL));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(DEEP_RECESS_BG));
    }

    juce::Label* createSliderTextBox(juce::Slider& slider) override
    {
        auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
        l->setFont(juce::FontOptions(10.0f, juce::Font::bold));
        l->setJustificationType(juce::Justification::centred);
        return l;
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& /*slider*/) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto centre = bounds.getCentre();

        // Dark metallic outer bezel
        g.setColour(juce::Colour(KNOB_BEZEL_BG));
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colour(KNOB_BEZEL_BORDER));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // Knurled inner dial
        auto innerRadius = radius * 0.78f;
        juce::ColourGradient grad(juce::Colour(KNOB_DIAL_TOP), centre.x, centre.y - innerRadius,
            juce::Colour(CHASSIS_BG), centre.x, centre.y + innerRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Amber active arc
        juce::Path activeArc;
        activeArc.addCentredArc(centre.x, centre.y, radius * 0.88f, radius * 0.88f, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(juce::Colour(INDUSTRIAL_AMBER));
        g.strokePath(activeArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Indicator line
        juce::Path p;
        p.addRectangle(-2.0f, -innerRadius + 2.0f, 4.0f, innerRadius * 0.5f);
        p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        g.setColour(juce::Colour(PURE_WHITE));
        g.fillPath(p);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
        juce::Slider::SliderStyle /*style*/, juce::Slider& slider) override
    {
        if (slider.getName().startsWith("step_vel"))
        {
            auto bounds = juce::Rectangle<float>(static_cast<float> (x), static_cast<float> (y),
                static_cast<float> (width), static_cast<float> (height));
            auto trackRect = bounds.withSizeKeepingCentre(bounds.getWidth(), 3.5f);

            // Dark groove
            g.setColour(juce::Colour(DEEP_RECESS_BG));
            g.fillRoundedRectangle(trackRect, 1.5f);
            g.setColour(juce::Colour(GROOVE_BORDER));
            g.drawRoundedRectangle(trackRect, 1.5f, 0.5f);

            // Filled amber bar proportional to velocity
            const float filledWidth = juce::jlimit(0.0f, trackRect.getWidth(), sliderPos - trackRect.getX());
            auto fillRect = trackRect.withWidth(filledWidth);
            g.setColour(juce::Colour(INDUSTRIAL_AMBER));
            g.fillRoundedRectangle(fillRect, 1.5f);

            // Thumb pip
            g.setColour(juce::Colour(PURE_WHITE));
            g.fillEllipse(sliderPos - 2.5f, trackRect.getCentreY() - 2.5f, 5.0f, 5.0f);
            return;
        }

        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0, 0, juce::Slider::LinearHorizontal, slider);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto text = button.getButtonText();
        const bool isStepPad = (text.getIntValue() >= 1 && text.getIntValue() <= 16);

        if (text == "HARD SQUELCH")
        {
            auto r = bounds.reduced(1.0f);
            const bool active = button.getToggleState();

            // Background & Border matching web version
            if (active)
            {
                g.setColour(juce::Colour(SQUELCH_ROSE_BG));
                g.fillRoundedRectangle(r, 3.5f);
                g.setColour(juce::Colour(SQUELCH_ROSE_BORDER));
                g.drawRoundedRectangle(r, 3.5f, 1.5f);
            }
            else
            {
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(BTN_HOVER_BG) : juce::Colour(CHASSIS_BG));
                g.fillRoundedRectangle(r, 3.5f);
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(BORDER_HOVER_SLATE) : juce::Colour(SQUELCH_OFF_BORDER));
                g.drawRoundedRectangle(r, 3.5f, 1.0f);
            }

            // [X] checkbox indicator box matching web version
            auto boxRect = juce::Rectangle<float>(r.getX() + 6.0f, r.getCentreY() - 6.0f, 12.0f, 12.0f);
            if (active)
            {
                g.setColour(juce::Colour(SQUELCH_ROSE_BOX));
                g.fillRoundedRectangle(boxRect, 2.0f);
                g.setColour(juce::Colour(PURE_WHITE));
                g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
                g.drawText("X", boxRect.toNearestInt(), juce::Justification::centred, false);
            }
            else
            {
                g.setColour(juce::Colour(CHECKBOX_EMPTY_BG));
                g.fillRoundedRectangle(boxRect, 2.0f);
                g.setColour(juce::Colour(BORDER_HOVER_SLATE));
                g.drawRoundedRectangle(boxRect, 2.0f, 1.0f);
            }

            // Label text
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.setColour(active ? juce::Colour(SQUELCH_ROSE_TEXT) : (shouldDrawButtonAsHighlighted ? juce::Colour(TEXT_PRIMARY_LIGHT) : juce::Colour(TEXT_MUTED_SLATE)));
            auto labelBounds = r.withTrimmedLeft(24.0f);
            g.drawText(text, labelBounds.toNearestInt(), juce::Justification::centredLeft, false);
            return;
        }

        if (isStepPad)
        {
            // Illuminated step sequencer pad
            auto r = bounds.reduced(1.0f);
            const bool active = button.getToggleState();

            if (active)
            {
                juce::ColourGradient padGrad(juce::Colour(PAD_AMBER_TOP), r.getX(), r.getY(),
                    juce::Colour(PAD_ORANGE_BOTTOM), r.getX(), r.getBottom(), false);
                g.setGradientFill(padGrad);
                g.fillRoundedRectangle(r, 3.5f);

                g.setColour(juce::Colour(PAD_AMBER_BORDER));
                g.drawRoundedRectangle(r, 3.5f, 1.5f);

                g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
                g.setColour(juce::Colour(HEADER_BANNER_BG));
                g.drawText(text, r.toNearestInt(), juce::Justification::centred, false);
            }
            else
            {
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(BEZEL_BORDER) : juce::Colour(STEP_PAD_OFF_BG));
                g.fillRoundedRectangle(r, 3.5f);

                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(BORDER_HOVER_SLATE) : juce::Colour(STEP_PAD_OFF_BORDER));
                g.drawRoundedRectangle(r, 3.5f, 1.0f);

                g.setFont(juce::FontOptions(11.0f));
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(TEXT_HOVER_SLATE) : juce::Colour(TEXT_DIM_SLATE));
                g.drawText(text, r.toNearestInt(), juce::Justification::centred, false);
            }
        }
        else
        {
            // Industrial push toggle button with LED lamp indicator
            const bool active = button.getToggleState();
            const float ledRadius = 5.0f;
            const float ledX = bounds.getX() + 6.0f;
            const float ledY = bounds.getCentreY();

            // Recessed bezel for LED
            g.setColour(juce::Colour(DEEP_RECESS_BG));
            g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);
            g.setColour(juce::Colour(COMBOBOX_OUTLINE));
            g.drawEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f, 1.0f);

            if (active)
            {
                juce::ColourGradient ledGrad(juce::Colour(LED_YELLOW_TOP), ledX, ledY - 2.0f,
                    juce::Colour(LED_ORANGE_BOTTOM), ledX, ledY + 4.0f, false);
                g.setGradientFill(ledGrad);
                g.fillEllipse(ledX - ledRadius + 1.2f, ledY - ledRadius + 1.2f, (ledRadius - 1.2f) * 2.0f, (ledRadius - 1.2f) * 2.0f);

                g.setColour(juce::Colour(PURE_WHITE));
                g.fillEllipse(ledX - 1.5f, ledY - 2.5f, 2.0f, 1.5f);
            }
            else
            {
                g.setColour(juce::Colour(LED_OFF_BG));
                g.fillEllipse(ledX - ledRadius + 1.2f, ledY - ledRadius + 1.2f, (ledRadius - 1.2f) * 2.0f, (ledRadius - 1.2f) * 2.0f);
            }

            // Button label text
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.setColour(active ? juce::Colour(TEXT_BRIGHT_WHITE) : (shouldDrawButtonAsHighlighted ? juce::Colour(TEXT_HOVER_SLATE) : juce::Colour(TEXT_MUTED_SLATE)));
            auto textBounds = bounds.withTrimmedLeft(ledX + ledRadius + 8.0f);
            g.drawText(text, textBounds.toNearestInt(), juce::Justification::centredLeft, true);
        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& /*backgroundColour*/,
        bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto text = button.getButtonText();

        if (text.startsWith("CHOPPER"))
        {
            // Exact styling for [ CHOPPER ON / OFF ]
            const bool isOn = button.getToggleState();
            auto r = bounds.reduced(1.0f);
            if (isOn)
            {
                g.setColour(juce::Colour(TRANSLUCENT_AMBER_BG));
                g.fillRoundedRectangle(r, 4.0f);
                g.setColour(juce::Colour(INDUSTRIAL_AMBER));
                g.drawRoundedRectangle(r, 4.0f, 1.5f);
            }
            else
            {
                g.setColour(juce::Colour(CHASSIS_BG));
                g.fillRoundedRectangle(r, 4.0f);
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(MODE_PILL_BORDER) : juce::Colour(GROOVE_BORDER));
                g.drawRoundedRectangle(r, 4.0f, 1.0f);
            }
            return;
        }

        // Mode Segmented Buttons: HYBRID | CHOPPER | GATE ONLY
        if (text == "HYBRID" || text == "CHOPPER" || text == "GATE ONLY")
        {
            const bool isSelected = button.getToggleState();
            auto r = bounds.reduced(1.0f);
            if (isSelected)
            {
                // Active elevated dark pill
                g.setColour(juce::Colour(MODE_PILL_BG));
                g.fillRoundedRectangle(r, 3.0f);
                g.setColour(juce::Colour(MODE_PILL_BORDER));
                g.drawRoundedRectangle(r, 3.0f, 1.0f);
            }
            else if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(juce::Colour(MODE_HOVER_BG));
                g.fillRoundedRectangle(r, 3.0f);
            }
            return;
        }

        // Quick pattern buttons: ALL, 1/2, BURST, CLR
        auto r = bounds.reduced(0.5f);
        g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(BEZEL_BORDER) : juce::Colour(QUICK_BTN_BG));
        g.fillRoundedRectangle(r, 2.5f);
        g.setColour(juce::Colour(QUICK_BTN_BORDER));
        g.drawRoundedRectangle(r, 2.5f, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) override
    {
        const auto text = button.getButtonText();

        if (text.startsWith("CHOPPER"))
        {
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.setColour(button.getToggleState() ? juce::Colour(GLOWING_AMBER) : juce::Colour(TEXT_DIM_SLATE));
        }
        else if (text == "HYBRID" || text == "CHOPPER" || text == "GATE ONLY")
        {
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            if (button.getToggleState())
                g.setColour(juce::Colour(GLOWING_AMBER));
            else
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(TEXT_HOVER_SLATE) : juce::Colour(TEXT_DIM_SLATE));
        }
        else
        {
            g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
            g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(PURE_WHITE) : juce::Colour(TEXT_MUTED_SLATE));
        }

        g.drawText(text, button.getLocalBounds(), juce::Justification::centred, false);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
        int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& /*box*/) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        g.setColour(juce::Colour(CONTROL_DARK_BG));
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(juce::Colour(COMBOBOX_OUTLINE));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        juce::Path arrow;
        const float arrowX = bounds.getRight() - 14.0f;
        const float arrowY = bounds.getCentreY();
        arrow.startNewSubPath(arrowX - 4.0f, arrowY - 2.0f);
        arrow.lineTo(arrowX, arrowY + 3.0f);
        arrow.lineTo(arrowX + 4.0f, arrowY - 2.0f);

        g.setColour(juce::Colour(INDUSTRIAL_AMBER));
        g.strokePath(arrow, juce::PathStrokeType(1.5f));
    }
};
