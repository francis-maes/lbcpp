/*-----------------------------------------.---------------------------------.
| Filename: BooleanFunctions.h             | Boolean Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_BOOLEAN_H_
# define LBCPP_ML_FUNCTION_BOOLEAN_H_

# include <lbcpp-ml/Function.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

class BinaryBooleanFunction : public HomogeneousBinaryFunction
{
public:
  BinaryBooleanFunction()
    : HomogeneousBinaryFunction(booleanType) {}

  virtual bool computeBoolean(bool first, bool second) const = 0;

  virtual Flags getFlags() const
    {return (Flags)(commutativeFlag | allSameArgIrrelevantFlag);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    if (inputs[0].isMissingValue() || inputs[1].isMissingValue())
      return Variable::missingValue(booleanType);
    return computeBoolean(inputs[0].getBoolean(), inputs[1].getBoolean());
  }

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    LuapeSampleVector::const_iterator it1 = inputs[0]->begin();
    LuapeSampleVector::const_iterator it2 = inputs[1]->begin();
    size_t n = inputs[0]->size();
    jassert(n == inputs[1]->size());

    BooleanVectorPtr res = new BooleanVector(n);
    unsigned char* dest = res->getData();
    const unsigned char* lim = dest + n;
    while (dest < lim)
    {
      unsigned char b1 = it1.getRawBoolean();
      unsigned char b2 = it2.getRawBoolean();
      if (b1 == 2 || b2 == 2)
        *dest++ = 2;
      else
        *dest++ = computeBoolean(b1 == 1, b2 == 1);
      ++it1;
      ++it2;
    }
    return new LuapeSampleVector(inputs[0]->getIndices(), res);
  }
};

class AndBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual String toShortString() const
    {return "&&";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return inputs[0]->toShortString() + " && " + inputs[1]->toShortString();}

  virtual bool computeBoolean(bool first, bool second) const
    {return first && second;}
};

class EqualBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual String toShortString() const
    {return "==";}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return inputs[0]->toShortString() + " == " + inputs[1]->toShortString();}

  virtual bool computeBoolean(bool first, bool second) const
    {return first == second;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_BOOLEAN_H_
