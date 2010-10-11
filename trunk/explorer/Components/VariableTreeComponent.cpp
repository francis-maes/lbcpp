/*-----------------------------------------.---------------------------------.
| Filename: VariableTreeComponent.cpp      | Variable Tree component         |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "VariableTreeComponent.h"
using namespace lbcpp;

class VariableTreeViewItem : public SimpleTreeViewItem
{
public:
  VariableTreeViewItem(const String& name, const Variable& variable, const VariableTreeOptions& options)
    : SimpleTreeViewItem(name, NULL, true), 
      variable(variable), options(options), typeName(variable.getTypeName()), component(NULL)
  {
    shortSummary = variable.toShortString();

    TypePtr type = variable.getType();
    if (variable.exists() && variable.isObject())
    {
      ObjectPtr object = variable.getObject();
      subVariables.reserve(subVariables.size() + type->getObjectNumVariables());
      for (size_t i = 0; i < type->getObjectNumVariables(); ++i)
        addSubVariable(type->getObjectVariableName(i), object->getVariable(i));
    }

    subVariables.reserve(subVariables.size() + variable.size());
    for (size_t i = 0; i < variable.size(); ++i)
      addSubVariable(variable.getName(i), variable[i]);

    mightContainSubItemsFlag = !subVariables.empty();
  }

  virtual void itemSelectionChanged(bool isNowSelected)
  {
    VariableTreeComponent* owner = dynamic_cast<VariableTreeComponent* >(getOwnerView());
    jassert(owner);
    owner->invalidateSelection();
  }
  
  virtual void createSubItems()
  {
    for (size_t i = 0; i < subVariables.size(); ++i)
      addSubItem(new VariableTreeViewItem(subVariables[i].first, subVariables[i].second, options));
  }

  Variable getVariable() const
    {return variable;}
  
  virtual void paintItem(Graphics& g, int width, int height)
  {
    if (isSelected())
      g.fillAll(Colours::lightgrey);
    g.setColour(Colours::black);
    int x1 = 0;
    if (iconToUse)
    {
      g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);
      x1 += iconToUse->getWidth() + 5;
    }

    int numFields = 1;
    if (options.showTypes) ++numFields;
    if (options.showShortSummaries) ++numFields;

    int typeAndNameLength;
    enum {wantedLength = 300};
    int remainingWidth = width - x1;
    if (remainingWidth >= numFields * wantedLength)
      typeAndNameLength = wantedLength;
    else
      typeAndNameLength = remainingWidth / numFields;

    g.setFont(Font(12, numFields == 3 ? Font::bold : Font::plain));
    g.drawText(getUniqueName(), x1, 0, typeAndNameLength - 5, height, Justification::centredLeft, true);
    x1 += typeAndNameLength;
    if (options.showTypes)
    {
      g.setFont(Font(12, Font::italic));
      g.drawText(typeName, x1, 0, typeAndNameLength - 5, height, Justification::centredLeft, true);
      x1 += typeAndNameLength;
    }

    if (options.showShortSummaries && shortSummary.isNotEmpty())
    {
      g.setFont(Font(12));
      g.drawText(shortSummary, x1, 0, width - x1 - 2, height, Justification::centredLeft, true);
    }
  }

  juce_UseDebuggingNewOperator

protected:
  Variable variable;
  const VariableTreeOptions& options;
  String typeName;
  String shortSummary;
  Component* component;

  std::vector< std::pair<String, Variable> > subVariables;

  void addSubVariable(const String& name, const Variable& variable)
  {
    if (variable.exists() || options.showMissingVariables)
      subVariables.push_back(std::make_pair(name, variable));
  }
};

/*
** VariableTreeComponent
*/
VariableTreeComponent::VariableTreeComponent(const Variable& variable, const String& name, const VariableTreeOptions& options)
  : variable(variable), name(name), options(options), root(NULL), isSelectionUpToDate(false)
{
  setRootItemVisible(true);
  setWantsKeyboardFocus(true);
  setMultiSelectEnabled(true);
  buildTree();
  root->setSelected(true, true);
  startTimer(100);  
}

VariableTreeComponent::~VariableTreeComponent()
  {clearTree();}

bool VariableTreeComponent::keyPressed(const juce::KeyPress& key)
{
  if (key.getKeyCode() == juce::KeyPress::F5Key)
  {
    clearTree(), buildTree();
    return true;
  }
  return juce::TreeView::keyPressed(key);
}

void VariableTreeComponent::clearTree()
{
  if (root)
  {
    deleteRootItem();
    root = NULL;
  }    
}

void VariableTreeComponent::buildTree()
{
  root = new VariableTreeViewItem(name, variable, options);
  setRootItem(root);
  root->setOpen(true);
}

void VariableTreeComponent::paint(Graphics& g)
{
  g.fillAll(Colours::white);
  juce::TreeView::paint(g);
}

void VariableTreeComponent::timerCallback()
{
  if (!isSelectionUpToDate)
  {
    std::vector<Variable> selectedVariables;
    selectedVariables.reserve(getNumSelectedItems());
    for (int i = 0; i < getNumSelectedItems(); ++i)
    {
      VariableTreeViewItem* item = dynamic_cast<VariableTreeViewItem* >(getSelectedItem(i));
      if (item && item->getVariable().exists() && item != root)
        selectedVariables.push_back(item->getVariable());
    }
    sendSelectionChanged(selectedVariables);
    isSelectionUpToDate = true;
  }
}

void VariableTreeComponent::invalidateSelection()
  {isSelectionUpToDate = false;}

int VariableTreeComponent::getDefaultWidth() const
{
  int numFields = 1;
  if (options.showTypes) ++numFields;
  if (options.showShortSummaries) ++numFields;
  return numFields * 200;
}

