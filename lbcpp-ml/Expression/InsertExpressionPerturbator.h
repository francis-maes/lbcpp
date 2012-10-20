/*-----------------------------------------.---------------------------------.
| Filename: InsertExpressionPerturbator.h  | Insert Expression Perturbator   |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 22:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_PERTURBATOR_INSERT_H_
# define LBCPP_ML_EXPRESSION_PERTURBATOR_INSERT_H_

# include <lbcpp-ml/ExpressionSampler.h>

namespace lbcpp
{

// This operator mutate a GP tree by inserting a new branch at a random position in a tree,
// using the original subtree at this position as one argument, and if necessary randomly
// selecting terminal primitives to complete the arguments of the inserted node.
class InsertExpressionPerturbator : public ExpressionPerturbator
{
public:
  InsertExpressionPerturbator(size_t maxDepth = 17)
    : maxDepth(maxDepth) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionPerturbator::initialize(context, domain);
    terminals = this->domain->getTerminals();
  }
  
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ExpressionPtr node = expression->sampleNode(random);
    FunctionPtr function = domain->getFunction(random->sampleSize(domain->getNumFunctions()));
    std::vector<ExpressionPtr> arguments(function->getNumInputs());
    arguments[random->sampleSize(arguments.size())] = node;
    for (size_t i = 0; i < arguments.size(); ++i)
      if (!arguments[i])
        arguments[i] = terminals[random->sampleSize(terminals.size())];
    ExpressionPtr res = expression->cloneAndSubstitute(node, new FunctionExpression(function, arguments));
    jassert(res->getTreeSize() > expression->getTreeSize());
    return res->getDepth() <= maxDepth ? res : ExpressionPtr();
  }

protected:
  friend class InsertExpressionPerturbatorClass;

  size_t maxDepth;

  std::vector<ExpressionPtr> terminals;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_PERTURBATOR_INSERT_H_
