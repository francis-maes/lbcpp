/*-----------------------------------------.---------------------------------.
| Filename: IntegerFunctions.h             | Integer Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_INTEGER_H_
# define LBCPP_ML_FUNCTION_INTEGER_H_

# include <lbcpp-ml/Function.h>
# include <lbcpp-ml/Expression.h>

namespace lbcpp
{

class BinaryIntegerFunction : public HomogeneousBinaryFunction
{
public:
  BinaryIntegerFunction()
    : HomogeneousBinaryFunction(integerType) {}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(integerType) && !type.isInstanceOf<Enumeration>();} // exclude enumerations

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0] || !inputs[1])
      return ObjectPtr();
    return new NewInteger(computeInteger(NewInteger::get(inputs[0]), NewInteger::get(inputs[1])));
  }
};

class AddIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual String toShortString() const
    {return "+";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " + " + inputs[1]->toShortString() + ")";}

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
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

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
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

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
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

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
    {return second ? first / second : integerMissingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_INTEGER_H_
