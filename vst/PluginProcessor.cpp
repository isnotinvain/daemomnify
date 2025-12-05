#include "PluginProcessor.h"

#include "BinaryData.h"

//==============================================================================
OmnifyAudioProcessor::OmnifyAudioProcessor()
    : foleys::MagicProcessor(BusesProperties()
                                 .withInput("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", GeneratedParams::createParameterLayout()) {
    FOLEYS_SET_SOURCE_PATH(__FILE__);

    // Load the GUI layout from embedded binary data
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

//==============================================================================
void OmnifyAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void OmnifyAudioProcessor::releaseResources() {}

void OmnifyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(buffer, midiMessages);
    // Pass-through: do nothing to the audio
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OmnifyAudioProcessor(); }