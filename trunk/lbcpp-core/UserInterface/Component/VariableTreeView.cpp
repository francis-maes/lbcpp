/*-----------------------------------------.---------------------------------.
| Filename: VariableTreeView.cpp           | Variable Tree component         |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "VariableTreeView.h"
#include <lbcpp/Core/Container.h>
#include <lbcpp/Core/Loader.h>
#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/library.h>
using namespace lbcpp;
using juce::Font;
using juce::Justification;
using juce::Graphics;
using juce::Component;
using juce::Colours;

static String getIconForType(const TypePtr& type)
{
  String res = type->getName() + "-32.png";
  if (UserInterfaceManager().hasImage(res))
    return res;
  
  if (type->inheritsFrom(vectorClass()))
    return "Vector-32.png";

  return "Object-32.png";
}

static String getIconForVariable(const Variable& variable)
{
  if (variable.isFile())
  {
    File file = variable.getFile();
    LoaderPtr loader = lbcpp::getTopLevelLibrary()->findLoaderForFile(defaultExecutionContext(), file);
    if (loader)
      return getIconForType(loader->getTargetClass());
  }
  return getIconForType(variable.getType());
}

class VariableTreeViewItem : public SimpleTreeViewItem
{
public:
  VariableTreeViewItem(const String& name, const Variable& variable, const VariableTreeOptions& options)
    : SimpleTreeViewItem(name, getIconForVariable(variable), true), 
      variable(variable), options(options), typeName(variable.getTypeName()), component(NULL), numUndisplayedChildElements(0)
  {
    mightContainSubItemsFlag = false;
    shortSummary = variable.toShortString();
    TypePtr type = variable.getType();
    if (variable.exists() && variable.isObject())
    {
      ObjectPtr object = variable.getObject();
      type = object->getClass(); // the exact type may be different from the base type
      if (options.showMissingVariables)
      {
        subVariables.reserve(subVariables.size() + type->getNumMemberVariables());
        for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
          addSubVariable(type->getMemberVariableName(i), object->getVariable(i));
      }
      else
      {
        Object::VariableIterator* iterator = object->createVariablesIterator();
        if (iterator)
        {
          for (; iterator->exists(); iterator->next())
          {
            size_t variableIndex;
            Variable subVariable = iterator->getCurrentVariable(variableIndex);
            if (subVariable.exists())
              addSubVariable(type->getMemberVariableName(variableIndex), subVariable);
          }
          delete iterator;
        }
        else
        {
          subVariables.reserve(subVariables.size() + type->getNumMemberVariables());
          for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
          {
            Variable value = object->getVariable(i);
            if (value.exists())
              addSubVariable(type->getMemberVariableName(i), value);
          }
        }
      }
      mightContainSubItemsFlag = (subVariables.size() > 0);
    }

    SparseDoubleVectorPtr sparseVector = variable.dynamicCast<SparseDoubleVector>();
    if (sparseVector && !options.showMissingVariables) // otherwise use the default Container implementation
    {
      TypePtr elementsType = sparseVector->getElementsType();
      for (size_t i = 0; i < sparseVector->getNumValues(); ++i)
      {
        const std::pair<size_t, double>& value = sparseVector->getValue(i);
        addSubVariable(sparseVector->getElementName(value.first), Variable(value.second, elementsType));
      }
      mightContainSubItemsFlag = true;
      return;
    }
    
    CompositeDoubleVectorPtr compositeVector = variable.dynamicCast<CompositeDoubleVector>();
    if (compositeVector)
    {
      EnumerationPtr elementsEnumeration = compositeVector->getElementsEnumeration();
      for (size_t i = 0; i < compositeVector->getNumSubVectors(); ++i)
      {
        size_t offset = compositeVector->getSubVectorOffset(i);
        DoubleVectorPtr subVector = compositeVector->getSubVector(i);
        String name = T("[") + String((int)i) + T("]");

        String parentName = elementsEnumeration->getElementName(offset);
        String childName = subVector->getElementName(0);
        int j = parentName.indexOf(childName);
        if (j >= 0)
          name = parentName.substring(0, j - 1);
        addSubVariable(name, subVector);
      }
      mightContainSubItemsFlag = true;
      return;
    }

    ContainerPtr container = variable.dynamicCast<Container>();
    if (container)
    {
      static const size_t maxCount = 1000;
      size_t count = container->getNumElements();
      if (count > maxCount)
      {
        numUndisplayedChildElements = count - maxCount;
        count = maxCount;
      }
      subVariables.reserve(subVariables.size() + count);
      bool isDoubleVector = container.dynamicCast<DoubleVector>();
    
      if (isDoubleVector && !options.showMissingVariables)
      {
        // skip 0 values for double vectors
        for (size_t i = 0; i < count; ++i)
        {
          ObjectPtr elt = container->getElement(i).getObject();
          jassert(elt.dynamicCast<NewDouble>());
          if (NewDouble::get(elt) != 0.0)
            addSubVariable(container->getElementName(i), container->getElement(i));
        }
      }
      else
        for (size_t i = 0; i < count; ++i)
          addSubVariable(container->getElementName(i), container->getElement(i));

      mightContainSubItemsFlag = true;
      return;
    }

    if (variable.isFile())
    {
      File file = variable.getFile();
      if (file.isDirectory())
      {
        juce::OwnedArray<File> files;
        file.findChildFiles(files, File::findFilesAndDirectories, false);
        subVariables.reserve(subVariables.size() + files.size());
   
        for (int i = 0; i < files.size(); ++i)
          addSubVariable(files[i]->getFileName(), Variable(files[i]->getFullPathName(), fileType));
        mightContainSubItemsFlag = true;
      }
      else
        mightContainSubItemsFlag = false;
      return;
    }
  }

  virtual void itemSelectionChanged(bool isNowSelected)
  {
    VariableTreeView* owner = dynamic_cast<VariableTreeView* >(getOwnerView());
    jassert(owner);
    owner->invalidateSelection();
  }
  
  virtual void createSubItems()
  {
    for (size_t i = 0; i < subVariables.size(); ++i)
      addSubItem(new VariableTreeViewItem(subVariables[i].first, subVariables[i].second, options));
    if (numUndisplayedChildElements > 0)
      addSubItem(new SimpleTreeViewItem(String((int)numUndisplayedChildElements) + T(" other elements...")));
  }

  Variable getVariable() const
    {return variable;}
     
  virtual const String getTooltip()
  {
    String res = T("Name: ") + getUniqueName() +
      T("\nType: ") + typeName +
      T("\nValue: ") + shortSummary;
    if (getNumSubItems())
      res += T("\nNum sub variables: ") + String((int)getNumSubItems());
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
  Variable variable;
  const VariableTreeOptions& options;
  String typeName;
  String shortSummary;
  Component* component;
  size_t numUndisplayedChildElements;

  std::vector< std::pair<String, Variable> > subVariables;

  void addSubVariable(const String& name, const Variable& variable)
    {subVariables.push_back(std::make_pair(name, variable));}
};

/*
** VariableTreeView
*/
VariableTreeView::VariableTreeView(const Variable& variable, const String& name, const VariableTreeOptions& options)
  : variable(variable), name(name), options(options), root(NULL), isSelectionUpToDate(false)
{
  setRootItemVisible(options.makeRootNodeVisible);
  setWantsKeyboardFocus(true);
  setMultiSelectEnabled(true);
  buildTree();
  root->setSelected(true, true);
  startTimer(100);  
}

VariableTreeView::~VariableTreeView()
  {clearTree();}

bool VariableTreeView::keyPressed(const juce::KeyPress& key)
{
  if (key.getKeyCode() == juce::KeyPress::F5Key)
  {
    clearTree(), buildTree();
    return true;
  }
  return juce::TreeView::keyPressed(key);
}

void VariableTreeView::clearTree()
{
  if (root)
  {
    deleteRootItem();
    root = NULL;
  }    
}

void VariableTreeView::buildTree()
{
  root = new VariableTreeViewItem(name, variable, options);
  setRootItem(root);
  root->setOpen(true);
}

void VariableTreeView::paint(Graphics& g)
{
  g.fillAll(Colours::white);
  juce::TreeView::paint(g);
}

void VariableTreeView::timerCallback()
{
  if (!isSelectionUpToDate)
  {
    std::vector<Variable> selectedVariables;
    selectedVariables.reserve(getNumSelectedItems());
    String selectionName;
    for (int i = 0; i < getNumSelectedItems(); ++i)
    {
      VariableTreeViewItem* item = dynamic_cast<VariableTreeViewItem* >(getSelectedItem(i));
      if (item && item->getVariable().exists() && item != root)
      {
        selectedVariables.push_back(item->getVariable());
        if (!selectionName.isEmpty())
          selectionName += T(", ");
        selectionName += item->getUniqueName();
      }
    }
    isSelectionUpToDate = true;
    sendSelectionChanged(selectedVariables, selectionName);
  }
}

void VariableTreeView::invalidateSelection()
  {isSelectionUpToDate = false;}

int VariableTreeView::getDefaultWidth() const
{
  return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 5;
  /*
  int numFields = 1;
  if (options.showTypes) ++numFields;
  if (options.showShortSummaries) ++numFields;
  return numFields * 200
  */
}

