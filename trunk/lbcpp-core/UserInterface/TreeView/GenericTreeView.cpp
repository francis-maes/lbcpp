/*-----------------------------------------.---------------------------------.
| Filename: GenericTreeView.h              | Base classes for tree views     |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2012 11:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "GenericTreeView.h"
#include <lbcpp/Core/Library.h>
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/library.h>
using namespace lbcpp;

static string getIconForType(const ClassPtr& type)
{
  string res = type->getName() + "-32.png";
  if (UserInterfaceManager().hasImage(res))
    return res;
  
  if (type->inheritsFrom(vectorClass()))
    return "Vector-32.png";

  return "Object-32.png";
}

static string getIconForObject(const ObjectPtr& object)
{
  if (!object)
    return string::empty;
  if (object.isInstanceOf<File>())
  {
    juce::File file = File::get(object);
    LoaderPtr loader = lbcpp::getTopLevelLibrary()->findLoaderForFile(defaultExecutionContext(), file);
    if (loader)
      return getIconForType(loader->getTargetClass());
  }
  if (object.isInstanceOf<ExecutionTraceItem>())
    return object.staticCast<ExecutionTraceItem>()->getPreferedIcon();
  return getIconForType(object->getClass());
}

/*
** GenericTreeViewItem
*/
GenericTreeViewItem::GenericTreeViewItem(GenericTreeView* owner, ObjectPtr object, const string& uniqueName)
  : owner(owner), object(object), uniqueName(uniqueName), iconToUse(NULL), hasBeenOpened(false)
{
  string icon = getIconForObject(object);
  if (icon.isNotEmpty())
    iconToUse = userInterfaceManager().getImage(icon, defaultIconSize, defaultIconSize);
}
     
bool GenericTreeViewItem::mightContainSubItems()
  {return owner->mightHaveSubObjects(object);}

const string GenericTreeViewItem::getUniqueName() const
  {return uniqueName;}

void GenericTreeViewItem::paintItem(juce::Graphics& g, int width, int height)
{
  if (isSelected())
    g.fillAll(juce::Colours::lightgrey);
  g.setColour(juce::Colours::black);
  int x1 = 0;
  if (iconToUse)
  {
    g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);
    x1 += iconToUse->getWidth() + 5;
  }
  g.setFont(juce::Font(12));
  g.drawText(getUniqueName(), x1, 0, width - x1, height, juce::Justification::centredLeft, true);
}

void GenericTreeViewItem::createSubItems()
{
  std::vector< std::pair<string, ObjectPtr> > subObjects = owner->getSubObjects(object);
  static const size_t maxCount = 1000;
  size_t size = subObjects.size() < maxCount ? subObjects.size() : maxCount;
  for (size_t i = 0; i < size; ++i)
    addSubItem(owner->createItem(subObjects[i].second, subObjects[i].first));
  if (size < subObjects.size())
    addSubItem(new GenericTreeViewItem(owner, ObjectPtr(), string((int)(subObjects.size() - size)) + T(" other elements...")));
}

void GenericTreeViewItem::itemOpennessChanged(bool isNowOpen)
{
  if (isNowOpen && !hasBeenOpened)
  {
    hasBeenOpened = true;
    createSubItems();
  }
}

void GenericTreeViewItem::itemSelectionChanged(bool isNowSelected)
  {owner->invalidateSelection();}
  
void GenericTreeViewItem::setIcon(const string& name)
  {iconToUse = userInterfaceManager().getImage(name, 18, 18);}

ObjectPtr GenericTreeViewItem::getTargetObject(ExecutionContext& context) const
  {return object;}

/*
** GenericTreeView
*/
GenericTreeView::GenericTreeView(ObjectPtr object, const string& name, bool rootNodeVisible)
  : object(object), name(name), root(NULL), isSelectionUpToDate(false), isTreeUpToDate(true)
{
  setColour(backgroundColourId, Colours::white);
  setMultiSelectEnabled(true);
  setWantsKeyboardFocus(true);
  setRootItemVisible(rootNodeVisible);
  startTimer(100);
}

GenericTreeView::~GenericTreeView()
  {clearTree();}

int GenericTreeView::getDefaultWidth() const
  {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 4;}
  
void GenericTreeView::timerCallback()
{
  if (!isSelectionUpToDate)
  {
    std::vector<ObjectPtr> selectedObjects;
    selectedObjects.reserve(getNumSelectedItems());
    string selectionName;
    for (int i = 0; i < getNumSelectedItems(); ++i)
    {
      GenericTreeViewItem* item = dynamic_cast<GenericTreeViewItem* >(getSelectedItem(i));
      if (item && item != root)
      {
        ObjectPtr targetObject = item->getTargetObject(defaultExecutionContext());
        selectedObjects.push_back(targetObject);
        if (!selectionName.isEmpty())
          selectionName += T(", ");
        selectionName += item->getUniqueName();
      }
    }
    isSelectionUpToDate = true;
    sendSelectionChanged(selectedObjects, selectionName);
  }
  if (!isTreeUpToDate)
  {
    repaint();
    isTreeUpToDate = true;
  }
}
  
bool GenericTreeView::keyPressed(const juce::KeyPress& key)
{
  if (key.getKeyCode() == juce::KeyPress::F5Key)
  {
    clearTree(), buildTree();
    return true;
  }
  return juce::TreeView::keyPressed(key);
}

void GenericTreeView::buildTree()
{
  jassert(!root);
  root = createItem(object, name);
  setRootItem(root);
  root->setOpen(true);
  root->setSelected(true, true);
}

void GenericTreeView::clearTree()
{
  if (root)
  {
    deleteRootItem();
    root = NULL;
  }    
}

/*
** DoubleVector Tree
*
SparseDoubleVectorPtr sparseVector = object.dynamicCast<SparseDoubleVector>();
if (sparseVector && !options.showMissingVariables) // otherwise use the default Container implementation
{
  ClassPtr elementsType = sparseVector->getElementsType();
  for (size_t i = 0; i < sparseVector->getNumValues(); ++i)
  {
    const std::pair<size_t, double>& value = sparseVector->getValue(i);
    addSubObject(sparseVector->getElementName(value.first), Double::create(elementsType, value.second));
  }
  mightContainSubItemsFlag = true;
  return;
}
    
CompositeDoubleVectorPtr compositeVector = object.dynamicCast<CompositeDoubleVector>();
if (compositeVector)
{
  EnumerationPtr elementsEnumeration = compositeVector->getElementsEnumeration();
  for (size_t i = 0; i < compositeVector->getNumSubVectors(); ++i)
  {
    size_t offset = compositeVector->getSubVectorOffset(i);
    DoubleVectorPtr subVector = compositeVector->getSubVector(i);
    string name = T("[") + string((int)i) + T("]");

    string parentName = elementsEnumeration->getElementName(offset);
    string childName = subVector->getElementName(0);
    int j = parentName.indexOf(childName);
    if (j >= 0)
      name = parentName.substring(0, j - 1);
    addSubObject(name, subVector);
  }
  mightContainSubItemsFlag = true;
  return;
}
    
if (object.isInstanceOf<DoubleVector>())
{
  DoubleVectorPtr doubleVector = object.staticCast<DoubleVector>();
  // skip 0 values for double vectors
  for (size_t i = 0; i < doubleVector->getNumElements(); ++i)
  {
    ObjectPtr elt = doubleVector->getElement(i);
    jassert(elt.dynamicCast<Double>());
    if (Double::get(elt) != 0.0)
      addSubObject(string((int)i), container->getElement(i));
  }
  mightContainSubItemsFlag = true;
  return;
}
*/
