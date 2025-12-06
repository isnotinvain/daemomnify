#include "GuiItems.h"

#include "../PluginProcessor.h"

const juce::Identifier MidiLearnItem::pAcceptMode{"accept-mode"};

MidiLearnItem::MidiLearnItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
    : foleys::GuiItem(builder, node) {
    addAndMakeVisible(midiLearnComponent);
}

void MidiLearnItem::update() {
    auto acceptModeText = magicBuilder.getStyleProperty(pAcceptMode, configNode).toString();
    if (acceptModeText == "Notes Only") {
        midiLearnComponent.setAcceptMode(MidiAcceptMode::NotesOnly);
    } else if (acceptModeText == "CCs Only") {
        midiLearnComponent.setAcceptMode(MidiAcceptMode::CCsOnly);
    } else {
        midiLearnComponent.setAcceptMode(MidiAcceptMode::Both);
    }
}

juce::Component* MidiLearnItem::getWrappedComponent() { return &midiLearnComponent; }

std::vector<foleys::SettableProperty> MidiLearnItem::getSettableProperties() const {
    std::vector<foleys::SettableProperty> props;
    props.push_back({configNode, pAcceptMode, foleys::SettableProperty::Choice, pAcceptModes[2],
                     magicBuilder.createChoicesMenuLambda(pAcceptModes)});
    return props;
}

ChordQualitySelectorItem::ChordQualitySelectorItem(foleys::MagicGUIBuilder& builder,
                                                   const juce::ValueTree& node)
    : foleys::GuiItem(builder, node) {
    addAndMakeVisible(chordQualitySelector);
}

void ChordQualitySelectorItem::update() {}

juce::Component* ChordQualitySelectorItem::getWrappedComponent() { return &chordQualitySelector; }