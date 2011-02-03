/*-----------------------------------------.---------------------------------.
| Filename: SelectPairVariablesFunction.h  | Select Pair Fields Function     |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
# define LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

// a => a.x
class SelectVariableFunction : public Function
{
public:
  SelectVariableFunction(int index = -1)
    : index(index) {}

  virtual TypePtr getInputType() const
    {return objectClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return index >= 0 ? inputType->getMemberVariableType((size_t)index) : inputType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (index >= 0)
    {
      Variable res = input.getObject()->getVariable(index);
      if (!res.exists())
        res = Variable::missingValue(input.getType()->getMemberVariableType((size_t)index));
      return res;
    }
    else
      return input;
  }

private:
  friend class SelectVariableFunctionClass;

  int index;
};

// pair(a, b) => pair(a.x, b.y)
class SelectPairVariablesFunction : public Function
{
public:
  SelectPairVariablesFunction(int index1, int index2, TypePtr inputPairClass)
    : index1(index1), index2(index2), inputPairClass(inputPairClass)
    {computeOutputType();}
  SelectPairVariablesFunction() : index1(-1), index2(-1) {}

  virtual TypePtr getInputType() const
    {return inputPairClass;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return outputType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    jassert(pair);
    const Variable& first = pair->getFirst();
    const Variable& second = pair->getSecond();
    return new Pair(outputType, index1 >= 0 ? first.getObject()->getVariable(index1) : first,
                                index2 >= 0 ? second.getObject()->getVariable(index2) : second);
  }

  virtual bool loadFromXml(XmlImporter& importer)
    {return Function::loadFromXml(importer) && computeOutputType();}

private:
  friend class SelectPairVariablesFunctionClass;

  int index1, index2;
  TypePtr inputPairClass;

  TypePtr outputType;

  bool computeOutputType()
  {
    outputType = pairClass(
      getOutputTypeBase(inputPairClass->getTemplateArgument(0), index1),
      getOutputTypeBase(inputPairClass->getTemplateArgument(1), index2));
    return true;
  }

  static TypePtr getOutputTypeBase(TypePtr inputType, int index)
  {
    if (index >= 0)
    {
      jassert((size_t)index < inputType->getNumMemberVariables());
      return inputType->getMemberVariableType((size_t)index);
    }
    else
      return inputType;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
