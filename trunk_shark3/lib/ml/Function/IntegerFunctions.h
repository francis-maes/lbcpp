/*-----------------------------------------.---------------------------------.
| Filename: IntegerFunctions.h             | Integer Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_FUNCTION_INTEGER_H_
# define ML_FUNCTION_INTEGER_H_

# include <ml/Function.h>
# include <ml/Expression.h>

namespace lbcpp
{

class BinaryIntegerFunction : public HomogeneousBinaryFunction
{
public:
  BinaryIntegerFunction()
    : HomogeneousBinaryFunction(integerClass) {}

  virtual bool doAcceptInputType(size_t index, const ClassPtr& type) const
    {return type->inheritsFrom(integerClass) && !type.isInstanceOf<Enumeration>();} // exclude enumerations

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0] || !inputs[1])
      return ObjectPtr();
    return new Integer(computeInteger(Integer::get(inputs[0]), Integer::get(inputs[1])));
  }
};

class AddIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual string toShortString() const
    {return "+";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " + " + inputs[1]->toShortString() + ")";}

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
    {return first + second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class SubIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual string toShortString() const
    {return "-";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " - " + inputs[1]->toShortString() + ")";}

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
    {return first - second;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class MulIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual string toShortString() const
    {return "*";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " x " + inputs[1]->toShortString() + ")";}

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
    {return first * second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class DivIntegerFunction : public BinaryIntegerFunction
{
public:
  virtual string toShortString() const
    {return "/";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " / " + inputs[1]->toShortString() + ")";}

  virtual juce::int64 computeInteger(juce::int64 first, juce::int64 second) const
    {return second ? first / second : IVector::missingValue;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

}; /* namespace lbcpp */

#endif // !ML_FUNCTION_INTEGER_H_
