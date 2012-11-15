/*-----------------------------------------.---------------------------------.
| Filename: ObjectTreeView.cpp             | Object Tree component           |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ObjectTreeView.h"
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/Loader.h>
#include <lbcpp/Core/Double.h>
#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/library.h>
using namespace lbcpp;
using juce::Font;
using juce::Justification;
using juce::Graphics;
using juce::Component;
using juce::Colours;

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
  if (object.isInstanceOf<File>())
  {
    juce::File file = File::get(object);
    LoaderPtr loader = lbcpp::getTopLevelLibrary()->findLoaderForFile(defaultExecutionContext(), file);
    if (loader)
      return getIconForType(loader->getTargetClass());
  }
  return getIconForType(object->getClass());
}

class ObjectTreeViewItem : public SimpleTreeViewItem
{
public:
  ObjectTreeViewItem(const string& name, const ObjectPtr& object, const ObjectTreeOptions& options)
    : SimpleTreeViewItem(name, getIconForObject(object), true), 
      object(object), options(options), typeName(object->getClassName()), component(NULL), numUndisplayedChildElements(0)
  {
    mightContainSubItemsFlag = false;
    shortSummary = object->toShortString();
    ClassPtr type = object->getClass();
    if (options.showMissingVariables)
    {
      subObjects.reserve(subObjects.size() + type->getNumMemberVariables());
      for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
        addSubObject(type->getMemberVariableName(i), object->getVariable(i));
    }
    else
    {
      subObjects.reserve(subObjects.size() + type->getNumMemberVariables());
      for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
      {
        ObjectPtr value = object->getVariable(i);
        if (value)
          addSubObject(type->getMemberVariableName(i), value);
      }
    }
    mightContainSubItemsFlag = (subObjects.size() > 0);
  
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

    VectorPtr container = object.dynamicCast<Vector>();
    if (container)
    {
      static const size_t maxCount = 1000;
      size_t count = container->getNumElements();
      if (count > maxCount)
      {
        numUndisplayedChildElements = count - maxCount;
        count = maxCount;
      }
      subObjects.reserve(subObjects.size() + count);
      for (size_t i = 0; i < count; ++i)
        addSubObject(string((int)i), container->getElement(i));
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

    if (object.dynamicCast<File>())
    {
      juce::File file = File::get(object);
      if (file.isDirectory())
      {
        juce::OwnedArray<juce::File> files;
        file.findChildFiles(files, juce::File::findFilesAndDirectories, false);
        subObjects.reserve(subObjects.size() + files.size());
   
        for (int i = 0; i < files.size(); ++i)
          addSubObject(files[i]->getFileName(), File::create(*files[i]));
        mightContainSubItemsFlag = true;
      }
      else
        mightContainSubItemsFlag = false;
      return;
    }
  }

  virtual void itemSelectionChanged(bool isNowSelected)
  {
    ObjectTreeView* owner = dynamic_cast<ObjectTreeView* >(getOwnerView());
    jassert(owner);
    owner->invalidateSelection();
  }
  
  virtual void createSubItems()
  {
    for (size_t i = 0; i < subObjects.size(); ++i)
      addSubItem(new ObjectTreeViewItem(subObjects[i].first, subObjects[i].second, options));
    if (numUndisplayedChildElements > 0)
      addSubItem(new SimpleTreeViewItem(string((int)numUndisplayedChildElements) + T(" other elements...")));
  }

  const ObjectPtr& getObject() const
    {return object;}
     
  virtual const string getTooltip()
  {
    string res = T("Name: ") + getUniqueName() +
      T("\nType: ") + typeName +
      T("\nValue: ") + shortSummary;
    if (getNumSubItems())
      res += T("\nNum sub variables: ") + string((int)getNumSubItems());
    return res;
  }

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
    enum {wantedLength = 400};
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
  ObjectPtr object;
  const ObjectTreeOptions& options;
  string typeName;
  string shortSummary;
  Component* component;
  size_t numUndisplayedChildElements;

  std::vector< std::pair<string, ObjectPtr> > subObjects;

  void addSubObject(const string& name, const ObjectPtr& variable)
    {subObjects.push_back(std::make_pair(name, variable));}
};

/*
** ObjectTreeView
*/
ObjectTreeView::ObjectTreeView(const ObjectPtr& object, const string& name, const ObjectTreeOptions& options)
  : GenericTreeView(object, name), options(options), root(NULL), isSelectionUpToDate(false)
{
  setRootItemVisible(options.makeRootNodeVisible);
  setWantsKeyboardFocus(true);
  setMultiSelectEnabled(true);
  buildTree();
  root->setSelected(true, true);
  startTimer(100);  
}

ObjectTreeView::~ObjectTreeView()
  {clearTree();}

bool ObjectTreeView::keyPressed(const juce::KeyPress& key)
{
  if (key.getKeyCode() == juce::KeyPress::F5Key)
  {
    clearTree(), buildTree();
    return true;
  }
  return juce::TreeView::keyPressed(key);
}

void ObjectTreeView::clearTree()
{
  if (root)
  {
    deleteRootItem();
    root = NULL;
  }    
}

void ObjectTreeView::buildTree()
{
  root = new ObjectTreeViewItem(name, object, options);
  setRootItem(root);
  root->setOpen(true);
}

void ObjectTreeView::timerCallback()
{
  if (!isSelectionUpToDate)
  {
    std::vector<ObjectPtr> selectedObjects;
    selectedObjects.reserve(getNumSelectedItems());
    string selectionName;
    for (int i = 0; i < getNumSelectedItems(); ++i)
    {
      ObjectTreeViewItem* item = dynamic_cast<ObjectTreeViewItem* >(getSelectedItem(i));
      if (item && item->getObject() && item != root)
      {
        selectedObjects.push_back(item->getObject());
        if (!selectionName.isEmpty())
          selectionName += T(", ");
        selectionName += item->getUniqueName();
      }
    }
    isSelectionUpToDate = true;
    sendSelectionChanged(selectedObjects, selectionName);
  }
}

void ObjectTreeView::invalidateSelection()
  {isSelectionUpToDate = false;}

