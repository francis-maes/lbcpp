/*-----------------------------------------.---------------------------------.
| Filename: MultiProtein2DComponent.h      | Component for protein           |
| Author  : Francis Maes                   |                2D contact maps  |
| Started : 22/06/2010 15:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_PROTEIN_MULTI_2D_COMPONENT_H_
# define EXPLORER_PROTEIN_MULTI_2D_COMPONENT_H_

# include "../../../explorer/Utilities/SplittedLayout.h"
# include "../Data/Protein.h"

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

class ContactMapComponent : public Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  ContactMapComponent(SymmetricMatrixPtr map1, SymmetricMatrixPtr map2)
    : map1(map1), map2(map2), n(0), selectedX(-1), selectedY(-1)
  {
    jassert(!map1 || !map2 || map1->getDimension() == map2->getDimension());
    if (map1)
      n = map1->getDimension();
    else if (map2)
      n = map2->getDimension();
  }
  
  int computePixelsPerEntry(int availableWidth, int availableHeight) const
  {
    int availableSize = juce::jmin(availableWidth, availableHeight);
    return n ? juce::jmax(1, availableSize / (int)n) : 4;
  }
  
  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    int pixelsPerEntry = computePixelsPerEntry(availableWidth, availableHeight);
    return juce::jmax(pixelsPerEntry * (int)n, availableWidth);
  }

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    int pixelsPerEntry = computePixelsPerEntry(availableWidth, availableHeight);
    return juce::jmax(pixelsPerEntry * (int)n, availableHeight);
  }

  virtual void paint(Graphics& g)
  {
    int pixelsPerEntry, x1, y1;
    getPaintCoordinates(pixelsPerEntry, x1, y1);
  
    for (int i = 0; i < (int)n; ++i)
      for (int j = i; j < (int)n; ++j)
      {
        paintEntry(g, x1 + i * pixelsPerEntry, y1 + j * pixelsPerEntry, pixelsPerEntry, map1, i, j);
        paintEntry(g, x1 + j * pixelsPerEntry, y1 + i * pixelsPerEntry, pixelsPerEntry, map2, i, j);
      }
      
    if (selectedX >= 0 && selectedY >= 0)
    {
      g.setColour(Colours::lightblue.withAlpha(0.7f));
      g.fillRect(x1 + (selectedX - 4) * pixelsPerEntry, y1 + selectedY * pixelsPerEntry, 9 * pixelsPerEntry - 1, pixelsPerEntry - 1);
      g.fillRect(x1 + selectedX * pixelsPerEntry, y1 + (selectedY - 4) * pixelsPerEntry, pixelsPerEntry - 1, 9 * pixelsPerEntry - 1);
    }

    g.setColour(Colours::black);
    g.drawRect(x1, y1, (int)n * pixelsPerEntry, (int)n * pixelsPerEntry);
    g.drawLine((float)x1, (float)y1, (float)(x1 + n * pixelsPerEntry), (float)(y1 + n * pixelsPerEntry));
  }

  virtual void mouseUp(const MouseEvent& e)
  {
    int pixelsPerEntry, x1, y1;
    getPaintCoordinates(pixelsPerEntry, x1, y1);
    jassert(pixelsPerEntry);
    int x = (e.getMouseDownX() - x1) / pixelsPerEntry;
    int y = (e.getMouseDownY() - y1) / pixelsPerEntry;
    if (x >= 0 && y >= 0 && x < (int)n && y < (int)n)
    {
      SymmetricMatrixPtr map = x <= y ? map1 : map2;
      if (map)
      {
        selectedX = x, selectedY = y;
        sendSelectionChanged(Variable::pair(map, Variable::pair((size_t)x, (size_t)y)), T("residue pair"));
        repaint();
        return;
      }
    }

    selectedX = -1, selectedY = -1;
    sendSelectionChanged(std::vector<Variable>(), String::empty);
    repaint();
  }
  
private:
  SymmetricMatrixPtr map1;
  SymmetricMatrixPtr map2;
  size_t n;
  int selectedX, selectedY;
    
  void getPaintCoordinates(int& pixelsPerEntry, int& x1, int& y1) const
  {
    pixelsPerEntry = computePixelsPerEntry(getWidth(), getHeight());
    x1 = (getWidth() - pixelsPerEntry * (int)n) / 2;
    y1 = (getHeight() - pixelsPerEntry * (int)n) / 2;
  }

  static Colour selectColour(SymmetricMatrixPtr map, size_t i, size_t j)
  {
    if (map->getElement(i, j).exists())
    {
      double probability = map->getElement(i, j).getDouble();
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
  
  void paintEntry(Graphics& g, int x1, int y1, int pixelsPerEntry, SymmetricMatrixPtr map, size_t i, size_t j)
  {
    if (map)
    {
      g.setColour(selectColour(map, i, j));
      g.fillRect(x1, y1, pixelsPerEntry, pixelsPerEntry);
    }
  }
};

class MultiProtein2DComponent : public Component, public juce::ChangeListener, public ComponentWithPreferedSize, public VariableSelector, public VariableSelectorCallback
{
public:
  MultiProtein2DComponent(const std::vector<ProteinPtr>& proteins, MultiProtein2DConfigurationPtr configuration)
    : proteins(proteins), configuration(configuration)
  {
    addAndMakeVisible(configurationComponent = new MultiProtein2DConfigurationComponent(configuration));
    configurationComponent->addChangeListener(this);
    addAndMakeVisible(viewport = new ViewportComponent(NULL, true, true));
    changeListenerCallback(NULL);
  }
  
  virtual ~MultiProtein2DComponent()
    {deleteAllChildren();}
  
  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    SymmetricMatrixPtr map1 = getMap(configuration->getProtein1());
    SymmetricMatrixPtr map2 = getMap(configuration->getProtein2());
    ContactMapComponent* contactMapComponent = new ContactMapComponent(map1, map2);
    contactMapComponent->addCallback(*this);
    viewport->setViewedComponent(contactMapComponent);
    viewport->resized();
  }
  
  virtual void resized()
  {
    enum {configurationHeight = 20};
    configurationComponent->setBounds(0, 0, getWidth(), configurationHeight);
    viewport->setBounds(0, configurationHeight, getWidth(), getHeight() - configurationHeight);
  }
 
  virtual int getDefaultWidth() const
    {return 900;}

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables, const String& selectionName)
  {
    std::vector<Variable> selection;
    selection.resize(selectedVariables.size());
    for (size_t i = 0; i < selection.size(); ++i)
      selection[i] = makeSelection(selectedVariables[i]);
    sendSelectionChanged(selection, selectionName);
  }

protected:
  MultiProtein2DConfigurationComponent* configurationComponent;
  ViewportComponent* viewport;
  
  std::vector<ProteinPtr> proteins;
  MultiProtein2DConfigurationPtr configuration;

  SymmetricMatrixPtr getMap(int proteinNumber) const
  {
    int currentMap = configuration->getCurrentMap();
    return currentMap < 0 || proteinNumber < 0
      ? SymmetricMatrixPtr()
      : proteins[proteinNumber]->getVariable(configuration->getMapIndex(currentMap)).getObjectAndCast<SymmetricMatrix>();
  }

  Variable makeSelection(const Variable& input) const
  {
    SymmetricMatrixPtr contactMap = input[0].getObjectAndCast<SymmetricMatrix>();

    for (size_t i = 0; i < proteins.size(); ++i)
      if (contactMap == getMap((int)i))
        return Variable::pair(proteins[i], input[1]);
    jassert(false);
    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !EXPLORER_PROTEIN_MULTI_2D_COMPONENT_H_

