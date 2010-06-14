/*-----------------------------------------.---------------------------------.
| Filename: ProteinComponent.h             | Components for proteins         |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_
# define EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

# include "MultiSequenceComponent.h"
# include "../../projects/Proteins/Protein/Protein.h"

namespace lbcpp
{

class MultiProtein1DConfigurationComponent : public Component, public juce::ButtonListener, public juce::ChangeBroadcaster
{
public:
  struct SelectSequenceButton : public juce::ToggleButton
  {
    SelectSequenceButton(const String& friendlyName, const String& sequenceName)
      : ToggleButton(friendlyName) {}
  };

  MultiProtein1DConfigurationComponent(const std::vector< std::pair<String, ProteinPtr> >& proteins)
  {
    std::vector<juce::Button* > buttonsColumn;
    for (size_t i = 0; i < proteins.size(); ++i)
      buttonsColumn.push_back(new juce::ToggleButton(proteins[i].first));
    buttons.push_back(buttonsColumn);
    
    buttonsColumn.clear();
    buttonsColumn.push_back(new SelectSequenceButton(T("Primary Sequence"), T("AminoAcidSequence")));
    buttonsColumn.push_back(new SelectSequenceButton(T("PSSM"), T("PositionSpecificScoringMatrix")));
    buttonsColumn.push_back(new SelectSequenceButton(T("Secondary Structure 3"), T("SecondaryStructureSequence")));
    buttonsColumn.push_back(new SelectSequenceButton(T("Secondary Structure 8"), T("DSSPSecondaryStructureSequence")));
    buttons.push_back(buttonsColumn);
      
    // todo: continue...

    for (size_t i = 0; i < buttons.size(); ++i)
      for (size_t j = 0; j < buttons[i].size(); ++j)
      {
        juce::Button* button = buttons[i][j];
        button->setToggleState(true, false);
        button->addButtonListener(this);
        addAndMakeVisible(button);
      }
  }

  virtual ~MultiProtein1DConfigurationComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
    {sendChangeMessage(this);}

  virtual void paint(Graphics& g)
    {g.fillAll(Colour(240, 245, 250));}

  virtual void resized()
  {
    for (size_t i = 0; i < buttons.size(); ++i)
    {
      int x = i * buttonWidth;
      for (size_t j = 0; j < buttons[i].size(); ++j)
        buttons[i][j]->setBounds(x, j * buttonHeight, buttonWidth, buttonHeight);
    }
  }

  enum {buttonWidth = 200, buttonHeight = 20};

private:
  std::vector< std::vector< juce::Button* > > buttons;
};

class MultiProtein1DComponent : public SplittedLayout, public juce::ChangeListener
{
public:
  MultiProtein1DComponent(const std::vector< std::pair<String, ProteinPtr> >& proteins)
    : SplittedLayout(new MultiProtein1DConfigurationComponent(proteins), new Viewport(), 0.16, SplittedLayout::typicalVertical)
  {
    dynamic_cast<juce::ChangeBroadcaster* >(first)->addChangeListener(this);
  }
  
  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    /*if (second)
      delete second;
    MultiSequenceComponent* component = 
    second = new MultiSequence
    */
  }
};

class MultiProteinComponent : public TabbedComponent
{
public:
  MultiProteinComponent(const std::vector< std::pair<String, ProteinPtr> >& proteins)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom), proteins(proteins)
    {addTab(T("Protein 1D"), Colours::white, new MultiProtein1DComponent(proteins), true);}
    
  juce_UseDebuggingNewOperator

private:
  std::vector< std::pair<String, ProteinPtr> > proteins;
};

class ProteinComponent : public MultiProteinComponent
{
public:
  ProteinComponent(ProteinPtr protein, const String& name)
    : MultiProteinComponent(std::vector< std::pair<String, ProteinPtr> >(1, std::make_pair(name, protein))) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

