#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <memory>

#include "MidiLearnComponent.h"

class ChordQualitySelectorComponent : public juce::Component {
   public:
    // Must match Python ChordQuality enum order
    static constexpr int NUM_QUALITIES = 9;
    static inline const char* QUALITY_NAMES[NUM_QUALITIES] = {
        "Major", "Minor", "Dominant 7th", "Major 7th", "Minor 7th",
        "Diminished 7th", "Augmented", "Suspended 4th", "Add 9"
    };

    ChordQualitySelectorComponent();

    void processNextMidiBuffer(const juce::MidiBuffer& buffer);

    MidiLearnedValue getLearnerValue(int qualityIndex) const;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    std::array<std::unique_ptr<MidiLearnComponent>, NUM_QUALITIES> learners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordQualitySelectorComponent)
};