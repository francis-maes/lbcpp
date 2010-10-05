/*-----------------------------------------.---------------------------------.
| Filename: FlattenPerception.h            | A decorator to flatten a        |
| Author  : Francis Maes                   | Perception                      |
| Started : 12/07/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_
# define LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_

# include <lbcpp/Function/Perception.h>

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

  virtual String getPreferedOutputClassName() const
    {return T("flat ") + decorated->getPreferedOutputClassName();}

  virtual TypePtr getOutputType() const
    {return Perception::getOutputType();}

  virtual size_t getNumOutputVariables() const
    {return outputVariables.size();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].first;}

  virtual String getOutputVariableName(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].second;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    PerceptionCallbackPtr decoratedVisitor(new FlattenCallback(decorated, callback, offsets));
    DecoratorPerception::computePerception(input, decoratedVisitor);
  }

  juce_UseDebuggingNewOperator

private:
  std::vector< std::pair<TypePtr, String> > outputVariables;
  std::map< std::vector<size_t> , size_t> offsets;

  void precompute(PerceptionPtr perception, const String& fullName, const std::vector<size_t>& stack = std::vector<size_t>())
  {
    offsets[stack] = outputVariables.size();

    size_t n = perception->getNumOutputVariables();
    std::vector<size_t> newStack(stack);
    newStack.push_back(0);
    for (size_t i = 0; i < n; ++i)
    {
      String name = fullName;
      if (name.isNotEmpty())
        name += '.';
      name += perception->getOutputVariableName(i);

      newStack.back() = i;

      PerceptionPtr subPerception = perception->getOutputVariableSubPerception(i);
      if (subPerception)
        precompute(subPerception, name, newStack);
      else
        outputVariables.push_back(std::make_pair(perception->getOutputVariableType(i), name));
    }
  }

  struct FlattenCallback : public PerceptionCallback
  {
    FlattenCallback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const std::map< std::vector<size_t> , size_t>& offsets)
      : targetCallback(targetCallback), offsets(offsets)
      {updateOffset();}

    virtual void sense(size_t variableNumber, const Variable& value)
      {targetCallback->sense(variableNumber + offset, value);}

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {
      stack.push_back(variableNumber);
      updateOffset();

      subPerception->computePerception(input, PerceptionCallbackPtr(this));

      stack.pop_back();
      updateOffset();
    }

  private:
    PerceptionCallbackPtr targetCallback;
    const std::map< std::vector<size_t> , size_t>& offsets;
    std::vector<size_t> stack;
    size_t offset;

    void updateOffset()
    {
      std::map< std::vector<size_t> , size_t>::const_iterator it = offsets.find(stack);
      jassert(it != offsets.end());
      offset = it->second;
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_
