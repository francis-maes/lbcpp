/*-----------------------------------------.---------------------------------.
| Filename: PropertyFinder.h               | Property Finder                 |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2011 16:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_PROPERTY_FINDER_H_
# define LBCPP_PROPERTY_FINDER_H_

# include <lbcpp/Function/Predicate.h>

namespace lbcpp
{

//////////////////////////////////////

class Property : public Object
{
public:
  virtual FunctionPtr getUnaryFunction() const = 0;
  virtual SamplerPtr getOutputSampler() const = 0;
  virtual size_t getNumberOfMeasures() const = 0;
};

typedef ReferenceCountedObjectPtr<Property> PropertyPtr;

extern ClassPtr propertyClass;

class PredicateProperty : public Property
{
public:
  PredicateProperty(PredicatePtr predicate, double probability, size_t numberOfMeasures)
    : predicate(predicate), probability(probability), numberOfMeasures(numberOfMeasures) {}
  PredicateProperty() : probability(0.0), numberOfMeasures(0) {}

  virtual FunctionPtr getUnaryFunction() const
    {return predicate;}

  virtual SamplerPtr getOutputSampler() const
    {return bernoulliSampler(probability);}

  virtual size_t getNumberOfMeasures() const
    {return numberOfMeasures;}

protected:
  friend class PredicatePropertyClass;

  PredicatePtr predicate;
  double probability;
  size_t numberOfMeasures;
};

//////////////////////////////////////

class PropertyList : public ObjectVector
{
public:
  PropertyList() : ObjectVector(propertyClass, 0) {}

  size_t getNumProperties() const
    {return getNumElements();}

  const PropertyPtr& getProperty(size_t index) const
    {return get(index).staticCast<Property>();}

  void setProperty(size_t index, const PropertyPtr& property)
    {set(index, property);}

  void addPredicateProperty(const PredicatePtr& predicate, double probability, size_t numberOfMeasures)
    {append(new PredicateProperty(predicate, probability, numberOfMeasures));}
};

typedef ReferenceCountedObjectPtr<PropertyList> PropertyListPtr;

//////////////////////////////////////

class Analyser : public Object
{
public:
  virtual void findProperties(ExecutionContext& context, const ContainerPtr& data, PropertyListPtr& res) = 0;
};

typedef ReferenceCountedObjectPtr<Analyser> AnalyserPtr;

class CompositeAnalyser : public Analyser
{
public:
  virtual void findProperties(ExecutionContext& context, const ContainerPtr& data, PropertyListPtr& res)
  {
    for (size_t i = 0; i < analysers.size(); ++i)
      analysers[i]->findProperties(context, data, res);
  }

  void addAnalyser(const AnalyserPtr& analyser)
    {analysers.push_back(analyser);}

protected:
  friend class CompositeAnalyserClass;

  std::vector<AnalyserPtr> analysers;
};

typedef ReferenceCountedObjectPtr<CompositeAnalyser> CompositeAnalyserPtr;

//////////////////////////////////////
/////////// Double ///////////////////
//////////////////////////////////////
class DoubleRangePredicate : public Predicate
{
public:
  DoubleRangePredicate(double lowerLimit = 0.0, double upperLimit = 1.0)
    : lowerLimit(lowerLimit), upperLimit(upperLimit) {}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}

  virtual bool computePredicate(ExecutionContext& context, const Variable& input) const
    {double value = input.toDouble(); return value >= lowerLimit && value <= upperLimit;}

protected:
  friend class DoubleRangePredicateClass;

  double lowerLimit;
  double upperLimit;
};

class DoubleRangeAnalyser : public Analyser
{
public:
  virtual void findProperties(ExecutionContext& context, const ContainerPtr& data, PropertyListPtr& res)
  {
    size_t n = data->getNumElements();
    if (!n)
      return;

    double minValue = DBL_MAX;
    double maxValue = -DBL_MAX;

    for (size_t i = 0; i < n; ++i)
    {
      double value = data->getElement(i).toDouble();
      if (value > maxValue)
        maxValue = value;
      else if (value < minValue)
        minValue = value;
    }
    
    if (minValue == maxValue)
      res->addPredicateProperty(equalToPredicate(minValue), 1.0, n);
    else
      res->addPredicateProperty(new DoubleRangePredicate(minValue, maxValue), 1.0, n);
  }
};

class AnalyserSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    DenseDoubleVectorPtr values = new DenseDoubleVector(100, 0.0);
    for (size_t i = 0; i < 100; ++i)
      values->setValue(i, random->sampleDoubleFromGaussian(1.0, 1.0));

    CompositeAnalyserPtr analyser = new CompositeAnalyser();
    analyser->addAnalyser(new DoubleRangeAnalyser());

    PropertyListPtr properties = new PropertyList();
    analyser->findProperties(context, values, properties);

    context.resultCallback(T("properties"), properties);
    return true;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_PROPERTY_FINDER_H_
