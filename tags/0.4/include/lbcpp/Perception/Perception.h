/*-----------------------------------------.---------------------------------.
| Filename: Perception.h                   | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_H_
# define LBCPP_DATA_PERCEPTION_H_

# include "../Core/Vector.h"
# include "../Core/DynamicObject.h"
# include "../Core/Function.h"
# include "../Data/RandomVariable.h"

namespace lbcpp
{

class Perception;
typedef ReferenceCountedObjectPtr<Perception> PerceptionPtr;

class PerceptionCallback
{
public:
  PerceptionCallback(ExecutionContext& context)
    : context(context) {}
  virtual ~PerceptionCallback() {}

  virtual void sense(size_t variableNumber, const Variable& value) = 0;

  virtual void sense(size_t variableNumber, size_t value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, int value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, double value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, const String& value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input);

protected:
  ExecutionContext& context;
};

typedef PerceptionCallback* PerceptionCallbackPtr; // no ref-counting for PerceptionCallbacks

class Perception : public Function
{
public:
  Perception();

  // Perception
  virtual TypePtr getOutputType() const;
  
  virtual void computeOutputType();
  void clearOutputType();

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const = 0;

  virtual bool isSparse() const
    {return false;}

  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return getOutputType();}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const;

  // Object
  virtual bool loadFromXml(XmlImporter& importer);
  virtual String toString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  // output variables
  struct OutputVariable
  {
    TypePtr type;
    String name;
    PerceptionPtr subPerception;
  };

  const std::vector<OutputVariable>& getOutputVariables() const
    {return outputVariables;}

  size_t getNumOutputVariables() const;
  TypePtr getOutputVariableType(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].type;}

  String getOutputVariableName(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].name;}

  PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].subPerception;}

  lbcpp_UseDebuggingNewOperator

protected:
  std::vector<OutputVariable> outputVariables;

  void reserveOutputVariables(size_t count)
    {jassert(outputVariables.empty()); outputVariables.reserve(count);}

  void addOutputVariable(const String& name, PerceptionPtr subPerception)
    {addOutputVariable(subPerception->getOutputType(), name, subPerception);}

  void addOutputVariable(const String& name, TypePtr type)
    {addOutputVariable(type, name, PerceptionPtr());}

  void clearOutputVariables();
  void addOutputVariable(TypePtr type, const String& name, PerceptionPtr subPerception);

  static String classNameToOutputClassName(const String& className);

private:
  void ensureOutputTypeIsComputed();

  friend class PerceptionClass;
  UnnamedDynamicClassPtr outputType;

  CriticalSection sparsenessLock;
  ScalarVariableRecentMeanAndVariance sparseness;

  void pushSparsenessValue(size_t numActives);
  size_t esimateSparsenessUpperBound() const;
};

extern ClassPtr perceptionClass;
typedef ReferenceCountedObjectPtr<Perception> PerceptionPtr;

class CompositePerception : public Perception
{
public:
  CompositePerception(TypePtr inputType, const String& stringDescription);
  CompositePerception() {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual bool isSparse() const;

  virtual String toString() const
    {return stringDescription;}

  size_t getNumPerceptions() const;
  String getPerceptionName(size_t index) const;
  PerceptionPtr getPerception(size_t index) const;
  virtual void addPerception(const String& name, PerceptionPtr subPerception);

  // Perception
  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const;

  lbcpp_UseDebuggingNewOperator

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

extern ClassPtr compositePerceptionClass;

// special perceptions
extern PerceptionPtr nullPerception();
extern PerceptionPtr identityPerception();
extern PerceptionPtr identityPerception(TypePtr type);

// container perceptions
extern PerceptionPtr windowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception = PerceptionPtr());
extern PerceptionPtr containerHistogramPerception(TypePtr elementsType, bool useCache = true); // Container input
extern PerceptionPtr windowHistogramPerception(TypePtr elementsType, size_t windowSize, bool useCache = true); // (Container, CentralIndex) input
extern PerceptionPtr segmentHistogramPerception(TypePtr elementsType, bool useCache = true); // (Container, IndexPair) input
extern PerceptionPtr boundsProximityPerception();
extern PerceptionPtr inputLabelPairPerception(PerceptionPtr inputPerception, EnumerationPtr classes);
extern PerceptionPtr concatenatePairPerception(const PerceptionPtr& firstPerception, const PerceptionPtr& secondPerception);

// probability distribution perceptions
extern PerceptionPtr discreteDistributionPerception(EnumerationPtr enumeration);

// modifier perceptions
extern PerceptionPtr functionBasedPerception(FunctionPtr function);
extern PerceptionPtr composePerception(FunctionPtr function, PerceptionPtr perception);
extern PerceptionPtr flattenPerception(PerceptionPtr perception);
extern PerceptionPtr collapsePerception(PerceptionPtr perception);
typedef std::vector< std::vector<size_t> > ConjunctionVector;
extern PerceptionPtr selectAndMakeProductsPerception(PerceptionPtr decorated, FunctionPtr multiplyFunction, const ConjunctionVector& selectedConjunctions = ConjunctionVector());

// product perceptions
extern PerceptionPtr productPerception(FunctionPtr multiplyFunction, PerceptionPtr perception1, PerceptionPtr perception2, bool symmetricFunction, bool singleInputForBothPerceptions = false);
extern PerceptionPtr productPerception(FunctionPtr multiplyFunction, PerceptionPtr perception1, TypePtr type2);
extern PerceptionPtr productPerception(FunctionPtr multiplyFunction, TypePtr type1, PerceptionPtr perception2);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_H_
