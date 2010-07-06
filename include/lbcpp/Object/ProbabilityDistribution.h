/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistribution.h      | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/Object/Variable.h>

namespace lbcpp
{

class ProbabilityDistribution : public Object
{
public:
  virtual double compute(const Variable& value) const = 0;
  virtual Variable sample(RandomGenerator& random) const = 0;
};

typedef ReferenceCountedObjectPtr<ProbabilityDistribution> ProbabilityDistributionPtr;

class DiscreteProbabilityDistribution : public Object
{
public:
  DiscreteProbabilityDistribution(EnumerationPtr enumeration);

  virtual double compute(const Variable& value) const;

  virtual Variable sample(RandomGenerator& random) const
    {return random.sampleWithProbabilities(values, sum);}

  EnumerationPtr getEnumeration() const
    {return enumeration;}

  virtual size_t getNumVariables() const
    {return values.size();}

  virtual Variable getVariable(size_t index) const
    {jassert(index < values.size()); return sum ? values[index] / sum : 0.0;}

  virtual void setVariable(size_t index, const Variable& value);
  void increment(const Variable& value);

private:
  EnumerationPtr enumeration;
  std::vector<double> values;
  double sum;
};

typedef ReferenceCountedObjectPtr<DiscreteProbabilityDistribution> DiscreteProbabilityDistributionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
