/*-----------------------------------------.---------------------------------.
| Filename: IntegerFunctions.h             | Integer Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_INTEGER_H_
# define LBCPP_ML_FUNCTION_INTEGER_H_

# include <lbcpp/Luape/Function.h>
# include <lbcpp/Luape/Expression.h>

namespace lbcpp
{

class BinaryIntegerFunction : public HomogeneousBinaryFunction
{
public:
  BinaryIntegerFunction()
    : HomogeneousBinaryFunction(integerType) {}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(integerType) && !type.isInstanceOf<Enumeration>();} // exclude enumerations

  virtual int computeInteger(int first, int second) const = 0;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    int v1 = inputs[0].getInteger();
    int v2 = inputs[1].getInteger();
    return v1 == integerMissingValue || v2 == integerMissingValue ? integerMissingValue : computeInteger(v1, v2);
  }
};

class AddIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual String toShortString() const
    {return "+";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " + " + inputs[1]->toShortString() + ")";}

  virtual int computeInteger(int first, int second) const
    {return first + second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class SubIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual String toShortString() const
    {return "-";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " - " + inputs[1]->toShortString() + ")";}

  virtual int computeInteger(int first, int second) const
    {return first - second;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class MulIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual String toShortString() const
    {return "*";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " x " + inputs[1]->toShortString() + ")";}

  virtual int computeInteger(int first, int second) const
    {return first * second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class DivIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual String toShortString() const
    {return "/";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " / " + inputs[1]->toShortString() + ")";}

  virtual int computeInteger(int first, int second) const
    {return second ? first / second : integerMissingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_INTEGER_H_
