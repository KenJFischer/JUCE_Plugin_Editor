/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthAudioProcessorEditor::SynthAudioProcessorEditor(SynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    
    // sliders
    rotarySlider(*audioProcessor.apvts.getParameter(audioProcessor.ParamNames[audioProcessor.rotaryParam]), this, audioProcessor.rotaryParam, false),
    linearSlider(*audioProcessor.apvts.getParameter(audioProcessor.ParamNames[audioProcessor.linearParam]), this, audioProcessor.linearParam),

    // attachments
    rotaryAttachment(audioProcessor.apvts, audioProcessor.ParamNames[audioProcessor.rotaryparam], rotarySlider),
    linearAttachment(audioProcessor.apvts, audioProcessor.ParamNames[audioProcessor.linearparam], linearSlider)
{
    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    setSize(800, 500);
}

std::vector<juce::Component*> SynthAudioProcessorEditor::getComps() {
    return { &rotarySlider, &linearSlider };
}

RotarySlider::RotarySlider(juce::RangedAudioParameter& rap, SynthAudioProcessorEditor* owner, int index, bool knobZeroAtLeft) :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap), myOwner(owner), parameterIndex(index)
{
    isZeroAtLeft = knobZeroAtLeft; // determine if origin on knob is at left or in the middle
    //juce::ImageCache::setCacheTimeout(10000); // uncomment this if using images for knobs instead of vector graphics
    setLookAndFeel(&lnf);
}

LookAndFeel::LookAndFeel() {
    gradient = juce::ColourGradient(juce::Colour::Colour(CustomColors[ArcFillStart]), 0, 0, juce::Colour::Colour(CustomColors[ArcFillEnd]), 100, 0, false); // linear gradient: 0 to 100
    gradient.addColour(0.6, juce::Colour::Colour(CustomColors[ArcFillMiddle]));
}

SynthAudioProcessorEditor::~SynthAudioProcessorEditor()
{
}

//==============================================================================
void SynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::Colour(CustomColors[EditorBackground]));

    // uncomment below and comment out the above line to draw an image in the background instead of filling with a solid color
    //int pngSize = 0;
    //int imgHash = BinaryData::namedResourceListSize - 1; // retrieves the last image from BinaryData
    //juce::Rectangle<float> bounds = getLocalBounds().toFloat();
    //const char* imgString = BinaryData::getNamedResource(BinaryData::namedResourceList[imgHash], pngSize);
    //juce::Image bkgrdImg = juce::ImageFileFormat::loadFrom(imgString, (size_t)pngSize);
    //g.drawImage(bkgrdImg, bounds);
}

void SynthAudioProcessorEditor::removeHeightOutsideSlider(juce::Rectangle<int>& compBounds) {
    int halfBoundsHeight = (compBounds.getHeight() - SLIDER_SIZE) / 2;
    compBounds.removeFromTop(halfBoundsHeight);
    compBounds.removeFromBottom(halfBoundsHeight);
}

void SynthAudioProcessorEditor::resized()
{
    // set positions of all components
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.removeFromTop(BIG_SLIDER_SIZE);
    bounds.removeFromLeft(BIG_SLIDER_SIZE);
    juce::Rectangle<int> compBounds = bounds.removeFromTop(SLIDER_SIZE);

    rotarySlider.setBounds(compBounds.removeFromLeft(SLIDER_SIZE));
    compBounds = bounds.removeFromTop(LINEAR_SLIDER_HEIGHT)
    linearSlider.setBounds(compBounds.removeFromLeft(LINEAR_SLIDER_WIDTH));
}




//==============================================================================
void RotarySlider::paint(juce::Graphics& g) {
    using namespace juce;
    constexpr float startAng = degreesToRadians(225.f);
    constexpr float endAng = degreesToRadians(135.f) + MathConstants<float>::twoPi; //270 degrees of rotation
    juce::Range<double> range = getRange();
    juce::Rectangle<int> sliderBounds = getSquareBounds(true);

    // use this to debug bounding boxes
    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBounds);
    ////juce::Rectangle<int> debugBounds = getSquareBounds(false);
    ////g.setColour(Colours::blue);
    ////g.drawRect(debugBounds);

    getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), jmap(getValue(),
        range.getStart(), range.getEnd(), 0.0, 1.0), startAng, endAng, *this);
}

juce::Rectangle<int> RotarySlider::getSquareBounds(bool applyMargin) const {
    juce::Rectangle<int> bounds = getLocalBounds();
    int size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    if (applyMargin) { size -= TEXT_HEIGHT * 2; }
    juce::Rectangle<int> newBounds;
    newBounds.setSize(size, size);
    newBounds.setCentre(bounds.getCentre());
    return newBounds;
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider& comp) {
    using namespace juce;
    Rectangle<float> bounds = Rectangle<float>(x, y, width, height - 1);

    // draw Slider body
    g.setColour(Colour::Colour(CustomColors[KnobCenter]));
    g.fillEllipse(bounds);
    // draw Slider outline
    g.setColour(Colour::Colour(CustomColors[KnobOutline]));
    g.drawEllipse(bounds, OUTLINE_THICKNESS);

    // add details to Slider
    if (RotarySlider* slider = dynamic_cast<RotarySlider*>(&comp)) {
        Point<float> center = bounds.getCentre();
        float toAngle = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle); // current position on Slider

        // Cool color-changing arc on Slider
        Path valueArc;
        int radius = (juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2) - 2; // finds square bounds with margin
        float arcLengthAdjustment = 0.f;

        float colorPoint = rotaryStartAngle;
        if (!slider->isZeroAtLeft) {
            colorPoint = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) / 2; // set the starting position of the knob to the center
        }
        if (toAngle != colorPoint) {
            arcLengthAdjustment = (toAngle > colorPoint) ? -0.04f : 0.04f; // adjust the arc length so it fits better under the thumb
        }

        // calculate the color-changing arc
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), radius, radius, 0.0f, colorPoint,
                toAngle + arcLengthAdjustment, true);

        if (!slider->isZeroAtLeft) { // determine what color the arc should be based on thumb position
            colorPoint = (sliderPosProportional - 0.5f) * 2;
            if (colorPoint < 0) { colorPoint *= -1; }
        }
        else {
            colorPoint = sliderPosProportional;
        }
        
        Colour interpolatedColor = gradient.getColourAtPosition(colorPoint);
        g.setColour(interpolatedColor);
        g.strokePath(valueArc, juce::PathStrokeType(OUTLINE_THICKNESS * 3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // draw currrent slider value
        // draw textbox on control
        g.setFont(TEXT_HEIGHT * 1.5);
        juce::String text = slider->getDisplayString();
        int strWidth = g.getCurrentFont().getStringWidth(text);
        Rectangle<float> box;
        box.setSize(strWidth + 4, TEXT_HEIGHT + 2);
        box.setCentre(center);

        // draw text inside slider
        g.setColour(Colour::Colour(CustomColors[KnobCenter]));
        g.fillRect(box);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(text, box.toNearestInt(), juce::Justification::centred, 1);

        // thumb
        Path thumb;
        box.setLeft(center.getX() - 3);
        box.setRight(center.getX() + 3);
        box.setTop(bounds.getY() - 3);
        box.setBottom(center.getY() + ((bounds.getY() - center.getY()) / ((width == 92 /*big slider width - 2 * textHeight*/) ? 2.f : 1.5f)));

        thumb.addRectangle(box);
        jassert(rotaryStartAngle < rotaryEndAngle);
        // Put thumb in right place
        thumb.applyTransform(AffineTransform().rotated(toAngle, center.getX(), center.getY()));
        g.setColour(Colour::Colour(CustomColors[ThumbFill]));
        g.fillPath(thumb);

        // draw label for control
        // draw textbox below control
        g.setFont(TEXT_HEIGHT);
        text = slider->myOwner->audioProcessor.ParamNames[slider->parameterIndex];
        strWidth = g.getCurrentFont().getStringWidth(text);
        box.setSize(strWidth + 4, TEXT_HEIGHT);
        center.addXY(0, radius + 4 + (TEXT_HEIGHT / 2));
        box.setCentre(center);

        // draw text outside slider
        g.setColour(Colour::Colour(CustomColors[EditorBackground]));
        g.fillRect(box);
        g.setColour(Colour::Colour(CustomColors[Text]));
        g.drawFittedText(text, box.toNearestInt(), juce::Justification::centred, 2);
    }
}

juce::String RotarySlider::getDisplayString() const {
    juce::String str;
    // determine how many decimals to show on knob
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        float val = getValue();
        str = juce::String(val, 2);
    }
    else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(param)) {
            float val = getValue();
            str = juce::String(val, 0);
    }
    else {
        jassertfalse; // uh oh
    }
    
    return str;
}

//==============================================================================

void LinearSlider::paint(juce::Graphics& g) {
    using namespace juce;
    juce::Range<double> range = getRange();
    juce::Rectangle<int> sliderBounds = getRectangularBounds();

    // use this to debug bounding boxes
    /*g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::green);
    g.drawRect(sliderBounds);*/

    getLookAndFeel().drawLinearSlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), jmap(getValue(),
        range.getStart(), range.getEnd(), 0.0, 1.0), 0.f, 1.f,
        juce::Slider::SliderStyle::LinearVertical, *this);
}

juce::Rectangle<int> LinearSlider::getRectangularBounds() const {
    juce::Rectangle<int> bounds = getLocalBounds();
    int width = LINEAR_SLIDER_WIDTH;
    int height = bounds.getHeight() - TEXT_HEIGHT * 1.5;
    juce::Rectangle<int> newBounds;
    newBounds.setSize(width, height);
    newBounds.setCentre(bounds.getCentreX(), bounds.getCentreY() - 2);
    return newBounds;
}

void LookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderVal, float minSliderPos, float maxSliderPos,
    const juce::Slider::SliderStyle, juce::Slider& comp) {
    using namespace juce;
    Rectangle<float> bounds = Rectangle<float>(x + TEXT_HEIGHT / 2, y, width - TEXT_HEIGHT, height);

    // draw slider
    if (LinearSlider* slider = dynamic_cast<LinearSlider*>(&comp)) {
        // draw background
        g.setColour(Colour::Colour(CustomColors[KnobCenter]));
        g.fillRect(bounds);
        g.setColour(Colour::Colour(CustomColors[KnobOutline]));
        g.drawRect(bounds, OUTLINE_THICKNESS);

        // draw slider track
        Rectangle<float> box = Rectangle<float>(bounds.getX() + (bounds.getWidth() / 3) - 1.5, y + TEXT_HEIGHT, 3 + bounds.getWidth() / 3, height - 2 * TEXT_HEIGHT);
        g.setColour(Colour::Colour(CustomColors[SliderTrack]));
        g.fillRect(box);
        float sliderPos = juce::jmap(sliderVal, minSliderPos, maxSliderPos, box.getBottom(), box.getTopLeft().getY());

        // cool color-changing track
        box.removeFromTop(sliderPos - box.getTopLeft().getY());
        Colour interpolatedColor = gradient.getColourAtPosition(sliderVal);
        g.setColour(interpolatedColor);
        g.fillRect(box);

        // draw thumb
        box = Rectangle<float>(x + (width / 4), sliderPos - 12, width / 2, 24);
        g.setColour(Colour::Colour(CustomColors[ThumbFill]));
        g.fillRect(box);
        g.setColour(Colour::Colour(CustomColors[SliderTrack]));

        // draw label for control
        // draw textbox below control
        g.setFont(TEXT_HEIGHT);
        juce::String text = slider->myOwner->audioProcessor.ParamNames[slider->parameterIndex];
        int strWidth = g.getCurrentFont().getStringWidth(text);
        box.setSize(strWidth + 4, TEXT_HEIGHT + 2);
        box.setCentre(bounds.getCentreX(), y + height - 1 + TEXT_HEIGHT / 2);
        
        // draw text outside slider
        g.setColour(Colour::Colour(CustomColors[EditorBackground]));
        g.setColour(Colour::Colour(CustomColors[Text]));
        g.drawFittedText(text, box.toNearestInt(), juce::Justification::centred, 2);
    }
}