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
# include "Function.h"

namespace lbcpp
{

class PerceptionCallback : public Object
{
public:
  virtual void sense(size_t variableNumber, const Variable& value) = 0;
  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input);
};

typedef ReferenceCountedObjectPtr<PerceptionCallback> PerceptionCallbackPtr;

class Perception : public Function
{
public:
  virtual TypePtr getOutputType() const;
  virtual String getPreferedOutputClassName() const;

  virtual size_t getNumOutputVariables() const = 0;
  virtual TypePtr getOutputVariableType(size_t index) const = 0;
  virtual String getOutputVariableName(size_t index) const = 0;
  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return PerceptionPtr();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const = 0;
  
  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return getOutputType();}
    
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const;

  PerceptionPtr flatten() const;
  PerceptionPtr addPreprocessor(FunctionPtr preProcessingFunction) const;

  static PerceptionPtr compose(FunctionPtr preProcessingFunction, PerceptionPtr representation)
    {return representation->addPreprocessor(preProcessingFunction);}

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

class DecoratorPerception : public Perception
{
public:
  DecoratorPerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated) {}

  virtual String getPreferedOutputClassName() const
    {return decorated->getPreferedOutputClassName();}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual size_t getNumOutputVariables() const
    {return decorated->getNumOutputVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return decorated->getOutputVariableType(index);}

  virtual String getOutputVariableName(size_t index) const
    {return decorated->getOutputVariableName(index);}
  
  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return decorated->getOutputVariableGenerator(index);}

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
  CompositePerception();

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

  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return getPerception(index);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

  juce_UseDebuggingNewOperator

protected:
  friend class CompositePerceptionClass;
  VectorPtr subPerceptions;
};

typedef ReferenceCountedObjectPtr<CompositePerception> CompositePerceptionPtr;

extern ClassPtr compositePerceptionClass();

extern PerceptionPtr nullPerception();
extern PerceptionPtr identityPerception(TypePtr type);
extern PerceptionPtr identityPerception();
extern PerceptionPtr windowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception = PerceptionPtr());
extern PerceptionPtr histogramPerception(TypePtr elementsType, bool useCache = true);
extern DecoratorPerceptionPtr biContainerPerception(size_t windowSize, PerceptionPtr subPerception);
extern PerceptionPtr biVariablePerception(TypePtr firstElementType, TypePtr secondElementType);
extern PerceptionPtr functionBasedPerception(FunctionPtr function);
extern DecoratorPerceptionPtr preprocessPerception(FunctionPtr preProcessingFunction, PerceptionPtr perception);
extern DecoratorPerceptionPtr flattenPerception(PerceptionPtr perception);

// features
extern PerceptionPtr booleanFeatures();
extern PerceptionPtr enumValueFeatures(EnumerationPtr enumeration);
extern DecoratorPerceptionPtr biVariableFeatures(TypePtr firstElementType, TypePtr secondElementType, PerceptionPtr subPerception);  
extern PerceptionPtr perceptionToFeatures(PerceptionPtr perception);
extern PerceptionPtr hardDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures);
extern PerceptionPtr softDiscretizedNumberFeatures(TypePtr Type, size_t numIntervals, bool cycle);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_H_
