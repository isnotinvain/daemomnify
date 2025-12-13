#include "../GuiItems.h"

const juce::Identifier FilePickerItem::pFileFilter{"file-filter"};

FilePickerItem::FilePickerItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
    : foleys::GuiItem(builder, node) {

    // Set up the container with our components
    container.addAndMakeVisible(pathLabel);
    container.addAndMakeVisible(browseButton);

    // Style the label
    pathLabel.setColour(juce::Label::outlineColourId, juce::Colour(0xFFFF8800));  // lcars-orange
    pathLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8800));
    pathLabel.setJustificationType(juce::Justification::centredLeft);

    // Style the button
    browseButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFFF8800));
    browseButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    browseButton.onClick = [this]() { openFileChooser(); };

    addAndMakeVisible(container);
}

void FilePickerItem::resized() {
    foleys::GuiItem::resized();
    auto bounds = getLocalBounds();
    container.setBounds(bounds);

    // Layout children within container
    auto inner = container.getLocalBounds();
    browseButton.setBounds(inner.removeFromRight(80));
    pathLabel.setBounds(inner.reduced(2));
}

void FilePickerItem::update() {
    updateLabelFromState();
}

void FilePickerItem::updateLabelFromState() {
    auto path = getMagicState().getPropertyAsValue("chords_json_file").toString();
    pathLabel.setText(path, juce::dontSendNotification);
}

juce::Component* FilePickerItem::getWrappedComponent() {
    return &container;
}

std::vector<foleys::SettableProperty> FilePickerItem::getSettableProperties() const {
    std::vector<foleys::SettableProperty> props;
    props.push_back({configNode, pFileFilter, foleys::SettableProperty::Text, {}, {}});
    return props;
}

void FilePickerItem::openFileChooser() {
    auto filterPattern = magicBuilder.getStyleProperty(pFileFilter, configNode).toString();
    if (filterPattern.isEmpty()) {
        filterPattern = "*";
    }

    auto currentPath = getMagicState().getPropertyAsValue("chords_json_file").toString();
    auto startDir = currentPath.isNotEmpty()
                        ? juce::File(currentPath).getParentDirectory()
                        : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);

    fileChooser = std::make_unique<juce::FileChooser>("Select a file", startDir, filterPattern);

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            auto result = fc.getResult();
            if (result.existsAsFile()) {
                auto newPath = result.getFullPathName();
                getMagicState().getPropertyAsValue("chords_json_file").setValue(newPath);
                pathLabel.setText(newPath, juce::dontSendNotification);
            }
        });
}