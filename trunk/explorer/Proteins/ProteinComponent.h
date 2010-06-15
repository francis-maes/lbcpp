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
# include "../Utilities/SplittedLayout.h"
# include "../../projects/Proteins/Protein/Protein.h"

namespace lbcpp
{

class MultiProtein1DConfiguration : public Object
{
public:
  MultiProtein1DConfiguration(const std::vector<String>& proteinNames, const std::vector< std::pair<String, String> >& sequenceNames)
  {
    proteins.resize(proteinNames.size());
    for (size_t i = 0; i < proteins.size(); ++i)
      proteins[i] = std::make_pair(proteinNames[i], true);
    sequences.resize(sequenceNames.size());
    for (size_t i = 0; i < sequences.size(); ++i)
    {
      sequences[i].name = sequenceNames[i].second;
      sequences[i].friendlyName = sequenceNames[i].first;
      sequences[i].activated = true;
      sequences[i].colour = Colours::white;
    }
  }
  
  size_t getNumProteins() const
    {return proteins.size();}
  
  std::pair<String, bool>& getProtein(size_t index)
    {jassert(index < proteins.size()); return proteins[index];}
  
  String getProteinName(size_t index) const
    {jassert(index < proteins.size()); return proteins[index].first;}
  
  bool isProteinEnabled(size_t index) const
    {jassert(index < proteins.size()); return proteins[index].second;}
  
  size_t getNumSequences() const
    {return sequences.size();}

  struct SequenceConfiguration
  {
    String name;
    String friendlyName;
    bool activated;
    Colour colour;
  };
  
  SequenceConfiguration& getSequence(size_t index)
    {jassert(index < sequences.size()); return sequences[index];}
  
  String getSequenceName(size_t index) const
    {jassert(index < sequences.size()); return sequences[index].name;}

  String getSequenceFriendlyName(size_t index) const
    {jassert(index < sequences.size()); return sequences[index].friendlyName;}
  
  bool isSequenceEnabled(size_t index) const
    {jassert(index < sequences.size()); return sequences[index].activated;}
  
private:
  std::vector< std::pair<String, bool> > proteins;
  
  std::vector<SequenceConfiguration> sequences;
};

typedef ReferenceCountedObjectPtr<MultiProtein1DConfiguration> MultiProtein1DConfigurationPtr;

class MultiProtein1DConfigurationComponent : public Component, public juce::ButtonListener, public juce::ChangeBroadcaster
{
public:
  struct ConfigurationButton : public juce::ToggleButton
  {
    ConfigurationButton(const String& name, bool& value)
      : ToggleButton(name), value(value)
      {setToggleState(value, false);}
      
    bool& value;
  };

  MultiProtein1DConfigurationComponent(MultiProtein1DConfigurationPtr configuration)
    : configuration(configuration)
  {
    std::vector<ConfigurationButton* > buttonsColumn;
    for (size_t i = 0; i < configuration->getNumProteins(); ++i)
    {
      std::pair<String, bool>& p = configuration->getProtein(i);
      buttonsColumn.push_back(new ConfigurationButton(p.first, p.second));
    }
    buttons.push_back(buttonsColumn);
    
    buttonsColumn.clear();
    for (size_t i = 0; i < configuration->getNumSequences(); ++i)
    {
      MultiProtein1DConfiguration::SequenceConfiguration& p = configuration->getSequence(i);
      buttonsColumn.push_back(new ConfigurationButton(p.friendlyName, p.activated));
      if (buttonsColumn.size() >= 4)
      {
        buttons.push_back(buttonsColumn);
        buttonsColumn.clear();
      }
    }
    if (buttonsColumn.size())
      buttons.push_back(buttonsColumn);

    for (size_t i = 0; i < buttons.size(); ++i)
      for (size_t j = 0; j < buttons[i].size(); ++j)
      {
        ConfigurationButton* button = buttons[i][j];
        button->addButtonListener(this);
        addAndMakeVisible(button);
      }
  }

  virtual ~MultiProtein1DConfigurationComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
  {
    ((ConfigurationButton* )button)->value = button->getToggleState();
    sendChangeMessage(this);
  }

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
  MultiProtein1DConfigurationPtr configuration;
  std::vector< std::vector<ConfigurationButton* > > buttons;
};

class MultiProtein1DComponent : public SplittedLayout, public juce::ChangeListener
{
public:
  MultiProtein1DComponent(const std::vector<ProteinPtr>& proteins, MultiProtein1DConfigurationPtr configuration)
    : SplittedLayout(new MultiProtein1DConfigurationComponent(configuration), new ViewportComponent(NULL, true, false), 0.16, SplittedLayout::typicalVertical),
      proteins(proteins), configuration(configuration)
  {
    dynamic_cast<juce::ChangeBroadcaster* >(first)->addChangeListener(this);
    changeListenerCallback(NULL);
  }
  
  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    MultiSequenceComponent* component = new MultiSequenceComponent();
    for (size_t i = 0; i < configuration->getNumSequences(); ++i)
      if (configuration->isSequenceEnabled(i))
      {
        String sequenceName = configuration->getSequenceName(i);
        String sequenceFriendlyName = configuration->getSequenceFriendlyName(i);
        std::vector< std::pair<String, ObjectContainerPtr> > sequences;
        for (size_t j = 0; j < configuration->getNumProteins(); ++j)
          if (configuration->isProteinEnabled(j))
          {
            ObjectContainerPtr sequence = proteins[j]->getObject(sequenceName).dynamicCast<ObjectContainer>();
            if (sequence)
              sequences.push_back(std::make_pair(configuration->getProteinName(j), sequence));
          }
        if (sequences.size())
          component->addSequenceGroup(sequenceFriendlyName, sequences);
      }
    getViewport()->setViewedComponent(component);
    getViewport()->resized();
  }
  
protected:
  std::vector<ProteinPtr> proteins;
  MultiProtein1DConfigurationPtr configuration;

  ViewportComponent* getViewport() const
    {return (ViewportComponent* )second;}
};

class MultiProteinComponent : public TabbedComponent
{
public:
  MultiProteinComponent(const std::vector< std::pair<String, ProteinPtr> >& proteins)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom), proteins(proteins)
  {
    std::vector<ProteinPtr> proteinObjects(proteins.size());
    std::vector<String> proteinNames(proteins.size());
    for (size_t i = 0; i < proteinNames.size(); ++i)
    {
      proteinNames[i] = proteins[i].first;
      proteinObjects[i] = proteins[i].second;
    }

    std::vector< std::pair<String, String> > sequenceNames;
    addObjectNameIfExists(T("Primary structure"), T("AminoAcidSequence"), proteins, sequenceNames);
    addObjectNameIfExists(T("PSSM"), T("PositionSpecificScoringMatrix"), proteins, sequenceNames);
    addObjectNameIfExists(T("Secondary structure 3"), T("SecondaryStructureSequence"), proteins, sequenceNames);
    addObjectNameIfExists(T("Secondary structure 3 prob"), T("SecondaryStructureProbabilities"), proteins, sequenceNames);
    addObjectNameIfExists(T("Secondary structure 8"), T("DSSPSecondaryStructureSequence"), proteins, sequenceNames);
    addObjectNameIfExists(T("Solvent accesibility"), T("NormalizedSolventAccessibilitySequence"), proteins, sequenceNames);
    addObjectNameIfExists(T("Solvent accesibility 20%"), T("SolventAccessibilityThreshold20"), proteins, sequenceNames);    
    addObjectNameIfExists(T("Solvent accesibility 25%"), T("SolventAccessibilityThreshold25"), proteins, sequenceNames);    
    addObjectNameIfExists(T("Disorder region"), T("DisorderSequence"), proteins, sequenceNames);
    addObjectNameIfExists(T("Disorder region prob"), T("DisorderProbabilitySequence"), proteins, sequenceNames);
    addObjectNameIfExists(T("Structural alphabet"), T("StructuralAlphabetSequence"), proteins, sequenceNames);

    MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(proteinNames, sequenceNames);
    addTab(T("Protein 1D"), Colours::white, new MultiProtein1DComponent(proteinObjects, configuration), true);
  }
    
  juce_UseDebuggingNewOperator

private:
  std::vector< std::pair<String, ProteinPtr> > proteins;

  void addObjectNameIfExists(const String& friendlyName, const String& name, const std::vector< std::pair<String, ProteinPtr> >& proteins, std::vector< std::pair<String, String> >& res)
  {
    for (size_t i = 0; i < proteins.size(); ++i)
      if (proteins[i].second->getObject(name))
      {
        res.push_back(std::make_pair(friendlyName, name));
        return;
      }
  }  
};

class ProteinComponent : public MultiProteinComponent
{
public:
  ProteinComponent(ProteinPtr protein, const String& name)
    : MultiProteinComponent(std::vector< std::pair<String, ProteinPtr> >(1, std::make_pair(name, protein))) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

