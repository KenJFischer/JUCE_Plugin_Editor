/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SynthAudioProcessorEditor;
const static int TEXT_HEIGHT = 14;

enum ColorNames {
    EditorBackground, KnobCenter, KnobOutline, ThumbFill, ArcFillStart, ArcFillMiddle, ArcFillEnd, Text, SliderTrack
};
static juce::Array<juce::uint32> CustomColors{ 0xFF91B0BA, 0xFF575757, 0xFF000000, 0xFFFFFFFF, 0xFF00D0FF, 0xFFFBFF00, 0xFFFF6200, 0xFFFFFFFF, 0xFF333333 };

struct LookAndFeel : juce::LookAndFeel_V4 {
    LookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional,
        float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, 
            const juce::Slider::SliderStyle, juce::Slider&) override;

private:
    inline const static float OUTLINE_THICKNESS = 3.f;
    juce::ColourGradient gradient;

};

struct RotarySlider : juce::Slider {
    RotarySlider(juce::RangedAudioParameter& rap, SynthAudioProcessorEditor* owner, int index, bool knobZeroAtLeft);

    ~RotarySlider() {
        setLookAndFeel(nullptr);
    }

    juce::Rectangle<int> getSquareBounds(bool applyMargin) const;

    void paint(juce::Graphics& g) override;
    juce::String getDisplayString() const;

    SynthAudioProcessorEditor* myOwner;
    int parameterIndex;
    bool isZeroAtLeft = true;

private:
    
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
};

struct LinearSlider : juce::Slider {
    LinearSlider(juce::RangedAudioParameter& rap, SynthAudioProcessorEditor* owner, int index) :
        juce::Slider(juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap), myOwner(owner), parameterIndex(index)
    {
        setLookAndFeel(&lnf);
    }

    ~LinearSlider() {
        setLookAndFeel(nullptr);
    }

    juce::Rectangle<int> getRectangularBounds() const;

    void paint(juce::Graphics& g) override;

    SynthAudioProcessorEditor* myOwner;
    int parameterIndex;
    const static int LINEAR_SLIDER_WIDTH = 30 + TEXT_HEIGHT;

private:
    
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
};


class SynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SynthAudioProcessorEditor(SynthAudioProcessor&);
    ~SynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void removeHeightOutsideSlider(juce::Rectangle<int>& compBounds); // Changes bounds so that the height of the bounds = SLIDER_SIZE
    SynthAudioProcessor& audioProcessor;

private:
    std::vector<juce::Component*> getComps();

    const static int BIG_SLIDER_SIZE = 120;
    const static int LITTLE_SLIDER_SIZE = 92;
    const static int LINEAR_SLIDER_HEIGHT = 160;
    const static int LINEAR_SLIDER_WIDTH = 30 + TEXT_HEIGHT; // this should be the same as LinearSlider's LINEAR_SLIDER_WIDTH

    RotarySlider rotarySlider;
    
    LinearSlider linearSlider;

    juce::AudioProcessorValueTreeState::SliderAttachment rotaryAttachment, linearAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessorEditor)
};