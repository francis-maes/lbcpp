/*-----------------------------------------.---------------------------------.
| Filename: LoadFromFileFunction.h         | Load From File Function         |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_LOAD_FROM_FILE_H_
# define LBCPP_DATA_FUNCTION_LOAD_FROM_FILE_H_

# include <lbcpp/Data/Pair.h>
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
class LoadFromFilePairFunction : public Function
{
public:
  LoadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass)
    : expectedType(pairClass(expectedType1, expectedType2)) {}

  virtual TypePtr getInputType() const
    {return pairClass(fileType, fileType);}

  virtual TypePtr getOutputType(TypePtr ) const
    {return expectedType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    File file1 = pair->getFirst().getFile();
    File file2 = pair->getSecond().getFile();

    Variable res1 = Variable::createFromFile(context, file1);
    Variable res2 = Variable::createFromFile(context, file2);
    if (!res1.exists() || !res2.exists())
      return Variable::missingValue(expectedType);
    Variable res = Variable::pair(res1, res2);
    context.checkInheritance(res, expectedType);
    return res;
  }

protected:
  friend class LoadFromFilePairFunctionClass;

  TypePtr expectedType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_LOAD_FROM_FILE_H_
