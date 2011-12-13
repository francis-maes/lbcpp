/*-----------------------------------------.---------------------------------.
| Filename: BooleanLuapeFunctions.h        | Boolean Luape Functions         |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_BOOLEAN_H_
# define LBCPP_LUAPE_FUNCTION_BOOLEAN_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>

namespace lbcpp
{

class BinaryBooleanLuapeFunction : public HomogeneousBinaryLuapeFunction
{
public:
  BinaryBooleanLuapeFunction()
    : HomogeneousBinaryLuapeFunction(booleanType) {}

  virtual bool computeBoolean(bool first, bool second) const = 0;

  virtual Flags getFlags() const
    {return (Flags)(commutativeFlag | allSameArgIrrelevantFlag);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    if (inputs[0].isMissingValue() || inputs[1].isMissingValue())
      return Variable::missingValue(booleanType);
    return computeBoolean(inputs[0].getBoolean(), inputs[1].getBoolean());
  }

  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
  {
    const BooleanVectorPtr& inputs1 = inputs[0].staticCast<BooleanVector>();
    const BooleanVectorPtr& inputs2 = inputs[1].staticCast<BooleanVector>();
    jassert(inputs1->getNumElements() == inputs2->getNumElements());
    size_t n = inputs1->getNumElements();
    
    BooleanVectorPtr res = new BooleanVector(n);
    
    const unsigned char* ptr1 = inputs1->getData();
    const unsigned char* ptr2 = inputs2->getData();
    unsigned char* dest = res->getData();
    const unsigned char* lim = ptr1 + n;
    while (ptr1 < lim)
    {
      if (*ptr1 < 2 && *ptr2 < 2)
        *dest = computeBoolean(*ptr1 == 1, *ptr2 == 1);
      ptr1++;
      ptr2++;
      dest++;
    }
    return res;
  }
};

class AndBooleanLuapeFunction : public BinaryBooleanLuapeFunction
{
public:
  virtual String toShortString() const
    {return "&&";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + " && " + inputs[1]->toShortString();}

  virtual bool computeBoolean(bool first, bool second) const
    {return first && second;}
};

class EqualBooleanLuapeFunction : public BinaryBooleanLuapeFunction
{
public:
  virtual String toShortString() const
    {return "==";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + " == " + inputs[1]->toShortString();}

  virtual bool computeBoolean(bool first, bool second) const
    {return first == second;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_BOOLEAN_H_
