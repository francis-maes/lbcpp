/*-----------------------------------------.---------------------------------.
| Filename: Representation.h               | Representation                  |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_REPRESENTATION_H_
# define LBCPP_DATA_REPRESENTATION_H_

# include "../ObjectPredeclarations.h"
# include "Function.h"

namespace lbcpp
{

class RepresentationCallback : public Object
{
public:
  virtual void sense(size_t variableNumber, const Variable& value) = 0;
  virtual void sense(size_t variableNumber, RepresentationPtr subRepresentation, const Variable& input);
};

typedef ReferenceCountedObjectPtr<RepresentationCallback> RepresentationCallbackPtr;

class Representation : public Function
{
public:
  virtual TypePtr getOutputType() const;

  virtual size_t getNumOutputVariables() const = 0;
  virtual TypePtr getOutputVariableType(size_t index) const = 0;
  virtual String getOutputVariableName(size_t index) const = 0;
  virtual RepresentationPtr getOutputVariableGenerator(size_t index) const
    {return RepresentationPtr();}

  virtual void computeRepresentation(const Variable& input, RepresentationCallbackPtr callback) const = 0;
  
  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return getOutputType();}
    
  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const;

  RepresentationPtr flatten() const;
  RepresentationPtr addPreprocessor(FunctionPtr preProcessingFunction) const;

  static RepresentationPtr compose(FunctionPtr preProcessingFunction, RepresentationPtr representation)
    {return representation->addPreprocessor(preProcessingFunction);}

private:
  DynamicClassPtr type;

  void ensureTypeIsComputed();
};

class DecoratorRepresentation : public Representation
{
public:
  DecoratorRepresentation(RepresentationPtr decorated = RepresentationPtr())
    : decorated(decorated) {}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual size_t getNumOutputVariables() const
    {return decorated->getNumOutputVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return decorated->getOutputVariableType(index);}

  virtual String getOutputVariableName(size_t index) const
    {return decorated->getOutputVariableName(index);}

  virtual void computeRepresentation(const Variable& input, RepresentationCallbackPtr visitor) const
    {decorated->computeRepresentation(input, visitor);}

protected:
  RepresentationPtr decorated;
};

RepresentationPtr vectorWindowRepresentation(TypePtr elementsType, size_t windowSize);

class CompositeRepresentation : public Representation
{
public:
  size_t getNumRepresentations() const
    {return subRepresentations.size();}

  String getRepresentationName(size_t index) const
    {jassert(index < subRepresentations.size()); return subRepresentations[index].first;}

  RepresentationPtr getRepresentation(size_t index) const
    {jassert(index < subRepresentations.size()); return subRepresentations[index].second;}

  void addRepresentation(const String& name, RepresentationPtr subRepresentation)
    {subRepresentations.push_back(std::make_pair(name, subRepresentation));}

  // Representation
  virtual size_t getNumOutputVariables() const;
  virtual TypePtr getOutputVariableType(size_t index) const;
  virtual String getOutputVariableName(size_t index) const;
  virtual RepresentationPtr getOutputVariableGenerator(size_t index) const;
  virtual void computeRepresentation(const Variable& input, RepresentationCallbackPtr callback) const;

protected:
  std::vector< std::pair<String, RepresentationPtr> > subRepresentations;
};

typedef ReferenceCountedObjectPtr<CompositeRepresentation> CompositeRepresentationPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_REPRESENTATION_H_
