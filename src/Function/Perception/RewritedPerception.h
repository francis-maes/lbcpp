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
  RewritedPerception(PerceptionPtr decorated, PerceptionRewriterPtr rewriter, std::vector<String>& stack)
    : DecoratorPerception(decorated), rewriter(rewriter) {computeOutputVariables(stack);}
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

  PerceptionRewriterPtr rewriter;

  struct OutputVariable
  {
    TypePtr type;
    String name;
    PerceptionPtr subPerception;
  };

  std::vector<OutputVariable> outputVariables;
  std::vector< std::pair<size_t, PerceptionPtr> > variablesMap; // decorated perception variable index -> perception
  
  void computeOutputVariables(std::vector<String>& stack)
  {
    jassert(rewriter);
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
        newPerception = rewriter->rewriteRecursively(variablePerception, stack);
      else
        newPerception = rewriter->applyRules(variableType, stack);

      if (newPerception)
      {
        TypePtr newVariableType;
        if (newPerception == identityPerception())
          newVariableType = variableType;
        else if (newPerception->getNumOutputVariables() > 0)
          newVariableType = newPerception->getOutputType();
        if (newVariableType)
          addOutputVariable(newVariableType, variableName, newPerception, i);
      }
      stack.pop_back();
    }
  }

  void addOutputVariable(TypePtr type, const String& name, PerceptionPtr subPerception, size_t sourceIndex)
  {
    size_t index = outputVariables.size();

    OutputVariable v;
    v.type = type;
    v.name = name;
    v.subPerception = subPerception;
    outputVariables.push_back(v);

    if (variablesMap.size() <= sourceIndex)
      variablesMap.resize(sourceIndex + 1, std::make_pair(0, PerceptionPtr()));
    variablesMap[sourceIndex] = std::make_pair(index, subPerception);
  }

  struct Callback : public PerceptionCallback
  {
    Callback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const RewritedPerception* owner)
      : targetCallback(targetCallback), owner(owner)
      {}

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
