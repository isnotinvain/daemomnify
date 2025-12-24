#include "MidiDeviceSelectorItem.h"

#include "../LcarsLookAndFeel.h"

MidiDeviceSelectorComponent::MidiDeviceSelectorComponent() {
    addAndMakeVisible(comboBox);
    addAndMakeVisible(captionLabel);

    captionLabel.setText("Input Device", juce::dontSendNotification);
    captionLabel.setColour(juce::Label::textColourId, LcarsColors::africanViolet);
    captionLabel.setJustificationType(juce::Justification::centredLeft);

    comboBox.onChange = [this]() {
        int idx = comboBox.getSelectedItemIndex();
        if (idx >= 0 && idx < deviceNames.size()) {
            currentDeviceName = deviceNames[idx];
            enableMidiDeviceInStandalone(currentDeviceName);
            if (onDeviceSelected) {
                onDeviceSelected(currentDeviceName);
            }
        }
    };

    refreshDeviceList();

    // Poll for device changes every 2 seconds
    startTimer(2000);
}

MidiDeviceSelectorComponent::~MidiDeviceSelectorComponent() { stopTimer(); }

void MidiDeviceSelectorComponent::paint(juce::Graphics& g) {
    g.setColour(LcarsColors::africanViolet);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), LcarsLookAndFeel::borderRadius, 1.0F);
}

void MidiDeviceSelectorComponent::resized() {
    if (auto* laf = dynamic_cast<LcarsLookAndFeel*>(&getLookAndFeel())) {
        captionLabel.setFont(laf->getOrbitronFont(LcarsLookAndFeel::fontSizeSmall));
    }

    constexpr int contentHeight = 30;
    auto bounds = getLocalBounds().reduced(6, 0);
    bounds = bounds.withSizeKeepingCentre(bounds.getWidth(), contentHeight);

    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(captionLabel.getFont(), captionLabel.getText(), 0, 0);
    int captionWidth = static_cast<int>(glyphs.getBoundingBox(0, -1, false).getWidth()) + 8;

    captionLabel.setBounds(bounds.removeFromLeft(captionWidth));
    bounds.removeFromLeft(4);
    comboBox.setBounds(bounds);
}

void MidiDeviceSelectorComponent::setSelectedDevice(const juce::String& deviceName) {
    currentDeviceName = deviceName;

    int idx = deviceNames.indexOf(deviceName);
    if (idx >= 0) {
        comboBox.setSelectedItemIndex(idx, juce::dontSendNotification);
    } else if (deviceNames.size() > 0) {
        if (deviceName.isEmpty()) {
            // No device selected yet - default to first device
            comboBox.setSelectedItemIndex(0, juce::dontSendNotification);
            currentDeviceName = deviceNames[0];
        } else {
            // Saved device not found - show placeholder
            comboBox.setSelectedId(0, juce::dontSendNotification);
            comboBox.setText(deviceName + " (not found)", juce::dontSendNotification);
        }
    }

    if (currentDeviceName.isNotEmpty()) {
        enableMidiDeviceInStandalone(currentDeviceName);
    }
}

void MidiDeviceSelectorComponent::refreshDeviceList() {
    auto devices = juce::MidiInput::getAvailableDevices();

    juce::StringArray newNames;
    for (const auto& device : devices) {
        if (!device.name.startsWith("Omnify")) {
            newNames.add(device.name);
        }
    }

    // Only update if the list changed
    if (newNames != deviceNames) {
        deviceNames = newNames;

        // Remember current selection
        juce::String savedSelection = currentDeviceName;

        comboBox.clear(juce::dontSendNotification);
        int id = 1;
        for (const auto& name : deviceNames) {
            comboBox.addItem(name, id++);
        }

        // Restore selection
        setSelectedDevice(savedSelection);
    }
}

void MidiDeviceSelectorComponent::setCaption(const juce::String& text) { captionLabel.setText(text, juce::dontSendNotification); }

void MidiDeviceSelectorComponent::timerCallback() { refreshDeviceList(); }

void MidiDeviceSelectorComponent::enableMidiDeviceInStandalone(const juce::String& deviceName) {
    // In standalone mode, the StandaloneFilterWindow handles MIDI device management.
    // This component is display-only for plugin mode.
    juce::ignoreUnused(deviceName);
}
