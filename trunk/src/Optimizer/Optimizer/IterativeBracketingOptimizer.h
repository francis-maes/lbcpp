/*-----------------------------------------.---------------------------------.
| Filename: IterativeBracketingOptimizer.h | Iterative Bracketing Optimizer  |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2010 23:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_ITERATIVE_BRACKETING_H_
# define LBCPP_OPTIMIZER_ITERATIVE_BRACKETING_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>
# include "../ObjectiveFunction/MarginalObjectiveFunction.h"

// TODO arnaud : modified to use new Function interface but not tested yet
// TODO arnaud : initial guess necessary ?

namespace lbcpp
{

// works for Objects
// uses a MultiVariateDistribution apriori and the initial guess
class IterativeBracketingOptimizer : public Optimizer
{
public:
  IterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerPtr& baseOptimizer)
    : numPasses(numPasses), reductionFactor(reductionFactor), baseOptimizer(baseOptimizer) {}
  IterativeBracketingOptimizer() {}
  
  virtual TypePtr getRequiredAprioriType() const
    {return independentMultiVariateDistributionClass(variableType);}

  virtual Variable optimize(ExecutionContext& context, const FunctionPtr& objective, const DistributionPtr& apriori, const Variable& guess) const
  {
    IndependentMultiVariateDistributionPtr distribution = apriori.dynamicCast<IndependentMultiVariateDistribution>();
    jassert(distribution);
    ObjectPtr currentGuess = guess.getObject()->clone(context);
    // TODO arnaud : jassert
    
    TypePtr parametersType = distribution->getElementsType();
    size_t numVariables = parametersType->getNumMemberVariables();
    for (size_t i = 0; i < numPasses; ++i)
    {
      for (size_t j = 0; j < numVariables; ++j)
      {
        //Object::displayObjectAllocationInfo(std::cerr);
        
        DistributionPtr marginalDistribution = distribution->getSubDistribution(j);
        if (!marginalDistribution)
          continue;
        
        /*OptimizerInputPtr subOptimizerInput(new OptimizerInput(
         marginalObjectiveFunction(objective, currentGuess, j),
         distribution->getSubDistribution(j),
         currentGuess->getVariable(j)));*/
        
        Variable bestValue = baseOptimizer->compute(context, marginalObjectiveFunction(objective, currentGuess, j), distribution->getSubDistribution(j), currentGuess->getVariable(j));
        if (bestValue.isNil())
          return Variable();
        
        currentGuess->setVariable(j, bestValue);
        marginalDistribution = updateMarginalDistribution(marginalDistribution, bestValue);
        distribution->setSubDistribution(j, marginalDistribution);
        
        context.informationCallback(T("Pass ") + String((int)i + 1) + T(" Param ") + currentGuess->getVariableName(j) + T(" Guess ") + currentGuess->toShortString());
        Variable(distribution).printRecursively(std::cout);
      }
    }
    return currentGuess;
  }

private:
  friend class IterativeBracketingOptimizerClass;

  size_t numPasses;
  double reductionFactor;
  OptimizerPtr baseOptimizer;

  DistributionPtr updateMarginalDistribution(const DistributionPtr& currentDistribution, const Variable& newGuess) const
  {
    UniformDistributionPtr uniform = currentDistribution.dynamicCast<UniformDistribution>();
    jassert(uniform); // this is the only one which is supported yet
    double minimum = uniform->getMinimum();
    double maximum = uniform->getMaximum();

    double halfRange = (maximum - minimum) / (2.0 * reductionFactor);
    double newMinimum = newGuess.getDouble() - halfRange;
    double newMaximum = newGuess.getDouble() + halfRange;
    jassert(newMinimum < newMaximum);
    return new UniformDistribution(newMinimum, newMaximum);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_ITERATIVE_BRACKETING_H_
