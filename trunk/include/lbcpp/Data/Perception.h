/*-----------------------------------------.---------------------------------.
| Filename: Perception.h                   | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_H_
# define LBCPP_DATA_PERCEPTION_H_

# include "../ObjectPredeclarations.h"
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

  virtual size_t getNumOutputVariables() const = 0;
  virtual TypePtr getOutputVariableType(size_t index) const = 0;
  virtual String getOutputVariableName(size_t index) const = 0;
  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return PerceptionPtr();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const = 0;
  
  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return getOutputType();}
    
  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const;

  PerceptionPtr flatten() const;
  PerceptionPtr addPreprocessor(FunctionPtr preProcessingFunction) const;

  static PerceptionPtr compose(FunctionPtr preProcessingFunction, PerceptionPtr representation)
    {return representation->addPreprocessor(preProcessingFunction);}

private:
  DynamicClassPtr type;

  void ensureTypeIsComputed();
};

extern ClassPtr perceptionClass();

class DecoratorPerception : public Perception
{
public:
  DecoratorPerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated) {}

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

protected:
  PerceptionPtr decorated;
};

class CompositePerception : public Perception
{
public:
  size_t getNumPerceptions() const
    {return subPerceptions.size();}

  String getPerceptionName(size_t index) const
    {jassert(index < subPerceptions.size()); return subPerceptions[index].first;}

  PerceptionPtr getPerception(size_t index) const
    {jassert(index < subPerceptions.size()); return subPerceptions[index].second;}

  virtual void addPerception(const String& name, PerceptionPtr subPerception)
    {subPerceptions.push_back(std::make_pair(name, subPerception));}

  // Perception
  virtual size_t getNumOutputVariables() const;
  virtual TypePtr getOutputVariableType(size_t index) const;
  virtual String getOutputVariableName(size_t index) const;
  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const;
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

protected:
  std::vector< std::pair<String, PerceptionPtr> > subPerceptions;
};

typedef ReferenceCountedObjectPtr<CompositePerception> CompositePerceptionPtr;

PerceptionPtr identityPerception(TypePtr type);
PerceptionPtr windowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception = PerceptionPtr());

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_H_
