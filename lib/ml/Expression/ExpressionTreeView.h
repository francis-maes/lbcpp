/*-----------------------------------------.---------------------------------.
| Filename: ExpressionTreeView.h           | Expression Tree View            |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2012 12:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_TREE_VIEW_H_
# define ML_EXPRESSION_TREE_VIEW_H_

# include "../../oil/UserInterface/TreeView/GenericTreeView.h"
# include <ml/Expression.h>

namespace lbcpp
{

class ExpressionTreeView : public GenericTreeView
{
public:
  ExpressionTreeView(ExpressionPtr expression, const string& name) : GenericTreeView(expression, name, true)
    {buildTree();}

  virtual bool mightHaveSubObjects(const ObjectPtr& object)
    {return object.dynamicCast<Expression>() ? object.staticCast<Expression>()->getNumSubNodes() > 0 || object->getNumVariables() > 0 : object->getNumVariables() > 0;}
    
  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object)
  {
    if (object.dynamicCast<Expression>() && object.staticCast<Expression>()->getNumSubNodes() > 0)
    {
      ExpressionPtr expression = object.staticCast<Expression>();
      std::vector< std::pair<string, ObjectPtr> > res(expression->getNumSubNodes());
      for (size_t i = 0; i < res.size(); ++i)
      {
        ExpressionPtr sub = expression->getSubNode(i);
        string str;
        if (sub.isInstanceOf<TestExpression>())
          str = "test";
        else if (sub.isInstanceOf<FunctionExpression>())
          str = sub.staticCast<FunctionExpression>()->getFunction()->toShortString();
        else
          str = sub->toShortString();
        res[i] = std::make_pair(str, sub);
      }
      return res;
    }
    else
    {
      std::vector< std::pair<string, ObjectPtr> > res(object->getNumVariables());
      for (size_t i = 0; i < res.size(); ++i)
        res[i] = std::make_pair(object->getVariableName(i), object->getVariable(i));
      if (object.dynamicCast<Expression>())
        res.erase(res.begin()); // if it is an expression remove the 'type' member field, we don't need to show it
      return res;
    }
  }
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_TREE_VIEW_H_
