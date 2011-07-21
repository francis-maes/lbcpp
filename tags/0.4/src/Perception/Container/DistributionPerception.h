/*-----------------------------------------.---------------------------------.
| Filename: DistributionPerc...h| Probability Distribution        |
| Author  : Francis Maes                   |  Perception                     |
| Started : 05/10/2010 17:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_FUNCTION_PERCEPTION_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

class DiscreteDistributionPerception : public Perception
{
public:
  DiscreteDistributionPerception(EnumerationPtr enumeration)
    : enumeration(enumeration)
    {computeOutputType();}

  DiscreteDistributionPerception() {}

  virtual TypePtr getInputType() const
    {return enumerationDistributionClass(enumeration);}

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const EnumerationDistributionPtr& distribution = input.getObjectAndCast<EnumerationDistribution>(context);
    jassert(distribution);
    size_t n = enumeration->getNumElements();
    for (size_t i = 0; i <= n; ++i)
    {
      double p = distribution->getProbability(i);
      if (p)
        callback->sense(i, Variable(p, probabilityType));
    }
    callback->sense(n + 1, distribution->computeEntropy());
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DiscreteDistributionPerceptionClass;

  EnumerationPtr enumeration;

  virtual void computeOutputType()
  {
    reserveOutputVariables(enumeration->getNumElements() + 2);
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      addOutputVariable(T("p[") + enumeration->getElementName(i) + T("]"), probabilityType);
    addOutputVariable(T("p[missing]"), probabilityType);
    addOutputVariable(T("entropy"), negativeLogProbabilityType);
    Perception::computeOutputType();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_PROBABILITY_DISTRIBUTION_H_