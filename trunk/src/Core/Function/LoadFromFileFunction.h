/*-----------------------------------------.---------------------------------.
| Filename: LoadFromFileFunction.h         | Load From File Function         |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_LOAD_FROM_FILE_H_
# define LBCPP_CORE_FUNCTION_LOAD_FROM_FILE_H_

# include <lbcpp/Core/Pair.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

// File -> Variable
class LoadFromFileFunction : public SimpleUnaryFunction
{
public:
  LoadFromFileFunction(TypePtr expectedType = objectClass)
    : SimpleUnaryFunction(fileType, expectedType, T("Loaded")), expectedType(expectedType) {}

  virtual String toString() const
    {return T("Load ") + expectedType->getName() + T(" From File");}

  virtual String getDescription(const Variable& input) const
    {return T("Load ") + input.toShortString();}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    File file = input.getFile();
    Variable res = Variable::createFromFile(context, file);
    context.checkInheritance(res, expectedType);
    return res;
  }

protected:
  friend class LoadFromFileFunctionClass;

  TypePtr expectedType;
};

// Pair<File,File> -> Pair<Variable,Variable>
class LoadFromFilePairFunction : public SimpleBinaryFunction
{
public:
  LoadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass)
    : SimpleBinaryFunction(fileType, fileType, pairClass(expectedType1, expectedType2), T("Loaded")), expectedType1(expectedType1), expectedType2(expectedType2) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    File file1 = inputs[0].getFile();
    File file2 = inputs[1].getFile();

    Variable res1 = Variable::createFromFile(context, file1);
    Variable res2 = Variable::createFromFile(context, file2);
    if (!res1.exists() || !res2.exists())
      return Variable::missingValue(getOutputType());
    return new Pair(getOutputType(), res1, res2);
  }

protected:
  friend class LoadFromFilePairFunctionClass;

  TypePtr expectedType1;
  TypePtr expectedType2;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_LOAD_FROM_FILE_H_
