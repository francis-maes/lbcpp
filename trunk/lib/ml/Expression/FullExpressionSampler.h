/*-----------------------------------------.---------------------------------.
| Filename: FullExpressionSampler.h        | Full Expression Sampler         |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 21:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_FULL_SAMPLER_H_
# define ML_EXPRESSION_FULL_SAMPLER_H_

# include <ml/ExpressionSampler.h>

namespace lbcpp
{

class FullExpressionSampler : public DepthControlledExpressionSampler
{
public:
  FullExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5)
    : DepthControlledExpressionSampler(minDepth, maxDepth) {}
  
  // returns a tree of depth maxDepth (argument minDepth is ignored)
  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const
  {
    jassert(maxDepth > 0);
    if (maxDepth == 1)
      return terminals[random->sampleSize(terminals.size())];
    else
    {
      FunctionPtr function = domain->getFunction(random->sampleSize(domain->getNumFunctions()));
      std::vector<ExpressionPtr> expressions(function->getNumInputs());
      for (size_t i = 0; i < expressions.size(); ++i)
        expressions[i] = sampleTree(random, minDepth, maxDepth - 1);
      return new FunctionExpression(function, expressions);
    }
  }
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_FULL_SAMPLER_H_
