/*-----------------------------------------.---------------------------------.
| Filename: CollapsePerception.h           | A decorator to collapse a       |
| Author  : Francis Maes                   | Perception                      |
| Started : 05/10/2010 19:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_COLLAPSE_H_
# define LBCPP_FUNCTION_PERCEPTION_COLLAPSE_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class CollapsePerception : public VariableVectorPerception
{
public:
  CollapsePerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated)
    {precompute(decorated, String::empty);}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual String getPreferedOutputClassName() const
    {return decorated->getPreferedOutputClassName() + T(" collapsed");}

  struct Callback : public PerceptionCallback
  {
    Callback(const CollapsePerception* owner, PerceptionCallbackPtr targetCallback)
      : owner(owner), targetCallback(targetCallback) {}

    virtual void sense(size_t variableNumber, const Variable& value)
      {}

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {
      currentStack.push_back(variableNumber);
      std::map<std::vector<size_t>, size_t>::const_iterator it = owner->pathsToOutputNumber.find(currentStack);
      if (it != owner->pathsToOutputNumber.end())
        targetCallback->sense(it->second, subPerception, input);
      else
        subPerception->computePerception(input, PerceptionCallbackPtr(this));
      currentStack.pop_back();     
    }

    const CollapsePerception* owner;
    PerceptionCallbackPtr targetCallback;
    std::vector<size_t> currentStack;
  };

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    Callback callback(this, targetCallback);
    callback.setStaticAllocationFlag();
    decorated->computePerception(input, &callback);
  }

  juce_UseDebuggingNewOperator

private:
  friend class CollapsePerceptionClass;

  PerceptionPtr decorated;
  std::map<std::vector<size_t>, size_t> pathsToOutputNumber;

  bool isLeafPerception(PerceptionPtr perception) const
  {
    size_t n = perception->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
      if (perception->getOutputVariableSubPerception(i))
        return false;
    return true;
  }

  void precompute(PerceptionPtr perception, const String& fullName, const std::vector<size_t>& stack = std::vector<size_t>())
  {
    if (isLeafPerception(perception))
    {
      pathsToOutputNumber[stack] = outputVariables.size();
      addOutputVariable(perception->getOutputType(), fullName, perception);
    }
    else
    {
      size_t n = perception->getNumOutputVariables();
      std::vector<size_t> newStack(stack);
      newStack.push_back(0);
      for (size_t i = 0; i < n; ++i)
      {
        PerceptionPtr subPerception = perception->getOutputVariableSubPerception(i);
        if (subPerception)
        {
          String newFullName = fullName;
          if (newFullName.isNotEmpty())
            newFullName += '.';
          newFullName += perception->getOutputVariableName(i);

          newStack.back() = i;

          precompute(subPerception, newFullName, newStack);
        }
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_COLLAPSE_H_
