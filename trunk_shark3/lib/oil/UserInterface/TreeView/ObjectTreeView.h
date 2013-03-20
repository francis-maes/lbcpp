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

class ObjectTreeView : public GenericTreeView
{
public:
  ObjectTreeView(const ObjectPtr& object, const string& name, bool makeRootNodeVisible)
    : GenericTreeView(object, name, makeRootNodeVisible)
    {buildTree();}
  
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
  
  virtual string getObjectTooltip(const string& name, const ObjectPtr& object)
  {
    return JUCE_T("Name: ") + name +
      JUCE_T("\nClass: ") + object->getClassName() +
      JUCE_T("\nValue: ") + object->toShortString();
  }

  virtual size_t getNumDataColumns()
    {return 2;}

  virtual std::vector<ObjectPtr> getObjectData(const ObjectPtr& object)
  {
    std::vector<ObjectPtr> res(2);
    res[0] = new String(object->toShortString());
    res[1] = new String(object->getClassName());
    return res;
  }

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_OBJECT_H_
