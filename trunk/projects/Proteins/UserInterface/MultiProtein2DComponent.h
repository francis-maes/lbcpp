/*-----------------------------------------.---------------------------------.
| Filename: MultiProtein2DComponent.h      | Component for protein           |
| Author  : Francis Maes                   |                2D contact maps  |
| Started : 22/06/2010 15:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_PROTEIN_MULTI_2D_COMPONENT_H_
# define EXPLORER_PROTEIN_MULTI_2D_COMPONENT_H_

# include <lbcpp/Data/Matrix.h>
# include <lbcpp/UserInterface/ObjectEditor.h>
# include <lbcpp/UserInterface/MatrixComponent.h>
# include "../Data/Protein.h"
# include "../Predictor/ProteinPredictor.h"

namespace lbcpp
{

class MultiProtein2DConfiguration : public Object
{
public:
  MultiProtein2DConfiguration(const std::vector<String>& proteinNames, const std::vector< std::pair<String, size_t> >& mapIndex)
    : protein1(proteinNames.size() > 0 ? 0 : -1), protein2(proteinNames.size() > 1 ? 1 : -1), currentMap(mapIndex.size() > 0 ? 0 : -1), proteinNames(proteinNames), mapIndex(mapIndex)
   {}
   
  /*
  ** Proteins
  */
  size_t getNumProteins() const
    {return proteinNames.size();}
    
  String getProteinName(size_t index) const
    {jassert(index < proteinNames.size()); return proteinNames[index];}
    
  int getProtein1() const
    {return protein1;}

  void setProtein1(int c)
    {protein1 = c;}

  int getProtein2() const
    {return protein2;}
    
  void setProtein2(int c)
    {protein2 = c;}
    
  /*
  ** Contact Maps
  */
  size_t getNumMaps() const
    {return mapIndex.size();}
    
  String getMapFriendlyName(size_t index) const
    {jassert(index < mapIndex.size()); return mapIndex[index].first;}

  size_t getMapIndex(size_t index) const
    {jassert(index < mapIndex.size()); return mapIndex[index].second;}
  
  int getCurrentMap() const
    {return currentMap;}
    
  void setCurrentMap(int c)
    {currentMap = c;}
  
protected:
  int protein1, protein2;
  int currentMap;
  std::vector< String > proteinNames;
  std::vector< std::pair<String, size_t> > mapIndex;
};

typedef ReferenceCountedObjectPtr<MultiProtein2DConfiguration> MultiProtein2DConfigurationPtr;

class MultiProtein2DConfigurationComponent : public Component, public juce::ChangeBroadcaster, public juce::ComboBoxListener
{
public:
  MultiProtein2DConfigurationComponent(MultiProtein2DConfigurationPtr configuration)
    : configuration(configuration)
  {
    addAndMakeVisible(protein1 = createProteinComboBox());
    protein1->setSelectedItemIndex(configuration->getProtein1(), false);
    addAndMakeVisible(protein2 = createProteinComboBox());
    protein2->setSelectedItemIndex(configuration->getProtein2(), false);
    addAndMakeVisible(map = createMapComboBox());
    map->setSelectedItemIndex(configuration->getCurrentMap(), false);
  }
  
  virtual ~MultiProtein2DConfigurationComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    protein1->setBoundsRelative(0, 0, 0.333f, 1.f);
    protein2->setBoundsRelative(0.333f, 0, 0.333f, 1.f);
    map->setBoundsRelative(0.666f, 0, 0.333f, 1.f);
    //setSize(getWidth(), 20);
  }

  virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == protein1)
      configuration->setProtein1(protein1->getSelectedItemIndex());
    else if (comboBoxThatHasChanged == protein2)
      configuration->setProtein2(protein2->getSelectedItemIndex());
    else if (comboBoxThatHasChanged == map)
      configuration->setCurrentMap(map->getSelectedItemIndex());
    sendChangeMessage(this);
  }
  
private:
  MultiProtein2DConfigurationPtr configuration;
  
  juce::ComboBox* protein1;
  juce::ComboBox* protein2;
  juce::ComboBox* map;

  juce::ComboBox* createProteinComboBox()
  {
    juce::ComboBox* res = new juce::ComboBox(T("ProteinObject"));
    for (size_t i = 0; i < configuration->getNumProteins(); ++i)
      res->addItem(configuration->getProteinName(i), (int)i + 1);
    res->addListener(this);
    return res;
  }

  juce::ComboBox* createMapComboBox()
  {
    juce::ComboBox* res = new juce::ComboBox(T("Map"));
    for (size_t i = 0; i < configuration->getNumMaps(); ++i)
      res->addItem(configuration->getMapFriendlyName(i), (int)i + 1);
    res->addListener(this);
    return res;
  }
};

class ContactMapComponent : public MatrixComponent
{
public:
  ContactMapComponent(MatrixPtr matrix)
    : MatrixComponent(matrix) {}
  
  virtual juce::Colour selectColour(const Variable& element)
  {
    if (element.exists())
    {
      double probability = element.getDouble();
      if (probability > 0.5)
      {
        // red
        unsigned char c = (unsigned char)((probability) * 255);
        return Colour(255, 255 - c, 255 - c);
      }
      else
      {
        // blue
        unsigned char c = (unsigned char)((1.0 - probability) * 255);
        return Colour(255 - c, 255 - c, 255);
      }
    }
    else
      return Colours::lightgrey;
  }
};

class MultiProtein2DComponent : public ObjectEditor, public VariableSelector, public VariableSelectorCallback
{
public:
  MultiProtein2DComponent(ExecutionContext& context, const std::vector<ProteinPtr>& proteins, MultiProtein2DConfigurationPtr configuration)
    : ObjectEditor(ObjectPtr(), configuration, true, true), proteins(proteins)
    , features(std::vector<SymmetricMatrixPtr>(proteins.size()))
    , disulfideFeatures(std::vector<SymmetricMatrixPtr>(proteins.size()))
  {
    initialize();
    
    /* Initialize function */
    NumericalProteinFeaturesParametersPtr featuresParameters = new NumericalProteinFeaturesParameters();
    ProteinPredictorParametersPtr predictorParameters = numericalProteinPredictorParameters(featuresParameters, new StochasticGDParameters());

    FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
    proteinfunction->initialize(context, (TypePtr)proteinClass);

    FunctionPtr residuefunction = predictorParameters->createResiduePairVectorPerception();
    residuefunction->initialize(context, proteinfunction->getOutputType());

    FunctionPtr disulfidResiduefunction = predictorParameters->createDisulfideResiduePairVectorPerception();
    residuefunction->initialize(context, proteinfunction->getOutputType());
    
    for (size_t i = 0; i < proteins.size(); ++i)
    {
      Variable proteinPerception = proteinfunction->compute(context, proteins[i]);
      features[i] = residuefunction->compute(context, proteinPerception).getObjectAndCast<SymmetricMatrix>();
      disulfideFeatures[i] = disulfidResiduefunction->compute(context, proteinPerception).getObjectAndCast<SymmetricMatrix>();
    }
  }
  
  virtual Component* createConfigurationComponent(const ObjectPtr& configuration)
    {return new MultiProtein2DConfigurationComponent(configuration);}
  
  virtual size_t getConfigurationComponentHeight() const
    {return 20;}

  virtual Component* createContentComponent(const ObjectPtr& object, const ObjectPtr& cfg)
  {
    const MultiProtein2DConfigurationPtr& configuration = cfg.staticCast<MultiProtein2DConfiguration>();
    SymmetricMatrixPtr map1 = getMap(configuration, configuration->getProtein1());
    SymmetricMatrixPtr map2 = getMap(configuration, configuration->getProtein2());
    if (!map2)
      map2 = zeroSymmetricMatrix(map1->getDimension());
    MatrixPtr contactMap = upperLowerSquareMatrix(map1->getElementsType(), map2, map1);
    ContactMapComponent* contactMapComponent = new ContactMapComponent(contactMap);
    contactMapComponent->addCallback(*this);
    return contactMapComponent;
  }

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables, const String& selectionName)
  {
    std::vector<Variable> selection;
    selection.resize(selectedVariables.size());
    for (size_t i = 0; i < selection.size(); ++i)
      selection[i] = makeSelection(selectedVariables[i]);
    sendSelectionChanged(selection, selectionName);
  }

protected:
  std::vector<ProteinPtr> proteins;
  std::vector<SymmetricMatrixPtr> features;
  std::vector<SymmetricMatrixPtr> disulfideFeatures;

  SymmetricMatrixPtr getMap(const MultiProtein2DConfigurationPtr& configuration, int proteinNumber) const
  {
    int currentMap = configuration->getCurrentMap();
    return currentMap < 0 || proteinNumber < 0
      ? SymmetricMatrixPtr()
      : proteins[proteinNumber]->getVariable(configuration->getMapIndex(currentMap)).getObjectAndCast<SymmetricMatrix>();
  }

  Variable makeSelection(const Variable& input) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    const MultiProtein2DConfigurationPtr& configuration = this->configuration.staticCast<MultiProtein2DConfiguration>();
    MatrixPtr contactMap = pair->getFirst().getObjectAndCast<Matrix>();

    PairPtr position = pair->getSecond().getObject();
    const size_t row = position->getFirst().getInteger();
    const size_t column = position->getSecond().getInteger();

    String mapName = configuration->getMapFriendlyName(configuration->getCurrentMap());
    const std::vector<SymmetricMatrixPtr>& f = (mapName == T("Disulfide Bonds")) ? disulfideFeatures : features;
    size_t proteinIndex = (row < column) ? configuration->getProtein2() : configuration->getProtein1();
    return proteinIndex >= 0 ? f[proteinIndex]->getElement(row, column) : Variable();
  }
};

}; /* namespace lbcpp */

#endif // !EXPLORER_PROTEIN_MULTI_2D_COMPONENT_H_
