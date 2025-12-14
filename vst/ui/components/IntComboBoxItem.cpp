#include "IntComboBoxItem.h"

const juce::Identifier IntComboBoxItem::pProperty{"property"};
const juce::Identifier IntComboBoxItem::pMinValue{"min-value"};
const juce::Identifier IntComboBoxItem::pMaxValue{"max-value"};

IntComboBoxItem::IntComboBoxItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
    : foleys::GuiItem(builder, node) {
    // Same color translations as built-in ComboBox so it inherits styling from magic.xml
    setColourTranslation({
        {"combo-background", juce::ComboBox::backgroundColourId},
        {"combo-text", juce::ComboBox::textColourId},
        {"combo-outline", juce::ComboBox::outlineColourId},
        {"combo-button", juce::ComboBox::buttonColourId},
        {"combo-arrow", juce::ComboBox::arrowColourId},
        {"combo-focused-outline", juce::ComboBox::focusedOutlineColourId},
        {"combo-menu-background", juce::PopupMenu::backgroundColourId},
        {"combo-menu-background-highlight", juce::PopupMenu::highlightedBackgroundColourId},
        {"combo-menu-text", juce::PopupMenu::textColourId},
        {"combo-menu-text-highlight", juce::PopupMenu::highlightedTextColourId},
    });

    addAndMakeVisible(comboBox);

    comboBox.onChange = [this]() {
        int selectedValue = comboBox.getSelectedId();
        boundValue.setValue(selectedValue);
    };
}

void IntComboBoxItem::update() {
    // Read min/max from XML attributes
    auto minVar = magicBuilder.getStyleProperty(pMinValue, configNode);
    auto maxVar = magicBuilder.getStyleProperty(pMaxValue, configNode);

    minValue = minVar.isVoid() ? 1 : static_cast<int>(minVar);
    maxValue = maxVar.isVoid() ? 16 : static_cast<int>(maxVar);

    populateComboBox();

    // Bind to property
    auto propertyName = configNode.getProperty(pProperty, "").toString();
    if (propertyName.isNotEmpty()) {
        auto& state = dynamic_cast<foleys::MagicProcessorState&>(magicBuilder.getMagicState());
        boundValue.referTo(state.getValueTree().getPropertyAsValue(propertyName, nullptr));
        updateComboBoxFromValue();
    }
}

void IntComboBoxItem::populateComboBox() {
    comboBox.clear(juce::dontSendNotification);
    for (int i = minValue; i <= maxValue; ++i) {
        comboBox.addItem(juce::String(i), i);
    }
}

void IntComboBoxItem::updateComboBoxFromValue() {
    int value = static_cast<int>(boundValue.getValue());
    if (value >= minValue && value <= maxValue) {
        comboBox.setSelectedId(value, juce::dontSendNotification);
    }
}

juce::Component* IntComboBoxItem::getWrappedComponent() {
    return &comboBox;
}

std::vector<foleys::SettableProperty> IntComboBoxItem::getSettableProperties() const {
    std::vector<foleys::SettableProperty> props;
    props.push_back({configNode, pProperty, foleys::SettableProperty::Text, "", {}});
    props.push_back({configNode, pMinValue, foleys::SettableProperty::Number, 1, {}});
    props.push_back({configNode, pMaxValue, foleys::SettableProperty::Number, 16, {}});
    return props;
}