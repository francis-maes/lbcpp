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

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    Variable perceived = decorated->compute(input);
    for (size_t i = 0; i < paths.size(); ++i)
    {
      Variable variable = getVariableRecursively(perceived, paths[i]);
      if (variable && checkInheritance(variable, getOutputVariableType(i)))
        targetCallback->sense(i, variable);
    }
  }

  juce_UseDebuggingNewOperator

private:
  friend class CollapsePerceptionClass;

  PerceptionPtr decorated;
  std::vector< std::vector<size_t> > paths;

  bool isLeafPerception(PerceptionPtr perception) const
  {
    size_t n = perception->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
      if (perception->getOutputVariableSubPerception(i))
        return false;
    return true;
  }

  Variable getVariableRecursively(const Variable& variable, const std::vector<size_t>& path, size_t currentIndex = 0) const
  {
    if (currentIndex >= path.size() || !variable)    
      return variable;
    jassert(variable.isObject());
    ObjectPtr object = variable.getObject();
    return getVariableRecursively(object->getVariable(path[currentIndex]), path, currentIndex + 1);
  }

  void precompute(PerceptionPtr perception, const String& fullName, const std::vector<size_t>& stack = std::vector<size_t>())
  {
    if (isLeafPerception(perception))
    {
      addOutputVariable(perception->getOutputType(), fullName, PerceptionPtr());
      paths.push_back(stack);
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
