/*-----------------------------------------.---------------------------------.
| Filename: FlattenRepresentation.h        | A decorator to flatten a        |
| Author  : Francis Maes                   | Representation                  |
| Started : 12/07/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_REPRESENTATION_FLATTEN_H_
# define LBCPP_DATA_REPRESENTATION_FLATTEN_H_

# include <lbcpp/Data/Representation.h>

namespace lbcpp
{

class FlattenRepresentation : public DecoratorRepresentation
{
public:
  FlattenRepresentation(RepresentationPtr decorated = RepresentationPtr())
    : DecoratorRepresentation(decorated)
  {
    if (decorated)
      precompute(decorated, String::empty);
  }

  virtual size_t getNumOutputVariables() const
    {return outputVariables.size();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].first;}

  virtual String getOutputVariableName(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].second;}

  struct FlattenCallback : public RepresentationCallback
  {
    FlattenCallback(RepresentationPtr targetRepresentation, RepresentationCallbackPtr targetCallback, const std::map<RepresentationPtr, size_t>& offsets)
      : targetCallback(targetCallback), offsets(offsets)
      {offset = offsets.find(targetRepresentation)->second;}

    virtual void sense(size_t variableNumber, const Variable& value)
      {targetCallback->sense(variableNumber + offset, value);}

    virtual void sense(size_t variableNumber, RepresentationPtr subRepresentation, const Variable& input)
      {subRepresentation->computeRepresentation(input, new FlattenCallback(subRepresentation, targetCallback, offsets));}

  private:
    RepresentationCallbackPtr targetCallback;
    const std::map<RepresentationPtr, size_t>& offsets;
    size_t offset;
  };

  virtual void computeRepresentation(const Variable& input, RepresentationCallbackPtr callback) const
  {
    RepresentationCallbackPtr decoratedVisitor(new FlattenCallback(decorated, callback, offsets));
    DecoratorRepresentation::computeRepresentation(input, decoratedVisitor);
  }

private:
  std::vector< std::pair<TypePtr, String> > outputVariables;
  std::map<RepresentationPtr, size_t> offsets;

  void precompute(RepresentationPtr representation, const String& fullName)
  {
    offsets[representation] = outputVariables.size();
    size_t n = representation->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      String name = fullName;
      if (name.isNotEmpty())
        name += '.';
      name += representation->getOutputVariableName(i);

      RepresentationPtr subRepresentation = representation->getOutputVariableGenerator(i);
      if (subRepresentation)
        precompute(subRepresentation, name);
      else
        outputVariables.push_back(std::make_pair(representation->getOutputVariableType(i), name));
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_REPRESENTATION_FLATTEN_H_
