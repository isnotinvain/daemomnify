#include "../GuiItems.h"

const juce::Identifier FilePickerItem::pFileFilter{"file-filter"};

FilePickerItem::FilePickerItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
    : foleys::Container(builder, node) {
    // createSubComponents() is called by our DECLARE_CONTAINER_FACTORY
}

void FilePickerItem::createSubComponents() {
    foleys::Container::createSubComponents();

    // Get the first child as our button
    auto it = begin();
    if (it != end()) {
        buttonItem = it->get();
    }

    if (getButton() == nullptr) {
        return;
    }

    // Register a trigger for this specific FilePicker using its id
    auto myId = configNode.getProperty(foleys::IDs::id, "").toString();
    if (myId.isNotEmpty()) {
        getMagicState().addTrigger("filepicker_" + myId, [this]() { openFileChooser(); });
        selectedPath = getMagicState().getValueTree().getProperty("filepath_" + myId, "").toString();
    }

    updateButtonText();
}

void FilePickerItem::update() {
    foleys::Container::update();

    // Re-read path from ValueTree in case it changed externally
    auto myId = configNode.getProperty(foleys::IDs::id, "").toString();
    if (myId.isNotEmpty()) {
        auto newPath = getMagicState().getValueTree().getProperty("filepath_" + myId, "").toString();
        if (newPath != selectedPath) {
            selectedPath = newPath;
            updateButtonText();
        }
    }
}

juce::TextButton* FilePickerItem::getButton() const {
    if (buttonItem == nullptr) {
        return nullptr;
    }
    return dynamic_cast<juce::TextButton*>(buttonItem->getWrappedComponent());
}

void FilePickerItem::openFileChooser() {
    auto filterPattern = magicBuilder.getStyleProperty(pFileFilter, configNode).toString();
    if (filterPattern.isEmpty()) {
        filterPattern = "*";
    }

    auto startDir = selectedPath.isNotEmpty()
                        ? juce::File(selectedPath).getParentDirectory()
                        : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);

    fileChooser = std::make_unique<juce::FileChooser>("Select a file", startDir, filterPattern);

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            auto result = fc.getResult();
            if (result.existsAsFile()) {
                selectedPath = result.getFullPathName();
                updateButtonText();

                // Save to ValueTree using our id
                auto myId = configNode.getProperty(foleys::IDs::id, "").toString();
                if (myId.isNotEmpty()) {
                    getMagicState().getValueTree().setProperty("filepath_" + myId, selectedPath,
                                                               nullptr);
                }
            }
        });
}

void FilePickerItem::updateButtonText() {
    if (buttonItem == nullptr) {
        return;
    }

    // Set the "text" property on the button's config node so Foleys applies it during update()
    static const juce::Identifier pText{"text"};
    juce::String newText = selectedPath.isEmpty() ? "Browse..." : juce::File(selectedPath).getFileName();

    if (configNode.getNumChildren() > 0) {
        configNode.getChild(0).setProperty(pText, newText, nullptr);
    }
}