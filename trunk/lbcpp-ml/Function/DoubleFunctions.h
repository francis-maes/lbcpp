/*-----------------------------------------.---------------------------------.
| Filename: DoubleFunctions.h              | Double Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_DOUBLE_H_
# define LBCPP_ML_FUNCTION_DOUBLE_H_

# include <lbcpp-ml/Function.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

class UnaryDoubleLuapeFuntion : public HomogeneousUnaryFunction
{
public:
  UnaryDoubleLuapeFuntion()
    : HomogeneousUnaryFunction(doubleType), vectorClass(simpleDenseDoubleVectorClass) {}

  virtual double computeDouble(double value) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    double res = computeDouble(NewDouble::get(inputs[0]));
    jassert(res == doubleMissingValue || isNumberValid(res));
    return res == doubleMissingValue ? ObjectPtr() : ObjectPtr(new NewDouble(res));
  }

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& in, TypePtr outputType) const
  {
    const LuapeSampleVectorPtr& inputs = in[0];
    DenseDoubleVectorPtr res = new DenseDoubleVector(vectorClass, inputs->size(), 0.0);
    double* dest = res->getValuePointer(0);
    for (LuapeSampleVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it)
    {
      double value = it.getRawDouble();
      *dest++ = value == doubleMissingValue ? doubleMissingValue : computeDouble(value);
    }
    return new LuapeSampleVector(inputs->getIndices(), res);
  }

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return toShortString() + "(" + inputs[0]->toShortString() + ")";}

protected:
  ClassPtr vectorClass;
};

class OppositeDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "-";}

  virtual double computeDouble(double value) const
    {return -value;}
};

class InverseDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "inverse";}

  virtual double computeDouble(double value) const
    {return value ? 1.0 / value : doubleMissingValue;}
};

class AbsDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "abs";}

  virtual double computeDouble(double value) const
    {return fabs(value);}
};

class LogDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "log";}

  virtual double computeDouble(double value) const
    {return value <= 0.0 ? doubleMissingValue : log(value);}
};

class ProtectedLogDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "plog";}

  virtual double computeDouble(double value) const
    {return value > 0.000001 ? log(fabs(value)) : 1.0;}
};

class ExpDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "exp";}

  virtual double computeDouble(double value) const
    {return value >= 100 ? doubleMissingValue : exp(value);}
};

class SqrtDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "sqrt";}

  virtual double computeDouble(double value) const
    {return value < 0.0 ? doubleMissingValue : sqrt(value);}
};

class CosDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "cos";}

  virtual double computeDouble(double value) const
    {return cos(value);}
};

class SinDoubleFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "sin";}

  virtual double computeDouble(double value) const
    {return sin(value);}
};

class BinaryDoubleFunction : public HomogeneousBinaryFunction
{
public:
  BinaryDoubleFunction()
    : HomogeneousBinaryFunction(doubleType) {}

  virtual double computeDouble(double first, double second) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0] || !inputs[1])
      return ObjectPtr();
    double res = computeDouble(NewDouble::get(inputs[0]), NewDouble::get(inputs[1]));
    if (res == doubleMissingValue)
      return ObjectPtr();
    jassert(isNumberValid(res));
    return new NewDouble(res);
  }

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    LuapeSampleVector::const_iterator it1 = inputs[0]->begin();
    LuapeSampleVector::const_iterator it2 = inputs[1]->begin();
    size_t n = inputs[0]->size();
    jassert(n == inputs[1]->size());

    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
    double* dest = res->getValuePointer(0);
    double* lim = dest + n;
    while (dest < lim)
    {
      double d1 = it1.getRawDouble();
      double d2 = it2.getRawDouble();
      if (d1 == doubleMissingValue || d2 == doubleMissingValue)
        *dest++ = doubleMissingValue;
      else
        *dest++ = computeDouble(d1, d2);
      ++it1;
      ++it2;
    }
    return new LuapeSampleVector(inputs[0]->getIndices(), res);
  }

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " " + toShortString() + " " + inputs[1]->toShortString() + ")";}
};

class AddDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "+";}

  virtual double computeDouble(double first, double second) const
    {return first + second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class SubDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "-";}

  virtual double computeDouble(double first, double second) const
    {return first - second;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class MulDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "*";}

  virtual double computeDouble(double first, double second) const
    {return first * second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class DivDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "/";}

  virtual double computeDouble(double first, double second) const
    {return second ? first / second : doubleMissingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class ProtectedDivDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "p/";}

  virtual double computeDouble(double first, double second) const
    {return fabs(second) > 0.001 ? first / second : 1.0;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class PowDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "^";}

  virtual double computeDouble(double first, double second) const
    {return first || second ? std::pow(first, second) : doubleMissingValue;}
};

class MinDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "min";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "min(" + inputs[0]->toShortString() + ", " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return juce::jmin(first, second);}

  virtual Flags getFlags() const
    {return (Flags)(allSameArgIrrelevantFlag | commutativeFlag);}
};

class MaxDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual String toShortString() const
    {return "max";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "max(" + inputs[0]->toShortString() + ", " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return juce::jmax(first, second);}

  virtual Flags getFlags() const
    {return (Flags)(allSameArgIrrelevantFlag | commutativeFlag);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_DOUBLE_H_
