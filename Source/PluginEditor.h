// Delia - editor
#pragma once
#include "PluginProcessor.h"
#include "Look.h"

namespace delia
{
class DeckView; class ReelPanel; class SplicePanel; class LabPanel; class HeaderBar; class BankDrawer; class CorridorView;

class DeliaEditor : public juce::AudioProcessorEditor, public juce::FileDragAndDropTarget, public juce::DragAndDropContainer,
                    private juce::ChangeListener, private juce::Timer
{
public:
    explicit DeliaEditor (DeliaProcessor&);
    ~DeliaEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override { return true; }
    void filesDropped (const juce::StringArray&, int, int) override;

    DeliaProcessor& proc;
    void refreshAll();
    void layoutForDrawer();
    static constexpr int baseW = 1240, baseH = 820, drawerH = 236;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override { refreshAll(); }
    void timerCallback() override;

    VintageLook look;
    juce::Image hammer;
    juce::TooltipWindow tooltips { this, 500 };
    std::unique_ptr<HeaderBar> header;
    std::unique_ptr<DeckView> decks[kNumMachines];
    std::unique_ptr<ReelPanel> reelPanel;
    std::unique_ptr<SplicePanel> splicePanel;
    std::unique_ptr<LabPanel> labPanel;
    std::unique_ptr<BankDrawer> drawer;
    std::unique_ptr<CorridorView> corridor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeliaEditor)
};
} // namespace delia
