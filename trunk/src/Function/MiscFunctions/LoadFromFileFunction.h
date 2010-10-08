/*-----------------------------------------.---------------------------------.
| Filename: LoadFromFileFunction.h         | Load From File Function         |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_LOAD_FROM_FILE_H_
# define LBCPP_DATA_FUNCTION_LOAD_FROM_FILE_H_

# include <lbcpp/Function/Function.h>

namespace lbcpp
{

// File -> Variable
class LoadFromFileFunction : public Function
{
public:
  LoadFromFileFunction(TypePtr expectedType = objectClass)
    : expectedType(expectedType) {}

  virtual TypePtr getInputType() const
    {return fileType;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return expectedType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    File file(input.getString());
    Variable res = Variable::createFromFile(file, callback);
    checkInheritance(res, expectedType);
    return res;
  }

protected:
  friend class LoadFromFileFunctionClass;

  TypePtr expectedType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_LOAD_FROM_FILE_H_
