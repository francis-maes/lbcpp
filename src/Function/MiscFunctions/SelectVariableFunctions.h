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
# include <lbcpp/Data/Pair.h>

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
    {return index >= 0 ? inputType->getObjectVariableType((size_t)index) : inputType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    Variable res = input;
    if (index >= 0)
    {
      res = res.getObject()->getVariable(index);
      if (!res.exists())
        res = Variable::missingValue(input.getType()->getObjectVariableType((size_t)index));
    }
    return res;
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
  {
    outputType = getOutputType(inputPairClass);
  }
  SelectPairVariablesFunction() : index1(-1), index2(-1) {}

  virtual TypePtr getInputType() const
    {return inputPairClass;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return pairClass(
      getOutputTypeBase(inputType->getTemplateArgument(0), index1),
      getOutputTypeBase(inputType->getTemplateArgument(1), index2));}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    PairPtr pair = input.getObjectAndCast<Pair>();
    jassert(pair);
    Variable first = pair->getFirst();
    if (index1 >= 0)
      first = first.getObject()->getVariable(index1);
    Variable second = pair->getSecond();
    if (index2 >= 0)
      second = second.getObject()->getVariable(index2);
    return new Pair(outputType, first, second);
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!Function::loadFromXml(importer))
      return false;
    outputType = getOutputType(inputPairClass);
    return true;
  }

private:
  friend class SelectPairVariablesFunctionClass;

  int index1, index2;
  TypePtr inputPairClass;

  TypePtr outputType;

  static TypePtr getOutputTypeBase(TypePtr inputType, int index)
  {
    if (index >= 0)
    {
      jassert((size_t)index < inputType->getObjectNumVariables());
      return inputType->getObjectVariableType((size_t)index);
    }
    else
      return inputType;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
