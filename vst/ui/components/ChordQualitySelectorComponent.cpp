#include "ChordQualitySelectorComponent.h"

#include "../LcarsColors.h"

ChordQualitySelectorComponent::ChordQualitySelectorComponent() {
    for (size_t i = 0; i < NUM_QUALITIES; ++i) {
        learners[i] = std::make_unique<MidiLearnComponent>();
        addAndMakeVisible(learners[i].get());
    }
}

MidiLearnedValue ChordQualitySelectorComponent::getLearnerValue(size_t qualityIndex) const {
    if (qualityIndex < NUM_QUALITIES) {
        return learners[qualityIndex]->getLearnedValue();
    }
    return {};
}

void ChordQualitySelectorComponent::paint(juce::Graphics& g) {
    g.fillAll(LcarsColors::grey.darker().darker());

    auto bounds = getLocalBounds();
    int rowHeight = bounds.getHeight() / NUM_QUALITIES;
    int labelWidth = 120;

    for (size_t i = 0; i < NUM_QUALITIES; ++i) {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        auto labelBounds = rowBounds.removeFromLeft(labelWidth);

        // Alternate colors for LCARS feel
        g.setColour(i % 2 == 0 ? LcarsColors::purple : LcarsColors::red);
        g.drawText(GeneratedParams::ChordQualities::NAMES[i], labelBounds.reduced(5),
                   juce::Justification::centredLeft);
    }
}

void ChordQualitySelectorComponent::resized() {
    auto bounds = getLocalBounds();
    int rowHeight = bounds.getHeight() / NUM_QUALITIES;
    int labelWidth = 120;

    for (size_t i = 0; i < NUM_QUALITIES; ++i) {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        rowBounds.removeFromLeft(labelWidth);  // Skip label area
        learners[i]->setBounds(rowBounds.reduced(2));
    }
}