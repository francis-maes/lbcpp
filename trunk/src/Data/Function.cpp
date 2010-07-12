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
  virtual TypePtr getInputType() const
    {return fileType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return objectClass();}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
    {File file(input.getString()); return Variable::createFromFile(file, callback);}
};

FunctionPtr lbcpp::loadFromFileFunction()
  {return new LoadFromXmlFunction();}

// Input: (a,b)
// do: a[fieldIndex] = b
// Output: a
class SetFieldFunction : public Function
{
public:
  SetFieldFunction(size_t fieldIndex)
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
    if (object)
      object->setVariable(fieldIndex, input[1]);
    else
      callback.warningMessage(T("SetFieldFunction::computeFunction"), T("Null object"));
    return input[0];
  }
};

FunctionPtr lbcpp::setFieldFunction(size_t fieldIndex)
  {return new SetFieldFunction(fieldIndex);}

void declareFunctionClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Function, Object);
    LBCPP_DECLARE_CLASS(LoadFromXmlFunction, Function);
}
