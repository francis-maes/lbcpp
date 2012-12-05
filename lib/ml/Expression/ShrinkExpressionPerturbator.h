/*-----------------------------------------.---------------------------------.
| Filename: ShrinkExpressionPerturbator.h  | Shrink Expression Perturbator   |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 22:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_PERTURBATOR_SHRINK_H_
# define ML_EXPRESSION_PERTURBATOR_SHRINK_H_

# include <ml/ExpressionSampler.h>

namespace lbcpp
{

// replaces a randomly chosen internal node by one of its child nodes (also randomly chosen)
class ShrinkExpressionPerturbator : public ExpressionPerturbator
{
public:
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random  = context.getRandomGenerator();
    ExpressionPtr selected = expression->sampleNode(random, 1.0);
    ExpressionPtr res = expression->cloneAndSubstitute(selected, selected->sampleSubNode(random));
    jassert(res->getTreeSize() < expression->getTreeSize());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_PERTURBATOR_SHRINK_H_
