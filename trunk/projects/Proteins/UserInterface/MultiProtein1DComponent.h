/*-----------------------------------------.---------------------------------.
| Filename: MultiProtein1DComponent.h      | Component for protein           |
| Author  : Francis Maes                   |                   1D sequences  |
| Started : 22/06/2010 15:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_USER_INTERFACE_MULTI_1D_H_
# define LBCPP_PROTEINS_USER_INTERFACE_MULTI_1D_H_

# include "MultiSequenceComponent.h"
# include <lbcpp/UserInterface/ObjectEditor.h>
# include "../Data/Protein.h"
# include "../Inference/ProteinInferenceFactory.h"

namespace lbcpp
{

class MultiProtein1DConfiguration : public Object
{
public:
  MultiProtein1DConfiguration(const std::vector<String>& proteinNames, const std::vector< std::pair<String, size_t> >& sequenceIndex)
  {
    proteins.resize(proteinNames.size());
    for (size_t i = 0; i < proteins.size(); ++i)
      proteins[i] = std::make_pair(proteinNames[i], true);
    sequences.resize(sequenceIndex.size());
    for (size_t i = 0; i < sequences.size(); ++i)
    {
      sequences[i].index = sequenceIndex[i].second;
      sequences[i].friendlyName = sequenceIndex[i].first;
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
    size_t index;
    String friendlyName;
    bool activated;
    Colour colour;
  };
  
  SequenceConfiguration& getSequence(size_t index)
    {jassert(index < sequences.size()); return sequences[index];}
  
  size_t getSequenceIndex(size_t index) const
    {jassert(index < sequences.size()); return sequences[index].index;}

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
      int x = (int)i * buttonWidth;
      for (size_t j = 0; j < buttons[i].size(); ++j)
        buttons[i][j]->setBounds(x, (int)j * buttonHeight, buttonWidth, buttonHeight);
    }
  }

  enum {buttonWidth = 200, buttonHeight = 20};

private:
  MultiProtein1DConfigurationPtr configuration;
  std::vector< std::vector<ConfigurationButton* > > buttons;
};

class MultiProtein1DComponent : public ObjectEditor, public VariableSelector, public VariableSelectorCallback
{
public:
  MultiProtein1DComponent(const std::vector<ProteinPtr>& proteins, MultiProtein1DConfigurationPtr configuration)
    : ObjectEditor(ObjectPtr(), configuration, true, false), proteins(proteins)
    {initialize();}

  virtual Component* createConfigurationComponent(const ObjectPtr& configuration)
    {return new MultiProtein1DConfigurationComponent(configuration);}

  virtual Component* createContentComponent(const ObjectPtr& object, const ObjectPtr& cfg)
  {
    MultiProtein1DConfigurationPtr configuration = cfg.staticCast<MultiProtein1DConfiguration>();
    MultiSequenceComponent* component = new MultiSequenceComponent();
    component->addCallback(*this);
    for (size_t i = 0; i < configuration->getNumSequences(); ++i)
      if (configuration->isSequenceEnabled(i))
      {
        int sequenceIndex = (int)configuration->getSequenceIndex(i);
        String sequenceFriendlyName = configuration->getSequenceFriendlyName(i);
        std::vector< std::pair<String, ContainerPtr> > sequences;
        for (size_t j = 0; j < configuration->getNumProteins(); ++j)
          if (configuration->isProteinEnabled(j))
          {
            VectorPtr sequence = proteins[j]->getVariable(sequenceIndex).getObjectAndCast<Container>();
            if (sequence)
              sequences.push_back(std::make_pair(configuration->getProteinName(j), sequence));
          }
        if (sequences.size())
          component->addSequenceGroup(sequenceFriendlyName, sequences);
      }

    return component;
  }

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables, const String& selectionName)
  {
    std::vector<Variable> selection;
    selection.resize(selectedVariables.size());
    for (size_t i = 0; i < selection.size(); ++i)
      selection[i] = makeSelection(selectedVariables[i]);
    sendSelectionChanged(selection, selectionName);
    MultiSequenceComponent* component = dynamic_cast<MultiSequenceComponent* >(getContentComponent());
    if (component)
    {
      if (selectedVariables.size() == 1)
        component->setSelection(selectedVariables[0][1].getInteger(), String::empty, String::empty);
      else
        component->setNoSelection();
    }
  }
  
protected:
  std::vector<ProteinPtr> proteins;

  Variable makeSelection(const Variable& sequenceVariable) const
  {
    jassert(sequenceVariable.getType() == pairClass(pairClass(stringType, stringType), integerType));
    Variable names = sequenceVariable[0];
    //String sequenceName = names[0].getString();
    ProteinPtr protein = findProteinWithName(names[1].getString());
    jassert(protein);
    int position = sequenceVariable[1].getInteger();
    if (position < 0)
      return protein;
    else
      return Variable::pair(protein, (size_t)position);
  }
 
  ProteinPtr findProteinWithName(const String& proteinName) const
  {
    MultiProtein1DConfigurationPtr configuration = this->configuration.staticCast<MultiProtein1DConfiguration>();
    for (size_t i = 0; i < configuration->getNumProteins(); ++i)
      if (configuration->getProteinName(i) == proteinName)
        return proteins[i];
    return ProteinPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_USER_INTERFACE_MULTI_1D_H_
