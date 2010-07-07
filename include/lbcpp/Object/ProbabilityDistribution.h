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
  virtual double computeEntropy() const = 0;
  virtual double compute(const Variable& value) const = 0;
  virtual Variable sample(RandomGenerator& random) const = 0;
};

typedef ReferenceCountedObjectPtr<ProbabilityDistribution> ProbabilityDistributionPtr;

class BernoulliDistribution : public ProbabilityDistribution
{
public:
  BernoulliDistribution(double pTrue) : pTrue(pTrue), pFalse(1.0 - pTrue) {}
  BernoulliDistribution() : pTrue(0.0), pFalse(0.0) {}

  double getProbabilityOfTrue() const
    {return pTrue ? pTrue / (pTrue + pFalse) : 0.0;}

  double getProbabilityOfFalse() const
    {return 1.0 - getProbabilityOfTrue();}

  virtual double compute(const Variable& value) const;
  virtual Variable sample(RandomGenerator& random) const;
  virtual double computeEntropy() const;

protected:
  double pTrue, pFalse;
};

typedef ReferenceCountedObjectPtr<BernoulliDistribution> BernoulliDistributionPtr;

class DiscreteProbabilityDistribution : public ProbabilityDistribution
{
public:
  DiscreteProbabilityDistribution(EnumerationPtr enumeration);
  DiscreteProbabilityDistribution() : sum(0.0) {}

  virtual TypePtr getClass() const;
  virtual String toString() const;

  virtual double compute(const Variable& value) const;
  virtual Variable sample(RandomGenerator& random) const;
  virtual double computeEntropy() const;

  EnumerationPtr getEnumeration() const
    {return enumeration;}

  virtual size_t getNumVariables() const
    {return values.size();}

  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);

  void increment(const Variable& value);

  virtual ObjectPtr multiplyByScalar(double scalar);
  virtual ObjectPtr addWeighted(const Variable& value, double weight);

private:
  EnumerationPtr enumeration;
  std::vector<double> values;
  double sum;
};

typedef ReferenceCountedObjectPtr<DiscreteProbabilityDistribution> DiscreteProbabilityDistributionPtr;

extern ClassPtr discreteProbabilityDistributionClass(EnumerationPtr enumeration);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
