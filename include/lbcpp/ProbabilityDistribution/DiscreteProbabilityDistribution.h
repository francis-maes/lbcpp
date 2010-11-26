/*-----------------------------------------.---------------------------------.
| Filename: DiscreteProbabilityDistrib...h | Discrete Probability            |
| Author  : Julien Becker                  | Distributions                   |
| Started : 26/11/2010 11:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISCRETE_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_DISCRETE_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/ProbabilityDistribution/ProbabilityDistribution.h>
# include <lbcpp/Data/Variable.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class DiscreteProbabilityDistribution : public ProbabilityDistribution
{
public:
  DiscreteProbabilityDistribution(ClassPtr thisClass) : ProbabilityDistribution(thisClass) {}
  DiscreteProbabilityDistribution() : ProbabilityDistribution() {}

};

class BernoulliDistribution : public DiscreteProbabilityDistribution
{
public:
  BernoulliDistribution(double pTrue) : pTrue(pTrue), pFalse(1.0 - pTrue) {}
  BernoulliDistribution() : pTrue(0.0), pFalse(0.0) {}
  
  double getProbabilityOfTrue() const
    {return pTrue ? pTrue / (pTrue + pFalse) : 0.0;}
  
  double getProbabilityOfFalse() const
    {return 1.0 - getProbabilityOfTrue();}
  
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual double computeEntropy() const;
  
protected:
  friend class BernoulliDistributionClass;
  
  double pTrue, pFalse;
};

typedef ReferenceCountedObjectPtr<BernoulliDistribution> BernoulliDistributionPtr;

class EnumerationProbabilityDistribution : public DiscreteProbabilityDistribution
{
public:
  EnumerationProbabilityDistribution(EnumerationPtr enumeration);
  EnumerationProbabilityDistribution() {}
  
  // DiscreteProbabilityDistribution
  EnumerationPtr getEnumeration() const
    {return getClass()->getTemplateArgument(0).dynamicCast<Enumeration>();}
  
  void increment(const Variable& value);
  
  void normalize();
  
  // ProbabilityDistribution
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual double computeEntropy() const;
  
  // Object
  virtual String toString() const;
  
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);
  virtual bool loadFromString(ExecutionContext& context, const String& str);
  
  virtual int compare(ObjectPtr otherObject) const
    {return compareVariables(otherObject);}
  
  void setProbability(size_t index, double probability);
  void setMissingProbability(double probability)
  {setProbability(values.size() - 1, probability);}
  
  double getProbability(size_t index) const
    {jassert(index < values.size()); return values[index];}
  
  lbcpp_UseDebuggingNewOperator
  
private:
  friend class EnumerationProbabilityDistributionClass;
  
  std::vector<double> values;
  CriticalSection cachedEntropyLock;
  Variable cachedEntropy;
  size_t count;
};

typedef ReferenceCountedObjectPtr<EnumerationProbabilityDistribution> EnumerationProbabilityDistributionPtr;

extern ClassPtr enumerationProbabilityDistributionClass(TypePtr type);
inline ClassPtr enumerationProbabilityDistributionClass(EnumerationPtr enumeration)
  {return enumerationProbabilityDistributionClass((TypePtr)enumeration);}

}; /* namespace lbcpp */

#endif // !LBCPP_DISCRETE_PROBABILITY_DISTRIBUTION_H_
