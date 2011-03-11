/*-----------------------------------------.---------------------------------.
| Filename: DiscreteProbabilityDistrib...h | Discrete Probability            |
| Author  : Julien Becker                  | Distributions                   |
| Started : 26/11/2010 11:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISCRETE_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_DISCRETE_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/Core/Variable.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Distribution/Distribution.h>
# include <algorithm>

namespace lbcpp
{

class DiscreteDistribution : public Distribution
{
public:
  DiscreteDistribution(ClassPtr thisClass) : Distribution(thisClass) {}
  DiscreteDistribution() : Distribution() {}

  juce_UseDebuggingNewOperator
};

class BernoulliDistribution : public DiscreteDistribution
{
public:
  BernoulliDistribution(double pTrue) : pTrue(pTrue), pFalse(1.0 - pTrue) {}
  BernoulliDistribution() : pTrue(0.0), pFalse(0.0) {}
  
  virtual TypePtr getElementsType() const
    {return booleanType;}

  double getProbabilityOfTrue() const
    {return pTrue ? pTrue / (pTrue + pFalse) : 0.0;}
  
  double getProbabilityOfFalse() const
    {return 1.0 - getProbabilityOfTrue();}
  
  virtual double computeProbability(const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual Variable sampleBest(RandomGeneratorPtr random) const;
  virtual double computeEntropy() const;
  
  //virtual DistributionBuilderPtr getBuilder() const
  //  {return new BernoulliDistributionBuilder();}
  
protected:
  friend class BernoulliDistributionClass;
  
  double pTrue, pFalse;
};

typedef ReferenceCountedObjectPtr<BernoulliDistribution> BernoulliDistributionPtr;

class EnumerationDistribution : public DiscreteDistribution
{
public:
  EnumerationDistribution(EnumerationPtr enumeration);
  EnumerationDistribution(EnumerationPtr enumeration, const std::vector<double>& probabilities);
  EnumerationDistribution() {}
  
  // DiscreteDistribution
  virtual TypePtr getElementsType() const
    {return getEnumeration();}

  EnumerationPtr getEnumeration() const
    {return getClass()->getTemplateArgument(0).dynamicCast<Enumeration>();}

  // Distribution
  virtual double computeProbability(const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual Variable sampleBest(RandomGeneratorPtr random) const;
  virtual double computeEntropy() const;
  
  //virtual DistributionBuilderPtr getBuilder() const
  //  {return new EnumerationDistributionBuilder(getEnumeration());}
  
  // Object
  virtual String toString() const;
  
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);
  virtual bool loadFromString(ExecutionContext& context, const String& str);
  
  virtual int compare(ObjectPtr otherObject) const
    {return compareVariables(otherObject);}

  double getProbability(size_t index) const
    {jassert(index < values.size()); return values[index];}
  
  const std::vector<double>& getProbabilities() const
    {return values;}

  void setProbability(size_t index, double value)
    {jassert(index < values.size()); values[index] = value;}

  lbcpp_UseDebuggingNewOperator
  
private:
  friend class EnumerationDistributionClass;
  friend class EnumerationDistributionBuilder;
  
  std::vector<double> values;
  CriticalSection cachedEntropyLock;
  Variable cachedEntropy;
};

typedef ReferenceCountedObjectPtr<EnumerationDistribution> EnumerationDistributionPtr;
  
  
class IntegerGaussianDistribution : public DiscreteDistribution
{
public:
  // TODO arnaud complete implementation
  IntegerGaussianDistribution(double mean = 0.0, double variance = 0.0) : mean(mean), variance(variance) {}
  
  virtual TypePtr getElementsType() const
    {return integerType;}
  
  virtual double computeEntropy() const
    {jassert(false); return 0;} // not implemented !
  
  virtual double computeProbability(const Variable& value) const
    {jassert(false); return 0;} // not implemented !
  
  virtual Variable sample(RandomGeneratorPtr random) const
    {return Variable((int)roundDouble(random->sampleDoubleFromGaussian(getMean(), getVariance())), integerType);} // FIXME: variance or stddev ? // TODO arnaud

  virtual Variable sampleBest(RandomGeneratorPtr random) const
    {jassert(false); return Variable();} // not implemented !

  double getMean() const
    {return mean;}
  
  double getVariance() const
    {return variance;}
  
  //virtual DistributionBuilderPtr getBuilder() const
  //  {return new IntegerGaussianDistributionBuilder();}
  
  juce_UseDebuggingNewOperator
  
protected:  
  friend class IntegerGaussianDistributionClass;

  double mean;
  double variance;
  
  static inline int roundPositiveDouble(double value)
    {return (int)(value + 0.5);}

  static inline int roundDouble(double value)
  {
    if (value >= 0)
      return roundPositiveDouble(value);
    else
      return -roundPositiveDouble(-value);
  }
};
  
typedef ReferenceCountedObjectPtr<IntegerGaussianDistribution> IntegerGaussianDistributionPtr;
  
  
class PositiveIntegerGaussianDistribution : public IntegerGaussianDistribution
{
public:
  // TODO arnaud complete implementation
  PositiveIntegerGaussianDistribution(double mean = 0.0, double variance = 0.0) : IntegerGaussianDistribution(mean, variance) {}
  
  virtual TypePtr getElementsType() const
    {return positiveIntegerType;}
  
  virtual Variable sample(RandomGeneratorPtr random) const
    {return Variable(juce::jmax(0, roundDouble(random->sampleDoubleFromGaussian(getMean(), getVariance()))), positiveIntegerType);} // FIXME: variance or stddev ? // TODO arnaud
  
  //virtual DistributionBuilderPtr getBuilder() const
  //  {return new PositiveIntegerGaussianDistributionBuilder();}
  
  juce_UseDebuggingNewOperator
  
protected:  
  friend class PositiveIntegerGaussianDistributionClass;
};
  
typedef ReferenceCountedObjectPtr<PositiveIntegerGaussianDistribution> PositiveIntegerGaussianDistributionPtr;  
  

extern ClassPtr bernoulliDistributionClass;

extern ClassPtr enumerationDistributionClass(TypePtr type = enumValueType);
inline ClassPtr enumerationDistributionClass(EnumerationPtr enumeration)
  {return enumerationDistributionClass((TypePtr)enumeration);}

inline EnumerationDistributionPtr enumerationDistribution(EnumerationPtr enumeration, const std::vector<double>& probabilities)
  {return enumerationDistribution((TypePtr)enumeration, probabilities);}

inline EnumerationDistributionPtr enumerationDistribution(EnumerationPtr enumeration)
  {return enumerationDistribution((TypePtr)enumeration);}

}; /* namespace lbcpp */

#endif // !LBCPP_DISCRETE_PROBABILITY_DISTRIBUTION_H_
