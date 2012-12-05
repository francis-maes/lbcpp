/*-----------------------------------------.---------------------------------.
| Filename: SubTreeCrossOverExpressi..or.h | Sub-tree Cross over             |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 22:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_PERTURBATOR_SUBTREE_CROSS_OVER_H_
# define ML_EXPRESSION_PERTURBATOR_SUBTREE_CROSS_OVER_H_

# include <ml/ExpressionSampler.h>

namespace lbcpp
{

class SubTreeCrossOverExpressionPerturbator : public BinaryPerturbator
{
public:
  SubTreeCrossOverExpressionPerturbator(double functionSelectionProbability = 0.0, size_t maxDepth = 17)
    : functionSelectionProbability(functionSelectionProbability), maxDepth(maxDepth) {}

  virtual std::pair<ObjectPtr, ObjectPtr> samplePair(ExecutionContext& context, const ObjectPtr& object1, const ObjectPtr& object2) const
  {
    for (size_t attempt = 0; attempt < 2; ++attempt)
    {
      const ExpressionPtr& expression1 = object1.staticCast<Expression>();
      const ExpressionPtr& expression2 = object2.staticCast<Expression>();
      ExpressionPtr node1 = expression1->sampleNode(context.getRandomGenerator(), functionSelectionProbability);
      ExpressionPtr node2 = expression2->sampleNode(context.getRandomGenerator(), functionSelectionProbability);
      ExpressionPtr newExpression1 = expression1->cloneAndSubstitute(node1, node2);
      ExpressionPtr newExpression2 = expression2->cloneAndSubstitute(node2, node1);
      if (newExpression1->getDepth() <= maxDepth && newExpression2->getDepth() <= maxDepth)
        return std::make_pair(newExpression1, newExpression2);
    }
    return std::make_pair(object1, object2);
  }

protected:
  friend class SubTreeCrossOverExpressionPerturbatorClass;

  double functionSelectionProbability;
  size_t maxDepth;
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_PERTURBATOR_SUBTREE_CROSS_OVER_H_
