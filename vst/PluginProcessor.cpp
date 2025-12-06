#include "PluginProcessor.h"

#include "BinaryData.h"

//==============================================================================
OmnifyAudioProcessor::OmnifyAudioProcessor()
    : foleys::MagicProcessor(BusesProperties()
                                 .withInput("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", GeneratedParams::createParameterLayout(params)) {
    FOLEYS_SET_SOURCE_PATH(__FILE__);

    // Update parameter map after APVTS is created so Foleys can see the parameters
    magicState.updateParameterMap();

    // Load the GUI layout - from file in debug mode for hot reload, from binary data in release
#if JUCE_DEBUG
    auto xmlFile = juce::File(__FILE__).getParentDirectory().getChildFile("Resources/magic.xml");
    if (xmlFile.existsAsFile()) {
        magicState.setGuiValueTree(xmlFile);
    } else {
        magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
    }
#else
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
#endif
}

//==============================================================================
void OmnifyAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void OmnifyAudioProcessor::releaseResources() {}

void OmnifyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(buffer);
    noteLearn.processNextMidiBuffer(midiMessages);
}

//==============================================================================

class NoteLearnItem : public foleys::GuiItem {
   public:
    FOLEYS_DECLARE_GUI_FACTORY(NoteLearnItem)

    static const juce::Identifier pCaption;

    NoteLearnItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
        : foleys::GuiItem(builder, node) {
        if (auto* processor = dynamic_cast<OmnifyAudioProcessor*>(builder.getMagicState().getProcessor())) {
            noteLearnComponent = processor->getNoteLearnComponent();
            addAndMakeVisible(noteLearnComponent);
        }
    }

    void update() override {
        if (noteLearnComponent) {
            auto captionText = magicBuilder.getStyleProperty(pCaption, configNode).toString();
            if (captionText.isNotEmpty())
                noteLearnComponent->setCaption(captionText);
        }
    }

    juce::Component* getWrappedComponent() override { return noteLearnComponent; }

    std::vector<foleys::SettableProperty> getSettableProperties() const override {
        std::vector<foleys::SettableProperty> props;
        props.push_back({configNode, pCaption, foleys::SettableProperty::Text, {}, {}});
        return props;
    }

   private:
    NoteLearnComponent* noteLearnComponent = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteLearnItem)
};

const juce::Identifier NoteLearnItem::pCaption{"caption"};

void OmnifyAudioProcessor::initialiseBuilder(foleys::MagicGUIBuilder& builder) {
    builder.registerJUCEFactories();
    builder.registerJUCELookAndFeels();
    builder.registerFactory("NoteLearn", &NoteLearnItem::factory);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OmnifyAudioProcessor(); }