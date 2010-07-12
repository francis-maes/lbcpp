/*-----------------------------------------.---------------------------------.
| Filename: Representation.cpp             | Representation                  |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Representation.h>
#include <lbcpp/Data/Vector.h> // for DynamicObject
using namespace lbcpp;

void RepresentationCallback::sense(size_t variableNumber, RepresentationPtr subRepresentation, const Variable& input)
  {sense(variableNumber, subRepresentation->compute(input));}

TypePtr Representation::getOutputType() const
  {const_cast<Representation* >(this)->ensureTypeIsComputed(); return type;}

class SetInObjectRepresentationCallback : public RepresentationCallback
{
public:
  SetInObjectRepresentationCallback(ObjectPtr target)
    : target(target) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {target->setVariable(variableNumber, value);}

private:
  ObjectPtr target;
};

Variable Representation::computeFunction(const Variable& input, ErrorHandler& callback) const
{
  const_cast<Representation* >(this)->ensureTypeIsComputed();
  DynamicObjectPtr res = new DynamicObject(type);
  computeRepresentation(input, RepresentationCallbackPtr(new SetInObjectRepresentationCallback(res)));
  return res;
}

void Representation::ensureTypeIsComputed()
{
  if (type)
    return;
  type = new DynamicClass(getClassName() + T("Class"));
  size_t n = getNumOutputVariables();
  for (size_t i = 0; i < n; ++i)
    type->addVariable(getOutputVariableType(i), getOutputVariableName(i));
}

/*
** CompositeRepresentation
*/
size_t CompositeRepresentation::getNumOutputVariables() const
  {return subRepresentations.size();}

TypePtr CompositeRepresentation::getOutputVariableType(size_t index) const
{
  jassert(index < subRepresentations.size());
  return subRepresentations[index].second->getOutputType();
}

String CompositeRepresentation::getOutputVariableName(size_t index) const
{
  jassert(index < subRepresentations.size());
  return subRepresentations[index].first;
}

RepresentationPtr CompositeRepresentation::getOutputVariableGenerator(size_t index) const
{
  jassert(index < subRepresentations.size());
  return subRepresentations[index].second;
}

void CompositeRepresentation::computeRepresentation(const Variable& input, RepresentationCallbackPtr callback) const
{
  for (size_t i = 0; i < subRepresentations.size(); ++i)
    callback->sense(i, subRepresentations[i].second, input);
}

#include "Representation/PreprocessRepresentation.h"
#include "Representation/FlattenRepresentation.h"

RepresentationPtr Representation::addPreprocessor(FunctionPtr preProcessingFunction) const
  {return new PreprocessRepresentation(preProcessingFunction, RepresentationPtr(const_cast<Representation* >(this)));}

RepresentationPtr Representation::flatten() const
  {return new FlattenRepresentation(RepresentationPtr(const_cast<Representation* >(this)));}

#include "Representation/VectorWindowRepresentation.h"

RepresentationPtr lbcpp::vectorWindowRepresentation(TypePtr elementsType, size_t windowSize)
  {return new VectorWindowRepresentation(elementsType, windowSize);}

void declareRepresentationClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Representation, Function);
    LBCPP_DECLARE_ABSTRACT_CLASS(CompositeRepresentation, Representation);
    LBCPP_DECLARE_CLASS(VectorWindowRepresentation, Representation);
  
    LBCPP_DECLARE_ABSTRACT_CLASS(DecoratorRepresentation, Representation);
      LBCPP_DECLARE_CLASS(PreprocessRepresentation, DecoratorRepresentation);
      LBCPP_DECLARE_CLASS(FlattenRepresentation, DecoratorRepresentation);
}
