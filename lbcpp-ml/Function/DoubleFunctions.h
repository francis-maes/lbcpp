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

namespace lbcpp
{

class UnaryDoubleFunction : public HomogeneousUnaryFunction
{
public:
  UnaryDoubleFunction()
    : HomogeneousUnaryFunction(doubleClass) {}

  virtual double computeDouble(double value) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    double res = computeDouble(Double::get(inputs[0]));
    jassert(res == doubleMissingValue || isNumberValid(res));
    return res == doubleMissingValue ? ObjectPtr() : ObjectPtr(new Double(res));
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& in, ClassPtr outputType) const
  {
    const DataVectorPtr& inputs = in[0];
    DVectorPtr res = new DVector(outputType, inputs->size(), 0.0);
    double* dest = res->getDataPointer();
    for (DataVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it)
    {
      double value = it.getRawDouble();
      *dest++ = value == doubleMissingValue ? doubleMissingValue : computeDouble(value);
    }
    return new DataVector(inputs->getIndices(), res);
  }

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return toShortString() + "(" + inputs[0]->toShortString() + ")";}
};

class OppositeDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "-";}

  virtual double computeDouble(double value) const
    {return -value;}
};

class InverseDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "inverse";}

  virtual double computeDouble(double value) const
    {return value ? 1.0 / value : doubleMissingValue;}
};

class AbsDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "abs";}

  virtual double computeDouble(double value) const
    {return fabs(value);}
};

class LogDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "log";}

  virtual double computeDouble(double value) const
    {return value <= 0.0 ? doubleMissingValue : log(value);}
};

class ProtectedLogDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "plog";}

  virtual double computeDouble(double value) const
    {return value > 0.000001 ? log(fabs(value)) : 1.0;}
};

class ExpDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "exp";}

  virtual double computeDouble(double value) const
    {return value >= 100 ? doubleMissingValue : exp(value);}
};

class SqrtDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "sqrt";}

  virtual double computeDouble(double value) const
    {return value < 0.0 ? doubleMissingValue : sqrt(value);}
};

class CosDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "cos";}

  virtual double computeDouble(double value) const
    {return cos(value);}
};

class SinDoubleFunction : public UnaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "sin";}

  virtual double computeDouble(double value) const
    {return sin(value);}
};

class BinaryDoubleFunction : public HomogeneousBinaryFunction
{
public:
  BinaryDoubleFunction()
    : HomogeneousBinaryFunction(doubleClass) {}

  virtual double computeDouble(double first, double second) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0] || !inputs[1])
      return ObjectPtr();
    double res = computeDouble(Double::get(inputs[0]), Double::get(inputs[1]));
    if (res == doubleMissingValue)
      return ObjectPtr();
    jassert(isNumberValid(res));
    return new Double(res);
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
  {
    DataVector::const_iterator it1 = inputs[0]->begin();
    DataVector::const_iterator it2 = inputs[1]->begin();
    size_t n = inputs[0]->size();
    jassert(n == inputs[1]->size());

    DVectorPtr res = new DVector(outputType, n, 0.0);
    double* dest = res->getDataPointer();
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
    return new DataVector(inputs[0]->getIndices(), res);
  }

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " " + toShortString() + " " + inputs[1]->toShortString() + ")";}
};

class AddDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "+";}

  virtual double computeDouble(double first, double second) const
    {return first + second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class SubDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "-";}

  virtual double computeDouble(double first, double second) const
    {return first - second;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class MulDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "*";}

  virtual double computeDouble(double first, double second) const
    {return first * second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class DivDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "/";}

  virtual double computeDouble(double first, double second) const
    {return second ? first / second : doubleMissingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class ProtectedDivDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "p/";}

  virtual double computeDouble(double first, double second) const
    {return fabs(second) > 0.001 ? first / second : 1.0;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class PowDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "^";}

  virtual double computeDouble(double first, double second) const
    {return first || second ? std::pow(first, second) : doubleMissingValue;}
};

class MinDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "min";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "min(" + inputs[0]->toShortString() + ", " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return juce::jmin(first, second);}

  virtual Flags getFlags() const
    {return (Flags)(allSameArgIrrelevantFlag | commutativeFlag);}
};

class MaxDoubleFunction : public BinaryDoubleFunction
{
public:
  virtual string toShortString() const
    {return "max";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "max(" + inputs[0]->toShortString() + ", " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return juce::jmax(first, second);}

  virtual Flags getFlags() const
    {return (Flags)(allSameArgIrrelevantFlag | commutativeFlag);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_DOUBLE_H_
