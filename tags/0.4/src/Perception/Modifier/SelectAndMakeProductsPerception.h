/*-----------------------------------------.---------------------------------.
| Filename: SelectAndMakeProductsPerception.h| A decorator to make lots of   |
| Author  : Francis Maes                   | conjunction features            |
| Started : 05/10/2010 21:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_
# define LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class SelectAndMakeProductsPerception : public Perception
{
public:
  SelectAndMakeProductsPerception(PerceptionPtr decorated, FunctionPtr multiplyFunction, const std::vector< std::vector<size_t> >& selectedConjunctions)
    : decorated(decorated), multiplyFunction(multiplyFunction), selectedConjunctions(selectedConjunctions)
    {computeOutputType();}

  SelectAndMakeProductsPerception() {}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}
  
  virtual String toString() const
    {return decorated->toString() + T(" selected");}

  typedef std::vector< std::pair<PerceptionPtr, Variable> > PerceptionVariableVector;

  struct Callback : public PerceptionCallback
  {
    Callback(ExecutionContext& context, PerceptionVariableVector& variables)
      : PerceptionCallback(context), variables(variables) {}

    PerceptionVariableVector& variables;

    virtual void sense(size_t variableNumber, double value)
      {variables[variableNumber].second = value;}

    virtual void sense(size_t variableNumber, const ObjectPtr& value)
      {variables[variableNumber].second = value;}

    virtual void sense(size_t variableNumber, const Variable& value)
      {variables[variableNumber].second = value;}

    virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
      {variables[variableNumber].first = subPerception; variables[variableNumber].second = input;}
  };

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    // surface compute decorated perception
    PerceptionVariableVector variables(decorated->getNumOutputVariables());
    Callback callback(context, variables);
    decorated->computePerception(context, input, &callback);
  
    // sense
    size_t n = getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      PerceptionPtr subPerception = getOutputVariableSubPerception(i);
      const std::vector<size_t>& conjunction = selectedConjunctions[i];
      size_t arity = conjunction.size();
      if (arity == 1)
      {
        std::pair<PerceptionPtr, Variable> v = variables[conjunction[0]];
        if (v.second.exists())
        {
          jassert(subPerception == v.first);
          if (v.first)
            targetCallback->sense(i, v.first, v.second);
          else
            targetCallback->sense(i, v.second);
        }
      }
      else if (arity == 2)
      {
        std::pair<PerceptionPtr, Variable> v1 = variables[conjunction[0]];
        std::pair<PerceptionPtr, Variable> v2 = variables[conjunction[1]];
        if (v1.second.exists() && v2.second.exists())
        {
          jassert(v1.first && v2.first);
          targetCallback->sense(i, subPerception, Variable::pair(v1.second, v2.second));
        }
      }
      else
        jassert(false); // not supported yet
    }
  }

  PerceptionPtr getDecoratedPerception() const
    {return decorated;}

  FunctionPtr getMultiplyFunction() const
    {return multiplyFunction;}

  typedef std::vector<size_t> Conjunction;

  void clearConjunctions()
    {selectedConjunctions.clear(); clearOutputVariables();}

  void addConjunction(const Conjunction& conjunction)
    {selectedConjunctions.push_back(conjunction); clearOutputVariables();}

  void removeConjunctions(const std::set<size_t>& conjunctionsToRemove)
  {
    size_t oldIndex = 0;
    size_t newIndex = 0;
    clearOutputVariables();
    for (std::set<size_t>::const_iterator it = conjunctionsToRemove.begin(); it != conjunctionsToRemove.end(); ++it)
    {
      jassert(oldIndex <= *it);
      newIndex += *it - oldIndex;
      oldIndex = *it;
      selectedConjunctions.erase(selectedConjunctions.begin() + newIndex);
      //outputVariables.erase(outputVariables.begin() + newIndex);
      ++oldIndex;
    }
  }

  const std::vector<Conjunction>& getConjunctions() const
    {return selectedConjunctions;}

  size_t getNumConjunctions() const
    {return selectedConjunctions.size();}

  const Conjunction& getConjunction(size_t index) const
    {jassert(index < selectedConjunctions.size()); return selectedConjunctions[index];}

protected:
  friend class SelectAndMakeProductsPerceptionClass;

  PerceptionPtr decorated;
  FunctionPtr multiplyFunction;
  std::vector<Conjunction> selectedConjunctions; // outputNumber -> numberInConjunction -> variableNumber

  virtual void computeOutputType()
  {
    size_t n = selectedConjunctions.size();
    reserveOutputVariables(n);
    for (size_t i = 0; i < n; ++i)
      createSubPerception(selectedConjunctions[i]);
    Perception::computeOutputType();
  }

  void createSubPerception(const Conjunction& conjunction)
  {
    size_t arity = conjunction.size();
    jassert(arity);
    if (arity == 1)
    {
      size_t variableNumber = conjunction[0];
      jassert(variableNumber < decorated->getNumOutputVariables());
      String name = decorated->getOutputVariableName(variableNumber);
      
      PerceptionPtr subPerception = decorated->getOutputVariableSubPerception(variableNumber);
      addOutputVariable(decorated->getOutputVariableType(variableNumber), name, subPerception);
    }

    else if (arity == 2)
    {
      size_t index1 = conjunction[0];
      size_t index2 = conjunction[1];
      jassert(index1 <= (size_t)decorated->getNumOutputVariables());
      jassert(index2 <= (size_t)decorated->getNumOutputVariables());

      String name = decorated->getOutputVariableName(index1) + T("&&") + decorated->getOutputVariableName(index2);
      PerceptionPtr subPerception1 = decorated->getOutputVariableSubPerception(index1);
      PerceptionPtr subPerception2 = decorated->getOutputVariableSubPerception(index2);
      jassert(subPerception1 && subPerception2);
      PerceptionPtr subPerception = productPerception(multiplyFunction, subPerception1, subPerception2, true, false);
      addOutputVariable(subPerception->getOutputType(), name, subPerception);
    }
    else
    {
       // not supported yet
      jassert(false);
    }
  }
};

typedef ReferenceCountedObjectPtr<SelectAndMakeProductsPerception> SelectAndMakeProductsPerceptionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_