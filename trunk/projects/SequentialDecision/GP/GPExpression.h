/*-----------------------------------------.---------------------------------.
| Filename: GPExpression.h                 | Genetic Programming Expression  |
| Author  : Francis Maes                   |                                 |
| Started : 23/05/2011 16:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GENETIC_PROGRAMMING_EXPRESSION_H_
# define LBCPP_GENETIC_PROGRAMMING_EXPRESSION_H_

# include <lbcpp/Core/Object.h>

namespace lbcpp
{

enum GPOperator
{
  gpAddition,
  gpSubtraction,
  gpMultiplication,
  gpDivision,
};

extern EnumerationPtr gpOperatorEnumeration;

enum GPPre
{
//  gpExp,
  gpLog,
  gpSquareRoot,
  gpInverse
};

extern EnumerationPtr gpPreEnumeration;
extern EnumerationPtr gpConstantEnumeration;

class GPExpression : public Object
{
public:
  virtual double compute(const double* x) const = 0;

  virtual size_t size() const = 0;
};

typedef ReferenceCountedObjectPtr<GPExpression> GPExpressionPtr;
extern ClassPtr gpExpressionClass;

extern ClassPtr variableGPExpressionClass;
extern ClassPtr unaryGPExpressionClass;
extern ClassPtr binaryGPExpressionClass;
extern ClassPtr constantGPExpressionClass;

class BinaryGPExpression;
typedef ReferenceCountedObjectPtr<BinaryGPExpression> BinaryGPExpressionPtr;

class UnaryGPExpression;
typedef ReferenceCountedObjectPtr<UnaryGPExpression> UnaryGPExpressionPtr;



class UnaryGPExpression : public GPExpression
{
public:
  UnaryGPExpression(GPPre pre, GPExpressionPtr expr)
    : pre(pre), expr(expr) {}
  UnaryGPExpression() {}

  virtual size_t size() const
    {return 1 + expr->size();}

  virtual double compute(const double* x) const
  {
    double e = expr->compute(x);
    switch (pre)
    {
    //case gpSin: return sin(e);
    //case gpCos: return cos(e);
    //case gpExp: return exp(e);
    case gpLog: return e <= 0.0 || !isNumberValid(e) ? -DBL_MAX : log(e);
    case gpSquareRoot: return isNumberValid(e) ? sqrt(e) : e;
    //case gpSquare: return e * e;
    case gpInverse: return isNumberValid(e) ? (e != 0.0 ? 1.0 / e : DBL_MAX) : e;
    default: jassert(false); return 0.0;
    };
  }

  virtual String toShortString() const
    {return Variable(pre, gpPreEnumeration).toShortString() + T("(") + (expr ? expr->toShortString() : String("<null>")) + T(")");}

  GPPre getOperator() const
    {return pre;}

  GPExpressionPtr& getExpression()
    {return expr;}

  const GPExpressionPtr& getExpression() const
    {return expr;}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const UnaryGPExpressionPtr& other = otherObject.staticCast<UnaryGPExpression>();
    if (pre != other->pre)
      return (int)pre - (int)other->pre;
    else
      return Variable(expr).compare(other->expr);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& o) const
  {
    const UnaryGPExpressionPtr& other = o.staticCast<UnaryGPExpression>();
    other->pre = pre;
    other->expr = expr->cloneAndCast<GPExpression>(context);
  }
 
protected:
  friend class UnaryGPExpressionClass;

  GPPre pre;
  GPExpressionPtr expr;
};

class BinaryGPExpression : public GPExpression
{
public:
  BinaryGPExpression(GPExpressionPtr left, GPOperator op, GPExpressionPtr right)
    : left(left), op(op), right(right) {}
  BinaryGPExpression() {}

  virtual size_t size() const
    {return left->size() + 1 + right->size();}

  virtual double compute(const double* x) const
  {
    double l = left->compute(x);
    double r = right->compute(x);
    if (!isNumberValid(l) || !isNumberValid(r))
      return l;

    switch (op)
    {
    case gpAddition: return l + r;
    case gpSubtraction: return l - r;
    case gpMultiplication: return l * r;
    case gpDivision: return r ? l / r : DBL_MAX;
    default: jassert(false); return 0.0;
    }
  }

  virtual String toShortString() const
  {
    const char* names[] = {"+", "-", "*", "/"};
    return T("(") + (left ? left->toShortString() : String("<null>")) + T(")") +
            names[op] +
           T("(") + (right ? right->toShortString() : String("<null>")) + T(")");
  }

  GPOperator getOperator() const
    {return op;}

  GPExpressionPtr& getLeft()
    {return left;}

  const GPExpressionPtr& getLeft() const
    {return left;}

  GPExpressionPtr& getRight()
    {return right;}

  const GPExpressionPtr& getRight() const
    {return right;}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const BinaryGPExpressionPtr& other = otherObject.staticCast<BinaryGPExpression>();
    if (op != other->op)
      return (int)op - (int)other->op;
    else
    {
      int res = Variable(left).compare(other->left);
      if (res != 0)
        return res;
      else
        return Variable(right).compare(other->right);
    }
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& other) const
  {
    const BinaryGPExpressionPtr& o = other.staticCast<BinaryGPExpression>();
    o->left = left->cloneAndCast<GPExpression>(context);
    o->op = op;
    o->right = right->cloneAndCast<GPExpression>(context);
  }

protected:
  friend class BinaryGPExpressionClass;

  GPExpressionPtr left;
  GPOperator op;
  GPExpressionPtr right;
};

class VariableGPExpression : public GPExpression
{
public:
  VariableGPExpression(size_t index = 0)
    : index(index) {}
  VariableGPExpression(const Variable& index)
    : index((size_t)index.getInteger()), enumeration(index.getType()) {}

  virtual size_t size() const
    {return 1;}

  virtual double compute(const double* x) const
    {return x[index];}

  virtual String toShortString() const
    {return Variable(index, enumeration).toShortString();}

  size_t getIndex() const
    {return index;}

  virtual int compare(const ObjectPtr& otherObject) const
    {return (int)index - (int)otherObject.staticCast<VariableGPExpression>()->index;}

protected:
  friend class VariableGPExpressionClass;

  size_t index;
  EnumerationPtr enumeration;
};

typedef ReferenceCountedObjectPtr<VariableGPExpression> VariableGPExpressionPtr;

class ConstantGPExpression : public GPExpression
{
public:
  ConstantGPExpression(double value = 0.0)
    : value(value) {}

  virtual size_t size() const
    {return 1;}

  virtual double compute(const double* x) const
    {return value;}

  virtual String toShortString() const
    {return String(getValue());}

  double getValue() const
    {return value;}
 
  void setValue(double value)
    {this->value = value;}

  virtual int compare(const ObjectPtr& otherObject) const
    {return Variable(value).compare(otherObject.staticCast<ConstantGPExpression>()->value);}
 
protected:
  friend class ConstantGPExpressionClass;

  double value;
};

typedef ReferenceCountedObjectPtr<ConstantGPExpression> ConstantGPExpressionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_EXPRESSION_H_
