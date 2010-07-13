/*-----------------------------------------.---------------------------------.
| Filename: FlattenPerception.h            | A decorator to flatten a        |
| Author  : Francis Maes                   | Perception                      |
| Started : 12/07/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_FLATTEN_H_
# define LBCPP_DATA_PERCEPTION_FLATTEN_H_

# include <lbcpp/Data/Perception.h>

namespace lbcpp
{

class FlattenPerception : public DecoratorPerception
{
public:
  FlattenPerception(PerceptionPtr decorated = PerceptionPtr())
    : DecoratorPerception(decorated)
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

  struct FlattenCallback : public PerceptionCallback
  {
    FlattenCallback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const std::map<PerceptionPtr, size_t>& offsets)
      : targetCallback(targetCallback), offsets(offsets)
      {offset = offsets.find(targetRepresentation)->second;}

    virtual void sense(size_t variableNumber, const Variable& value)
      {targetCallback->sense(variableNumber + offset, value);}

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
      {subPerception->computePerception(input, new FlattenCallback(subPerception, targetCallback, offsets));}

  private:
    PerceptionCallbackPtr targetCallback;
    const std::map<PerceptionPtr, size_t>& offsets;
    size_t offset;
  };

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    PerceptionCallbackPtr decoratedVisitor(new FlattenCallback(decorated, callback, offsets));
    DecoratorPerception::computePerception(input, decoratedVisitor);
  }

private:
  std::vector< std::pair<TypePtr, String> > outputVariables;
  std::map<PerceptionPtr, size_t> offsets;

  void precompute(PerceptionPtr representation, const String& fullName)
  {
    offsets[representation] = outputVariables.size();
    size_t n = representation->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      String name = fullName;
      if (name.isNotEmpty())
        name += '.';
      name += representation->getOutputVariableName(i);

      PerceptionPtr subPerception = representation->getOutputVariableGenerator(i);
      if (subPerception)
        precompute(subPerception, name);
      else
        outputVariables.push_back(std::make_pair(representation->getOutputVariableType(i), name));
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_FLATTEN_H_
