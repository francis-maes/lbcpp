/*-----------------------------------------.---------------------------------.
| Filename: SwapExpressionPerturbator.h    | Swap Expression Perturbator     |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 22:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_PERTURBATOR_SWAP_H_
# define LBCPP_ML_EXPRESSION_PERTURBATOR_SWAP_H_

# include <ml/ExpressionSampler.h>

namespace lbcpp
{

// swaps a randomly chosen node symbol by another symbol of same arity
class SwapExpressionPerturbator : public ExpressionPerturbator
{
public:
  SwapExpressionPerturbator(double functionSelectionProbability = 0.0)
    : functionSelectionProbability(functionSelectionProbability) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionPerturbator::initialize(context, domain);
    terminals = this->domain->getTerminals();
    size_t maxArity = this->domain->getMaxFunctionArity();
    functionsByArity.resize(maxArity + 1);
    for (size_t i = 0; i < this->domain->getNumFunctions(); ++i)
    {
      FunctionPtr function = this->domain->getFunction(i);
      functionsByArity[function->getNumInputs()].push_back(function);
    }
  }

  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random  = context.getRandomGenerator();
    ExpressionPtr node = expression->sampleNode(random, functionSelectionProbability);
    FunctionExpressionPtr functionNode = node.dynamicCast<FunctionExpression>();
    ExpressionPtr newNode;
    if (functionNode)
    {
      const std::vector<FunctionPtr>& functions = functionsByArity[functionNode->getNumSubNodes()];
      FunctionPtr function = functions[random->sampleSize(functions.size())];
      newNode = new FunctionExpression(function, functionNode->getArguments());
    }
    else
      newNode = terminals[random->sampleSize(terminals.size())];
    ExpressionPtr res = expression->cloneAndSubstitute(node, newNode);
    jassert(res->getTreeSize() == expression->getTreeSize());
    return res;
  }

protected:
  friend class SwapExpressionPerturbatorClass;

  double functionSelectionProbability;

  std::vector<ExpressionPtr> terminals;
  std::vector< std::vector<FunctionPtr> > functionsByArity;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_PERTURBATOR_SWAP_H_
