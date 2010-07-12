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
    {return topLevelType();}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
    {File file(input.getString()); return Variable::createFromFile(file, callback);}
};

FunctionPtr lbcpp::loadFromFileFunction()
  {return new LoadFromXmlFunction();}

void declareFunctionClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Function, Object);
    LBCPP_DECLARE_CLASS(LoadFromXmlFunction, Function);
}
