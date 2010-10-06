/*-----------------------------------------.---------------------------------.
| Filename: Perception.h                   | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_H_
# define LBCPP_DATA_PERCEPTION_H_

# include "../Data/Vector.h"
# include "../Data/DynamicObject.h"
# include "../Function/Function.h"

namespace lbcpp
{

class PerceptionCallback : public Object
{
public:
  virtual void sense(size_t variableNumber, const Variable& value) = 0;
  virtual void sense(size_t variableNumber, double value)
    {sense(variableNumber, Variable(value));}
  virtual void sense(size_t variableNumber, ObjectPtr value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input);
};

typedef ReferenceCountedObjectPtr<PerceptionCallback> PerceptionCallbackPtr;

class Perception : public Function
{
public:
  virtual TypePtr getOutputType() const;
  virtual String getPreferedOutputClassName() const;

  virtual bool isSparse() const
    {return false;}

  virtual size_t getNumOutputVariables() const = 0;
  virtual TypePtr getOutputVariableType(size_t index) const = 0;
  virtual String getOutputVariableName(size_t index) const = 0;
  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {return PerceptionPtr();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const = 0;

  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return getOutputType();}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const;

  PerceptionPtr flatten() const;
  PerceptionPtr addPreprocessor(FunctionPtr preProcessingFunction) const;

  static PerceptionPtr compose(FunctionPtr preProcessingFunction, PerceptionPtr perception)
    {return perception->addPreprocessor(preProcessingFunction);}

  juce_UseDebuggingNewOperator

protected:
  static String classNameToOutputClassName(const String& className);

private:
  friend class PerceptionClass;

  CriticalSection outputTypeLock;
  DynamicClassPtr outputType;

  TypePtr ensureTypeIsComputed();
};

extern ClassPtr perceptionClass();
typedef ReferenceCountedObjectPtr<Perception> PerceptionPtr;

class VariableVectorPerception : public Perception
{
public:
  virtual size_t getNumOutputVariables() const
    {return outputVariables.size();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].type;}

  virtual String getOutputVariableName(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].name;}

  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].subPerception;}

protected:
  struct OutputVariable
  {
    TypePtr type;
    String name;
    PerceptionPtr subPerception;
  };
  std::vector<OutputVariable> outputVariables;

  void addOutputVariable(TypePtr type, const String& name, PerceptionPtr subPerception)
  {
    OutputVariable v;
    v.type = type;
    v.name = name;
    v.subPerception = subPerception;
    outputVariables.push_back(v);
  }
};

typedef ReferenceCountedObjectPtr<VariableVectorPerception> VariableVectorPerceptionPtr;

class DecoratorPerception : public Perception
{
public:
  DecoratorPerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated) {}

  virtual String getPreferedOutputClassName() const
    {return decorated->getPreferedOutputClassName();}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}
  
  virtual bool isSparse() const
    {return decorated->isSparse();}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual size_t getNumOutputVariables() const
    {return decorated->getNumOutputVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return decorated->getOutputVariableType(index);}

  virtual String getOutputVariableName(size_t index) const
    {return decorated->getOutputVariableName(index);}

  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {return decorated->getOutputVariableSubPerception(index);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {decorated->computePerception(input, callback);}

  juce_UseDebuggingNewOperator

protected:
  friend class DecoratorPerceptionClass;

  PerceptionPtr decorated;
};

typedef ReferenceCountedObjectPtr<DecoratorPerception> DecoratorPerceptionPtr;
extern ClassPtr decoratorPerceptionClass();

class CompositePerception : public Perception
{
public:
  CompositePerception(TypePtr inputType, const String& preferedOutputClassName);
  CompositePerception() {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual String getPreferedOutputClassName() const
    {return preferedOutputClassName;}

  size_t getNumPerceptions() const;
  String getPerceptionName(size_t index) const;
  PerceptionPtr getPerception(size_t index) const;
  virtual void addPerception(const String& name, PerceptionPtr subPerception);

  // Perception
  virtual size_t getNumOutputVariables() const
    {return getNumPerceptions();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return getPerception(index)->getOutputType();}

  virtual String getOutputVariableName(size_t index) const
    {return getPerceptionName(index);}

  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {return getPerception(index);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

  juce_UseDebuggingNewOperator

protected:
  friend class CompositePerceptionClass;
  TypePtr inputType;
  String preferedOutputClassName;
  VectorPtr subPerceptions;
  std::vector<std::pair<String, PerceptionPtr> > subPerceptionsCopy;
};

typedef ReferenceCountedObjectPtr<CompositePerception> CompositePerceptionPtr;

inline CompositePerceptionPtr compositePerception(TypePtr inputType, const String& preferedOutputClassName)
  {return new CompositePerception(inputType, preferedOutputClassName);}

extern ClassPtr compositePerceptionClass();

// special perceptions
extern PerceptionPtr nullPerception();
extern PerceptionPtr identityPerception();
extern PerceptionPtr identityPerception(TypePtr type);
extern PerceptionPtr selectAndMakeProductsPerception(PerceptionPtr decorated, FunctionPtr multiplyFunction, ContainerPtr selectedConjunctions);
extern PerceptionPtr selectAndMakeConjunctionFeatures(PerceptionPtr decorated, ContainerPtr selectedConjunctions);

// container perceptions
extern PerceptionPtr windowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception = PerceptionPtr());
extern PerceptionPtr containerHistogramPerception(TypePtr elementsType, bool useCache = true); // Container input
extern PerceptionPtr windowHistogramPerception(TypePtr elementsType, size_t windowSize, bool useCache = true); // (Container, CentralIndex) input
extern PerceptionPtr segmentHistogramPerception(TypePtr elementsType, bool useCache = true); // (Container, IndexPair) input
extern PerceptionPtr boundsProximityPerception();

// probability distribution perceptions
extern PerceptionPtr discreteProbabilityDistributionPerception(EnumerationPtr enumeration);

// modifier perceptions
extern PerceptionPtr functionBasedPerception(FunctionPtr function);
extern DecoratorPerceptionPtr preprocessPerception(FunctionPtr preProcessingFunction, PerceptionPtr perception);
extern PerceptionPtr flattenPerception(PerceptionPtr perception);
extern PerceptionPtr collapsePerception(PerceptionPtr perception);

// product perceptions
extern PerceptionPtr productPerception(FunctionPtr multiplyFunction, PerceptionPtr perception1, PerceptionPtr perception2, bool symmetricFunction, bool singleInputForBothPerceptions = false);
extern PerceptionPtr productPerception(FunctionPtr multiplyFunction, PerceptionPtr perception1, TypePtr type2);
extern PerceptionPtr productPerception(FunctionPtr multiplyFunction, TypePtr type1, PerceptionPtr perception2);
extern PerceptionPtr conjunctionFeatures(PerceptionPtr perception1, PerceptionPtr perception2);

// boolean / enumeration features
extern PerceptionPtr booleanFeatures();
extern PerceptionPtr enumValueFeatures(EnumerationPtr enumeration);

// number features
extern PerceptionPtr hardDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures);
extern PerceptionPtr softDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures, bool cyclicBehavior);
extern PerceptionPtr softDiscretizedLogNumberFeatures(TypePtr inputType, double minimumLogValue, double maximumLogValue, size_t numIntervals, bool doOutOfBoundsFeatures);

extern PerceptionPtr signedNumberFeatures(PerceptionPtr positiveNumberPerception);

extern PerceptionPtr defaultPositiveIntegerFeatures(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultIntegerFeatures(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultDoubleFeatures(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultPositiveDoubleFeatures(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultProbabilityFeatures(size_t numIntervals = 5);

// bi variables
extern DecoratorPerceptionPtr biVariableFeatures(TypePtr firstElementType, TypePtr secondElementType, PerceptionPtr subPerception);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_H_
