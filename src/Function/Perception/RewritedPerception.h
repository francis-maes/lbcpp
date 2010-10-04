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
# include <lbcpp/Data/XmlSerialisation.h>

namespace lbcpp
{

class RewritedPerception : public VariableVectorPerception
{
public:
  RewritedPerception(PerceptionPtr decorated, PerceptionRewriterPtr rewriter, std::vector<String>& stack)
    : decorated(decorated) {computeOutputVariables(rewriter, stack);}
  RewritedPerception() {}

  virtual String getPreferedOutputClassName() const
    {return decorated->getPreferedOutputClassName() + T(" rewrited");}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    PerceptionCallbackPtr decoratedVisitor(new Callback(decorated, callback, this));
    decorated->computePerception(input, decoratedVisitor);
  }

  virtual void saveToXml(XmlExporter& exporter)
  {
    Perception::saveToXml(exporter);

    exporter.enter(T("outputvar"));
    exporter.setAttribute(T("count"), (int)outputVariables.size());
    for (size_t i = 0; i < outputVariables.size(); ++i)
    {
      const OutputVariable& var = outputVariables[i];
      exporter.enter(T("var"));
      exporter.writeType(var.type);
      exporter.setAttribute(T("name"), var.name);
      exporter.saveVariable(T("subPerception"), var.subPerception);
      exporter.leave();
    }
    exporter.leave();

    exporter.enter(T("mapping"));
    exporter.setAttribute(T("size"), (int)variablesMap.size());
    for (size_t i = 0; i < variablesMap.size(); ++i)
      exporter.saveElement(i, Variable::pair(variablesMap[i].first, variablesMap[i].second));
    exporter.leave();
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!Perception::loadFromXml(importer))
      return false;

    jassert(false); // TODO: load outputVariables and variablesMap
    return true;
  }

  juce_UseDebuggingNewOperator

protected:
  friend class PerceptionRewriteRules;
  friend class RewritedPerceptionClass;

  PerceptionPtr decorated;
  std::vector< std::pair<size_t, PerceptionPtr> > variablesMap; // decorated perception variable index -> perception

  void computeOutputVariables(PerceptionRewriterPtr rewriter, std::vector<String>& stack)
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
    VariableVectorPerception::addOutputVariable(type, name, subPerception);
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
