/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Variable Functions              |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 11:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Function.h>
using namespace lbcpp;

class LoadFromXmlFunction : public Function
{
public:
  LoadFromXmlFunction(TypePtr expectedType = objectClass())
    : expectedType(expectedType) {}

  virtual TypePtr getInputType() const
    {return fileType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return expectedType;}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    File file(input.getString());
    Variable res = Variable::createFromFile(file, callback);
    checkInheritance(res, expectedType);
    return res;
  }

protected:
  TypePtr expectedType;
};

FunctionPtr lbcpp::loadFromFileFunction(TypePtr expectedType)
  {return new LoadFromXmlFunction(expectedType);}

// Input: (a,b)
// do: a[fieldIndex] = b if fieldIndex, nothing otherwise
// Output: a
class SetFieldFunction : public Function
{
public:
  SetFieldFunction(size_t fieldIndex = 0)
    : fieldIndex(fieldIndex) {}

  virtual TypePtr getInputType() const
    {return pairType(objectClass(), anyType());}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType->getTemplateArgument(0);}

protected:
  size_t fieldIndex;

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    ObjectPtr object = input[0].getObject();
    if (object && input[1])
      object->setVariable(fieldIndex, input[1]);
    else if (!object)
      callback.warningMessage(T("SetFieldFunction::computeFunction"), T("Null object"));
    return input[0];
  }
};

FunctionPtr lbcpp::setFieldFunction(size_t fieldIndex)
  {return new SetFieldFunction(fieldIndex);}

class SelectPairFieldsFunction : public Function
{
public:
  SelectPairFieldsFunction(int index1 = -1, int index2 = -1)
    : index1(index1), index2(index2)
    {}

  virtual TypePtr getInputType() const
    {return pairType(anyType(), anyType());}

  virtual TypePtr getOutputType(TypePtr inputType) const
  {
    return pairType(
      getOutputTypeBase(inputType->getTemplateArgument(0), index1),
      getOutputTypeBase(inputType->getTemplateArgument(1), index2));
  }

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    Variable first = input[0];
    if (index1 >= 0)
      first = first[index1];
    Variable second = input[1];
    if (index2 >= 0)
      second = second[index2];
    return Variable::pair(first, second);
  }

private:
  int index1, index2;

  static TypePtr getOutputTypeBase(TypePtr inputType, int index)
  {
    if (index >= 0)
    {
      if ((size_t)index < inputType->getNumStaticVariables())
        return inputType->getStaticVariableType((size_t)index);
      else
        return anyType();
    }
    else
      return inputType;
  }
};

FunctionPtr lbcpp::selectPairFieldsFunction(int index1, int index2)
  {return new SelectPairFieldsFunction(index1, index2);}

void declareFunctionClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Function, Object);
    LBCPP_DECLARE_CLASS(LoadFromXmlFunction, Function);
    LBCPP_DECLARE_CLASS(SetFieldFunction, Function);
    LBCPP_DECLARE_CLASS(SelectPairFieldsFunction, Function);
}
