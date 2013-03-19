/*-----------------------------------------.---------------------------------.
| Filename: GrowExpressionSampler.h        | Grow Expression Sampler         |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 21:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_GROW_SAMPLER_H_
# define ML_EXPRESSION_GROW_SAMPLER_H_

# include <ml/ExpressionSampler.h>

namespace lbcpp
{

class GrowExpressionSampler : public DepthControlledExpressionSampler
{
public:
  GrowExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5)
    : DepthControlledExpressionSampler(minDepth, maxDepth) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    DepthControlledExpressionSampler::initialize(context, domain);
    functions = this->domain->getFunctions();
    terminalsAndFunctions = this->domain->getTerminalsAndFunctions();
  }

  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const
  {
    jassert(minDepth >= 1 && maxDepth >= 1 && minDepth <= maxDepth);

    ObjectPtr action;
    if (minDepth > 1) // must be an internal node
      action = functions[random->sampleSize(functions.size())];
    else if (maxDepth == 1) // must be a leaf node
      action = terminals[random->sampleSize(terminals.size())];
    else // any kind of node
      action = terminalsAndFunctions[random->sampleSize(terminalsAndFunctions.size())];

    FunctionPtr function = action.dynamicCast<Function>();
    if (function)
    {
      jassert(maxDepth > 1);
      size_t newMinDepth = (minDepth > 1 ? minDepth - 1 : 1);
      size_t newMaxDepth = (maxDepth - 1);
      std::vector<ExpressionPtr> expressions(function->getNumInputs());
      for (size_t i = 0; i < expressions.size(); ++i)
      {
        expressions[i] = sampleTree(random, newMinDepth, newMaxDepth);
        jassert(expressions[i]);
      }
      ExpressionPtr res = new FunctionExpression(function, expressions);
#ifdef JUCE_DEBUG
      size_t depth = res->getDepth();
      jassert(depth >= minDepth && depth <= maxDepth);
#endif // JUCE_DEBUG
      return res;
    }
    else
    {
      jassert(1 >= minDepth && 1 <= maxDepth);
      return action.staticCast<Expression>();
    }
  }

protected:
  std::vector<FunctionPtr> functions;
  std::vector<ObjectPtr> terminalsAndFunctions;
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_GROW_SAMPLER_H_
