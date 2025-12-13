#pragma once

#include <foleys_gui_magic/foleys_gui_magic.h>

// A self-contained file picker with its own Label and TextButton.
// Does NOT use Foleys' Label - manages the juce::Label directly.
class FilePickerItem : public foleys::GuiItem {
   public:
    FOLEYS_DECLARE_GUI_FACTORY(FilePickerItem)

    static const juce::Identifier pFileFilter;

    FilePickerItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);
    ~FilePickerItem() override = default;

    void update() override;
    void resized() override;
    juce::Component* getWrappedComponent() override;
    std::vector<foleys::SettableProperty> getSettableProperties() const override;

   private:
    void openFileChooser();
    void updateLabelFromState();

    // Simple label with rounded border and path truncation from the left
    class RoundedLabel : public juce::Label {
       public:
        void paint(juce::Graphics& g) override {
            auto bounds = getLocalBounds().toFloat().reduced(1.0f);
            g.setColour(findColour(outlineColourId));
            g.drawRoundedRectangle(bounds, 4.0f, 2.0f);

            // Draw text - truncate from left if too long
            g.setColour(findColour(textColourId));
            auto font = getLookAndFeel().getLabelFont(*this);
            g.setFont(font);

            auto textBounds = getLocalBounds().reduced(8, 0);
            auto text = getText();

            // Measure text width using GlyphArrangement
            juce::GlyphArrangement glyphs;
            glyphs.addLineOfText(font, text, 0, 0);
            auto textWidth = glyphs.getBoundingBox(0, -1, false).getWidth();

            if (textWidth > textBounds.getWidth() && text.isNotEmpty()) {
                // Truncate from the left, show "..." prefix
                juce::String truncated = "..." + text;
                while (truncated.length() > 4) {
                    glyphs.clear();
                    glyphs.addLineOfText(font, truncated, 0, 0);
                    if (glyphs.getBoundingBox(0, -1, false).getWidth() <= textBounds.getWidth())
                        break;
                    truncated = "..." + truncated.substring(4);  // Remove char after "..."
                }
                text = truncated;
            }

            g.drawText(text, textBounds, juce::Justification::centredLeft, false);
        }
    };

    // Our own components - not managed by Foleys
    juce::Component container;
    RoundedLabel pathLabel;
    juce::TextButton browseButton{"Browse"};

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilePickerItem)
};