/*-----------------------------------------.---------------------------------.
| Filename: FlattenPerception.h            | A decorator to flatten a        |
| Author  : Francis Maes                   | Perception                      |
| Started : 12/07/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_
# define LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class FlattenPerception : public Perception
{
public:
  FlattenPerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated)
    {computeOutputType();}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual String toString() const
    {return decorated->toString() + T(" flattened");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    FlattenCallback callback(decorated, targetCallback, offsets);
    decorated->computePerception(input, &callback);
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class FlattenPerceptionClass;

  PerceptionPtr decorated;
  std::map< std::vector<size_t> , size_t> offsets;

  virtual void computeOutputType()
  {
    precompute(decorated, String::empty);
    Perception::computeOutputType();
  }

  void precompute(PerceptionPtr perception, const String& fullName, const std::vector<size_t>& stack = std::vector<size_t>())
  {
    offsets[stack] = outputVariables.size();

    TypePtr perceptionOutputType = perception->getOutputType();
    size_t n = perceptionOutputType->getObjectNumVariables();
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
        addOutputVariable(name, perception->getOutputVariableType(i));
    }
  }

  struct FlattenCallback : public PerceptionCallback
  {
    FlattenCallback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const std::map< std::vector<size_t> , size_t>& offsets)
      : targetCallback(targetCallback), offsets(offsets)
      {updateOffset();}

    virtual void sense(size_t variableNumber, const Variable& value)
      {targetCallback->sense(variableNumber + offset, value);}

    virtual void sense(size_t variableNumber, double value)
      {targetCallback->sense(variableNumber + offset, value);}

    virtual void sense(size_t variableNumber, const ObjectPtr& value)
      {targetCallback->sense(variableNumber + offset, value);}

    virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
    {
      stack.push_back(variableNumber);
      updateOffset();
      subPerception->computePerception(input, this);
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
