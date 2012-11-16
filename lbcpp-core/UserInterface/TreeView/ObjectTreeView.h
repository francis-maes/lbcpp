/*-----------------------------------------.---------------------------------.
| Filename: ObjectTreeView.h               | Object Tree component           |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_TREE_VIEW_OBJECT_H_
# define LBCPP_USER_INTERFACE_TREE_VIEW_OBJECT_H_

# include "GenericTreeView.h"

namespace lbcpp
{
  
class ObjectTreeViewItem : public GenericTreeViewItem
{
public:
  ObjectTreeViewItem(GenericTreeView* owner, const string& name, const ObjectPtr& object)
    : GenericTreeViewItem(owner, object, name), typeName(object->getClassName()), shortString(object->toShortString()) {}

  virtual const string getTooltip()
  {
    string res = T("Name: ") + getUniqueName() +
      T("\nType: ") + typeName +
      T("\nValue: ") + shortString;
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

    int typeAndNameLength;
    enum {wantedLength = 400, numFields = 3};
    int remainingWidth = width - x1;
    if (remainingWidth >= numFields * wantedLength)
      typeAndNameLength = wantedLength;
    else
      typeAndNameLength = remainingWidth / numFields;

    g.setFont(juce::Font(12, juce::Font::bold));
    g.drawText(getUniqueName(), x1, 0, typeAndNameLength - 5, height, juce::Justification::centredLeft, true);
    x1 += typeAndNameLength;
    g.setFont(juce::Font(12, juce::Font::italic));
    g.drawText(typeName, x1, 0, typeAndNameLength - 5, height, juce::Justification::centredLeft, true);
    x1 += typeAndNameLength;

    g.setFont(juce::Font(12));
    g.drawText(shortString, x1, 0, width - x1 - 2, height, juce::Justification::centredLeft, true);
  }

  juce_UseDebuggingNewOperator

protected:
  string typeName;
  string shortString;
};

class ObjectTreeView : public GenericTreeView
{
public:
  ObjectTreeView(const ObjectPtr& object, const string& name, bool makeRootNodeVisible)
    : GenericTreeView(object, name, makeRootNodeVisible)
    {buildTree();}
  
  virtual GenericTreeViewItem* createItem(const ObjectPtr& object, const string& name)
    {return new ObjectTreeViewItem(this, name, object);}

  virtual bool mightHaveSubObjects(const ObjectPtr& object)
    {return object->getNumVariables() > 0 || object.isInstanceOf<Vector>();}

  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object)
  {
    ClassPtr type = object->getClass();
    std::vector< std::pair<string, ObjectPtr> > res;
    res.reserve(type->getNumMemberVariables());
    for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
    {
      ObjectPtr value = object->getVariable(i);
      if (value)
        res.push_back(std::make_pair(type->getMemberVariableName(i), value));
    }
  
    VectorPtr vector = object.dynamicCast<Vector>();
    if (vector)
    {
      size_t count = vector->getNumElements();
      res.reserve(res.size() + count);
      for (size_t i = 0; i < count; ++i)
        res.push_back(std::make_pair(string((int)i), vector->getElement(i)));
    }
    return res;
  }

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_OBJECT_H_
