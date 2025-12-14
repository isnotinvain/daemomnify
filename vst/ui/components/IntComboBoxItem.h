#pragma once

#include <foleys_gui_magic/foleys_gui_magic.h>

// A simple ComboBox that binds to an integer ValueTree property
// Configurable via XML attributes: property, min-value, max-value
class IntComboBoxItem : public foleys::GuiItem {
   public:
    FOLEYS_DECLARE_GUI_FACTORY(IntComboBoxItem)

    static const juce::Identifier pProperty;
    static const juce::Identifier pMinValue;
    static const juce::Identifier pMaxValue;

    IntComboBoxItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);

    void update() override;
    juce::Component* getWrappedComponent() override;
    std::vector<foleys::SettableProperty> getSettableProperties() const override;

   private:
    juce::ComboBox comboBox;
    juce::Value boundValue;
    int minValue = 1;
    int maxValue = 16;

    void populateComboBox();
    void updateComboBoxFromValue();
};