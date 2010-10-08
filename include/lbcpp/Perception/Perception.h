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
  // Perception
  virtual TypePtr getOutputType() const;

  virtual void computeOutputType();
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const = 0;

  virtual bool isSparse() const
    {return false;}

  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return getOutputType();}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const;

  // Object
  virtual bool loadFromXml(XmlImporter& importer);
  virtual String toString() const;

  // output variables
  struct OutputVariable
  {
    TypePtr type;
    String name;
    PerceptionPtr subPerception;
  };

  const std::vector<OutputVariable>& getOutputVariables() const
    {return outputVariables;}

  size_t getNumOutputVariables() const
    {return outputVariables.size();}

  TypePtr getOutputVariableType(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].type;}

  String getOutputVariableName(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].name;}

  PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].subPerception;}

  juce_UseDebuggingNewOperator

protected:
  std::vector<OutputVariable> outputVariables;

  void reserveOutputVariables(size_t count)
    {jassert(outputVariables.empty()); outputVariables.reserve(count);}

  void addOutputVariable(const String& name, PerceptionPtr subPerception)
    {addOutputVariable(subPerception->getOutputType(), name, subPerception);}

  void addOutputVariable(const String& name, TypePtr type)
    {addOutputVariable(type, name, PerceptionPtr());}

  void addOutputVariable(TypePtr type, const String& name, PerceptionPtr subPerception)
  {
    OutputVariable v;
    v.type = type;
    v.name = name;
    v.subPerception = subPerception;
    outputVariables.push_back(v);
  }

  static String classNameToOutputClassName(const String& className);

private:
  friend class PerceptionClass;
  DynamicClassPtr outputType;
};

extern ClassPtr perceptionClass();
typedef ReferenceCountedObjectPtr<Perception> PerceptionPtr;

class CompositePerception : public Perception
{
public:
  CompositePerception(TypePtr inputType, const String& stringDescription);
  CompositePerception() {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual String toString() const
    {return stringDescription;}

  size_t getNumPerceptions() const;
  String getPerceptionName(size_t index) const;
  PerceptionPtr getPerception(size_t index) const;
  virtual void addPerception(const String& name, PerceptionPtr subPerception);

  // Perception
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

  juce_UseDebuggingNewOperator

protected:
  virtual void computeOutputType();

  friend class CompositePerceptionClass;

  TypePtr inputType;
  String stringDescription;
  VectorPtr subPerceptions;
};

typedef ReferenceCountedObjectPtr<CompositePerception> CompositePerceptionPtr;

inline CompositePerceptionPtr compositePerception(TypePtr inputType, const String& stringDescription)
  {return new CompositePerception(inputType, stringDescription);}

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
extern PerceptionPtr composePerception(FunctionPtr function, PerceptionPtr perception);
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

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_H_
