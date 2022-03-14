# JUCE_Plugin_Editor
 Some template code for a basic vector-based GUI for a JUCE plugin. This GUI will have a blue-grey background, dark grey rotary sliders, and dark grey linear sliders. The knobs can display their current values on them, and there are colorful arcs that give visual indications of each slider's position. Colors are easily changeable in the PluginEditor.h file.
 
 You'll need to provide parameters from the processor's apvts for the slider attachments to hook into. I recommend that you also have a ParamNames string array in your processor.h that stores all the text that you want to display under each knob. The index that you provide to the RotarySlider and LinearSlider constructors will determine which string it pulls from this ParamNames array.
 
