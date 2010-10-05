/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistributionPerc...h| Probability Distribution        |
| Author  : Francis Maes                   |  Perception                     |
| Started : 05/10/2010 17:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_FUNCTION_PERCEPTION_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/Function/Perception.h>
# include <lbcpp/Data/ProbabilityDistribution.h>

namespace lbcpp
{

class DiscreteProbabilityDistributionPerception : public Perception
{
public:
  DiscreteProbabilityDistributionPerception(EnumerationPtr enumeration)
    : enumeration(enumeration) {}
  DiscreteProbabilityDistributionPerception() {}

  virtual TypePtr getInputType() const
    {return discreteProbabilityDistributionClass(enumeration);}

  virtual size_t getNumOutputVariables() const
    {return enumeration->getNumElements() + 2;}

  virtual TypePtr getOutputVariableType(size_t index) const
  {
    if (index <= enumeration->getNumElements())
      return probabilityType();
    else
      return negativeLogProbabilityType();
  }

  virtual String getOutputVariableName(size_t index) const
  {
    if (index < enumeration->getNumElements())
      return T("p[") + enumeration->getElementName(index) + T("]");
    else if (index == enumeration->getNumElements())
      return T("p[missing]");
    else
      return T("entropy");
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    DiscreteProbabilityDistributionPtr distribution = input.getObjectAndCast<DiscreteProbabilityDistribution>();
    jassert(distribution);
    size_t n = enumeration->getNumElements();
    for (size_t i = 0; i <= n; ++i)
    {
      double p = distribution->getProbability(i);
      if (p)
        callback->sense(i, probability(p));
    }
    callback->sense(n + 1, Variable(distribution->computeEntropy(), negativeLogProbabilityType()));
  }

  juce_UseDebuggingNewOperator

protected:
  friend class DiscreteProbabilityDistributionPerceptionClass;

  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_PROBABILITY_DISTRIBUTION_H_
