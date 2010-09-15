/*-----------------------------------------.---------------------------------.
| Filename: RewritedPerception.h           | Rewrited Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 15/09/2010 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_REWRITED_H_
# define LBCPP_FUNCTION_PERCEPTION_REWRITED_H_

# include <lbcpp/Function/PerceptionRewriter.h>

namespace lbcpp
{

class RewritedPerception : public DecoratorPerception
{
public:
  RewritedPerception(PerceptionPtr decorated, PerceptionRewriterPtr rules, std::vector<String>& stack)
    : DecoratorPerception(decorated), rules(rules) {computeOutputVariables(stack);}
  RewritedPerception() {}

  virtual TypePtr getOutputType() const
    {return Perception::getOutputType();}

  virtual size_t getNumOutputVariables() const
    {return outputVariables.size();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].type;}

  virtual String getOutputVariableName(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].name;}
  
  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {jassert(index < outputVariables.size()); return outputVariables[index].subPerception;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    PerceptionCallbackPtr decoratedVisitor(new Callback(decorated, callback, this));
    DecoratorPerception::computePerception(input, decoratedVisitor);
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!DecoratorPerception::loadFromXml(importer))
      return false;
    std::vector<String> stack;
    computeOutputVariables(stack);
    return true;
  }

protected:
  friend class PerceptionRewriteRules;
  friend class RewritedPerceptionClass;

  PerceptionRewriterPtr rules;

  struct OutputVariable
  {
    TypePtr type;
    String name;
    PerceptionPtr subPerception;
  };

  std::vector<OutputVariable> outputVariables;
  std::vector<int> variablesMap; // decorated perception variable index -> this perception variable index, or -1 if non existent in this perception
  
  void computeOutputVariables(std::vector<String>& stack)
  {
    jassert(rules);
    outputVariables.clear();
    variablesMap.clear();

    size_t n = decorated->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      TypePtr variableType = decorated->getOutputVariableType(i);
      String variableName = decorated->getOutputVariableName(i);
      PerceptionPtr variablePerception = decorated->getOutputVariableGenerator(i);

      stack.push_back(variableName);

      PerceptionPtr newPerception;
      if (variablePerception)
        newPerception = rules->rewriteRecursively(variablePerception, stack);
      else
        newPerception = rules->applyRules(variableType, stack);

      if (newPerception)
      {
         if (newPerception == identityPerception())
           newPerception = PerceptionPtr();
        addOutputVariable(variableType, variableName, PerceptionPtr(), i);
      }

      stack.pop_back();
    }
  }

  void addOutputVariable(TypePtr type, const String& name, PerceptionPtr subPerception, size_t sourceIndex)
  {
    OutputVariable v;
    v.type = type;
    v.name = name;
    v.subPerception = subPerception;
    if (variablesMap.size() <= sourceIndex)
      variablesMap.resize(sourceIndex, -1);
    variablesMap[sourceIndex] = outputVariables.size();
    outputVariables.push_back(v);
  }

  struct Callback : public PerceptionCallback
  {
    Callback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const RewritedPerception* owner)
      : targetCallback(targetCallback), owner(owner)
      {}

    virtual void sense(size_t variableNumber, const Variable& value)
    {
      jassert(variableNumber < owner->variablesMap.size());
      variableNumber = owner->variablesMap[variableNumber];
      if (variableNumber >= 0)
      {
        jassert(variableNumber < owner->outputVariables.size());
        const OutputVariable& outputVariable = owner->outputVariables[variableNumber];
        if (outputVariable.subPerception)
          targetCallback->sense(variableNumber, outputVariable.subPerception, value);
        else
          targetCallback->sense(variableNumber, value);
      }
    }

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {
      jassert(variableNumber < owner->variablesMap.size());
      variableNumber = owner->variablesMap[variableNumber];
      if (variableNumber >= 0)
      {
        jassert(variableNumber >= 0 && variableNumber < owner->outputVariables.size());
        const OutputVariable& outputVariable = owner->outputVariables[variableNumber];
        targetCallback->sense(variableNumber, outputVariable.subPerception ? outputVariable.subPerception : subPerception, input);
      }
    }

  private:
    PerceptionCallbackPtr targetCallback;
    const RewritedPerception* owner;
  };
};

typedef ReferenceCountedObjectPtr<RewritedPerception> RewritedPerceptionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_REWRITED_H_
