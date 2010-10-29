/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistribution.h      | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

# include "Variable.h"

namespace lbcpp
{

class ProbabilityDistribution : public Object
{
public:
  ProbabilityDistribution(ClassPtr thisClass) : Object(thisClass) {}
  ProbabilityDistribution() : Object() {}
  
  virtual double computeEntropy() const = 0;
  virtual double compute(const Variable& value) const = 0;
  virtual Variable sample(RandomGeneratorPtr random) const = 0;
};

typedef ReferenceCountedObjectPtr<ProbabilityDistribution> ProbabilityDistributionPtr;

extern ClassPtr probabilityDistributionClass;

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
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual double computeEntropy() const;

protected:
  friend class BernoulliDistributionClass;

  double pTrue, pFalse;
};

typedef ReferenceCountedObjectPtr<BernoulliDistribution> BernoulliDistributionPtr;

class DiscreteProbabilityDistribution : public ProbabilityDistribution
{
public:
  DiscreteProbabilityDistribution(EnumerationPtr enumeration);
  DiscreteProbabilityDistribution() {}

  // DiscreteProbabilityDistribution
  EnumerationPtr getEnumeration() const
    {return getClass()->getTemplateArgument(0).dynamicCast<Enumeration>();}

  void increment(const Variable& value);

  // ProbabilityDistribution
  virtual double compute(const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual double computeEntropy() const;

  // Object
  virtual String toString() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);
  virtual bool loadFromString(const String& str, MessageCallback& callback);

  virtual int compare(ObjectPtr otherObject) const
    {return compareVariables(otherObject);}

  void setProbability(size_t index, double probability);
  void setMissingProbability(double probability)
    {setProbability(values.size() - 1, probability);}

  double getProbability(size_t index) const
    {jassert(index < values.size()); return values[index];}

  lbcpp_UseDebuggingNewOperator

private:
  friend class DiscreteProbabilityDistributionClass;

  std::vector<double> values;
  CriticalSection cachedEntropyLock;
  Variable cachedEntropy;
};

typedef ReferenceCountedObjectPtr<DiscreteProbabilityDistribution> DiscreteProbabilityDistributionPtr;

extern ClassPtr discreteProbabilityDistributionClass(TypePtr type);
inline ClassPtr discreteProbabilityDistributionClass(EnumerationPtr enumeration)
  {return discreteProbabilityDistributionClass((TypePtr)enumeration);}

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
