/*-----------------------------------------.---------------------------------.
| Filename: RewritedPerception.h           | Rewrited Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 15/09/2010 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_REWRITED_H_
# define LBCPP_FUNCTION_PERCEPTION_REWRITED_H_

# include <lbcpp/Perception/PerceptionRewriter.h>
# include <lbcpp/Data/XmlSerialisation.h>

namespace lbcpp
{

class RewritedPerception : public Perception
{
public:
  RewritedPerception(PerceptionPtr decorated, PerceptionRewriterPtr rewriter, const std::vector<String>& stack)
    : decorated(decorated), rewriter(rewriter), stack(stack)
    {computeOutputType();}
  RewritedPerception() {}

  virtual String toString() const
    {return decorated->toString() + T(" rewrited");}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    Callback callback(decorated, targetCallback, this);
    callback.setStaticAllocationFlag();
    decorated->computePerception(input, &callback);
  }

  juce_UseDebuggingNewOperator

protected:
  friend class PerceptionRewriteRules;
  friend class RewritedPerceptionClass;

  PerceptionPtr decorated;
  PerceptionRewriterPtr rewriter;
  std::vector<String> stack;

  std::vector< std::pair<size_t, PerceptionPtr> > variablesMap; // decorated perception variable index -> perception

  virtual void computeOutputType()
  {
    jassert(outputVariables.size() == 0);
    std::vector<String> stack = this->stack;
    computeOutputVariablesRecursively(rewriter, stack);
    Perception::computeOutputType();
  }

  void computeOutputVariablesRecursively(PerceptionRewriterPtr rewriter, std::vector<String>& stack)
  {
    jassert(rewriter);
    outputVariables.clear();
    variablesMap.clear();

    size_t n = decorated->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      TypePtr variableType = decorated->getOutputVariableType(i);
      String variableName = decorated->getOutputVariableName(i);
      PerceptionPtr variablePerception = decorated->getOutputVariableSubPerception(i);

      stack.push_back(variableName);

      PerceptionPtr newPerception;
      if (variablePerception)
        newPerception = rewriter->rewriteRecursively(variablePerception, stack);
      else
        newPerception = rewriter->applyRules(variableType, stack);

      if (newPerception)
      {
        if (newPerception->getNumOutputVariables() > 0)
        {
          size_t variableIndex = outputVariables.size();
          addOutputVariable(newPerception->getOutputType(variableType), variableName, newPerception);
          if (variablesMap.size() <= i)
            variablesMap.resize(i + 1, std::make_pair(0, PerceptionPtr()));
          variablesMap[i] = std::make_pair(variableIndex, newPerception);
        }
      }

      stack.pop_back();
    }
  }

  struct Callback : public PerceptionCallback
  {
    Callback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const RewritedPerception* owner)
      : targetCallback(targetCallback), owner(owner)
      {}

    template<class Type>
    void senseBuiltinType(size_t variableNumber, const Type& value)
    {
      PerceptionPtr targetPerception = getTargetPerception(variableNumber);
      if (targetPerception)
      {
        if (targetPerception == identityPerception())
          targetCallback->sense(variableNumber, value);
        else
          targetCallback->sense(variableNumber, targetPerception, Variable(value, targetPerception->getInputType()));
      }
    }

    virtual void sense(size_t variableNumber, size_t value)
      {senseBuiltinType(variableNumber, value);}

    virtual void sense(size_t variableNumber, int value)
      {senseBuiltinType(variableNumber, value);}

    virtual void sense(size_t variableNumber, double value)
      {senseBuiltinType(variableNumber, value);}
      
    virtual void sense(size_t variableNumber, const String& value)
      {senseBuiltinType(variableNumber, value);}

    virtual void sense(size_t variableNumber, ObjectPtr value)
      {senseBuiltinType(variableNumber, value);}

    virtual void sense(size_t variableNumber, const Variable& value)
    {
      PerceptionPtr targetPerception = getTargetPerception(variableNumber);
      if (targetPerception)
      {
        if (targetPerception == identityPerception())
          targetCallback->sense(variableNumber, value);
        else
          targetCallback->sense(variableNumber, targetPerception, value);
      }
    }

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {
      PerceptionPtr targetPerception = getTargetPerception(variableNumber);
      if (targetPerception)
        targetCallback->sense(variableNumber, targetPerception, input);
    }

  private:
    PerceptionCallbackPtr targetCallback;
    const RewritedPerception* owner;

    PerceptionPtr getTargetPerception(size_t& variableNumber) const
    {
      if (variableNumber >= owner->variablesMap.size())
        return PerceptionPtr();
      std::pair<size_t, PerceptionPtr> info = owner->variablesMap[variableNumber];
      if (!info.second)
        return PerceptionPtr();
      variableNumber = info.first;
      return info.second;
    }
  };
};

typedef ReferenceCountedObjectPtr<RewritedPerception> RewritedPerceptionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_REWRITED_H_
