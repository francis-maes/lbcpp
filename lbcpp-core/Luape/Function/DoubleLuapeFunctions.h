/*-----------------------------------------.---------------------------------.
| Filename: DoubleLuapeFunctions.h         | Double Luape Functions          |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_DOUBLE_H_
# define LBCPP_LUAPE_FUNCTION_DOUBLE_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

class UnaryDoubleLuapeFuntion : public HomogeneousUnaryLuapeFunction
{
public:
  UnaryDoubleLuapeFuntion()
    : HomogeneousUnaryLuapeFunction(doubleType), vectorClass(simpleDenseDoubleVectorClass) {}

  virtual double computeDouble(double value) const = 0;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    double value = inputs[0].getDouble();
    return (value == doubleMissingValue ? value : computeDouble(value));
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

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return toShortString() + "(" + inputs[0]->toShortString() + ")";}

protected:
  ClassPtr vectorClass;
};

class OppositeDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "-";}

  virtual double computeDouble(double value) const
    {return -value;}
};

class InverseDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "inverse";}

  virtual double computeDouble(double value) const
    {return value ? 1.0 / value : doubleMissingValue;}
};

class AbsDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "abs";}

  virtual double computeDouble(double value) const
    {return fabs(value);}
};

class LogDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "log";}

  virtual double computeDouble(double value) const
    {return value <= 0.0 ? doubleMissingValue : log(value);}
};

class ExpDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "exp";}

  virtual double computeDouble(double value) const
    {return value >= 100 ? doubleMissingValue : exp(value);}
};

class SqrtDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "sqrt";}

  virtual double computeDouble(double value) const
    {return value < 0.0 ? doubleMissingValue : sqrt(value);}
};

class CosDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "cos";}

  virtual double computeDouble(double value) const
    {return cos(value);}
};

class SinDoubleLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  virtual String toShortString() const
    {return "sin";}

  virtual double computeDouble(double value) const
    {return sin(value);}
};

class BinaryDoubleLuapeFunction : public HomogeneousBinaryLuapeFunction
{
public:
  BinaryDoubleLuapeFunction()
    : HomogeneousBinaryLuapeFunction(doubleType) {}

  virtual double computeDouble(double first, double second) const = 0;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    double v1 = inputs[0].getDouble();
    double v2 = inputs[1].getDouble();
    return v1 == doubleMissingValue || v2 == doubleMissingValue ? doubleMissingValue : computeDouble(v1, v2);
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

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " " + toShortString() + " " + inputs[1]->toShortString() + ")";}
};

class AddDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "+";}

  virtual double computeDouble(double first, double second) const
    {return first + second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class SubDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "-";}

  virtual double computeDouble(double first, double second) const
    {return first - second;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class MulDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "*";}

  virtual double computeDouble(double first, double second) const
    {return first * second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class DivDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "/";}

  virtual double computeDouble(double first, double second) const
    {return second ? first / second : doubleMissingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class PowDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "^";}

  virtual double computeDouble(double first, double second) const
    {return first || second ? std::pow(first, second) : doubleMissingValue;}
};

class MinDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "min";}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return "min(" + inputs[0]->toShortString() + ", " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return juce::jmin(first, second);}

  virtual Flags getFlags() const
    {return (Flags)(allSameArgIrrelevantFlag | commutativeFlag);}
};

class MaxDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "max";}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return "max(" + inputs[0]->toShortString() + ", " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return juce::jmax(first, second);}

  virtual Flags getFlags() const
    {return (Flags)(allSameArgIrrelevantFlag | commutativeFlag);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_DOUBLE_H_
