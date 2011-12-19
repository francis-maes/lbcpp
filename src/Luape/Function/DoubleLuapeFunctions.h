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

  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& in, TypePtr outputType) const
  {
    const DenseDoubleVectorPtr& inputs = in[0].staticCast<DenseDoubleVector>();

    DenseDoubleVectorPtr res = new DenseDoubleVector(vectorClass, inputs->getNumValues(), 0.0);
    const double* ptr = inputs->getValuePointer(0);
    const double* lim = ptr + inputs->getNumValues();
    double* target = res->getValuePointer(0);
    while (ptr != lim)
      *target++ = computeDouble(*ptr++);
    return res;
  }

protected:
  ClassPtr vectorClass;
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
};

class AddDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "+";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " + " + inputs[1]->toShortString() + ")";}

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

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " - " + inputs[1]->toShortString() + ")";}

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

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " x " + inputs[1]->toShortString() + ")";}

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

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " / " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return second ? first / second : doubleMissingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_DOUBLE_H_
