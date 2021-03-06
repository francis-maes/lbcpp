/*-----------------------------------------.---------------------------------.
| Filename: BooleanFunctions.h             | Boolean Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_FUNCTION_BOOLEAN_H_
# define ML_FUNCTION_BOOLEAN_H_

# include <ml/Function.h>
# include <ml/Expression.h>

namespace lbcpp
{

/*
** Unary function
*/
class NotBooleanFunction : public HomogeneousUnaryFunction
{
public:
  NotBooleanFunction()
    : HomogeneousUnaryFunction(booleanClass) {}
  
  virtual string toShortString() const
    {return "!";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "!" + inputs[0]->toShortString();}
  
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
    {return inputs[0] ? new Boolean(!Boolean::get(inputs[0])) : ObjectPtr();}

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
  {
    DataVector::const_iterator it = inputs[0]->begin();
    size_t n = inputs[0]->size();

    BVectorPtr res = new BVector(n);
    unsigned char* dest = res->getDataPointer();
    const unsigned char* lim = dest + n;
    while (dest < lim)
    {
      unsigned char b = it.getRawBoolean();
      *dest++ = (b == 2 ? 2 : 1 - b);
      ++it;
    }
    return new DataVector(inputs[0]->getIndices(), res);
  }
};

/*
** Binary functions
*/
class BinaryBooleanFunction : public HomogeneousBinaryFunction
{
public:
  BinaryBooleanFunction()
    : HomogeneousBinaryFunction(booleanClass) {}

  virtual bool computeBoolean(bool first, bool second) const = 0;
  
  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " " + toShortString() + " " + inputs[1]->toShortString() + ")";}

  virtual Flags getFlags() const
    {return (Flags)(commutativeFlag | allSameArgIrrelevantFlag);}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0] || !inputs[1])
      return ObjectPtr();
    return new Boolean(computeBoolean(Boolean::get(inputs[0]), Boolean::get(inputs[1])));
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
  {
    DataVector::const_iterator it1 = inputs[0]->begin();
    DataVector::const_iterator it2 = inputs[1]->begin();
    size_t n = inputs[0]->size();
    jassert(n == inputs[1]->size());

    BVectorPtr res = new BVector(n);
    unsigned char* dest = res->getDataPointer();
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
    return new DataVector(inputs[0]->getIndices(), res);
  }
};

class AndBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual string toShortString() const
    {return "&&";}

  virtual bool computeBoolean(bool first, bool second) const
    {return first && second;}
};

class OrBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual string toShortString() const
    {return "||";}

  virtual bool computeBoolean(bool first, bool second) const
    {return first || second;}
};

class NandBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual string toShortString() const
    {return "!&&";}

  virtual bool computeBoolean(bool first, bool second) const
    {return !(first && second);}
};

class NorBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual string toShortString() const
    {return "!||";}

  virtual bool computeBoolean(bool first, bool second) const
    {return !(first || second);}
};

class EqualBooleanFunction : public BinaryBooleanFunction
{
public:
  virtual string toShortString() const
    {return "==";}

  virtual bool computeBoolean(bool first, bool second) const
    {return first == second;}
};


/*
** Ternary function
*/
class IfThenElseBooleanFunction : public HomogeneousTernaryFunction
{
public:
  IfThenElseBooleanFunction() : HomogeneousTernaryFunction(booleanClass) {}
  
  virtual string toShortString() const
    {return "if-then-else";}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " ? " + inputs[1]->toShortString() + " : " + inputs[2]->toShortString() + ")";}
  
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    return Boolean::get(inputs[0]) ? inputs[1] : inputs[2];
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
  {
    DataVector::const_iterator it1 = inputs[0]->begin();
    DataVector::const_iterator it2 = inputs[1]->begin();
    DataVector::const_iterator it3 = inputs[2]->begin();
    size_t n = inputs[0]->size();
    jassert(n == inputs[1]->size() && n == inputs[2]->size());

    BVectorPtr res = new BVector(n);
    unsigned char* dest = res->getDataPointer();
    const unsigned char* lim = dest + n;
    while (dest < lim)
    {
      unsigned char b = it1.getRawBoolean();
      if (b == 2)
        *dest++ = 2;
      else
        *dest++ = (b == 1 ? it2.getRawBoolean() : it3.getRawBoolean());
      ++it1, ++it2, ++it3;
    }

    return new DataVector(inputs[0]->getIndices(), res);
  }
};

}; /* namespace lbcpp */

#endif // !ML_FUNCTION_BOOLEAN_H_
