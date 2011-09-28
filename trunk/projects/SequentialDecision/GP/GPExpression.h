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
  gpAddition = 0,
  gpSubtraction,
  gpMultiplication,
  gpDivision,
  gpMin,
  gpMax,
  gpPow,
  gpLessThan,
};

extern EnumerationPtr gpOperatorEnumeration;

enum GPPre
{
  gpIdentity = 0,
  gpOpposite,
  gpInverse,
  gpSquareRoot,
  gpLog,
  gpExp,
  gpAbs,
};

extern EnumerationPtr gpPreEnumeration;
extern EnumerationPtr gpConstantEnumeration;

class GPExpression;
typedef ReferenceCountedObjectPtr<GPExpression> GPExpressionPtr;
extern ClassPtr gpExpressionClass;

class GPExpression : public Object
{
public:
  virtual double compute(const double* x) const = 0;
  virtual size_t size() const = 0;
  virtual size_t getNumSubExpressions() const
    {return 0;}
  virtual GPExpressionPtr getSubExpression(size_t index) const
    {jassert(false); return GPExpressionPtr();}

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const = 0;

  static GPExpressionPtr createFromString(ExecutionContext& context, const String& str, EnumerationPtr variables)
    {int position = 0; return createFromString(context, str, variables, position);}
  static  GPExpressionPtr createFromString(ExecutionContext& context, const String& str, EnumerationPtr variables, int& position);

  static int createFromString(LuaState& state)
  {
    const char* str = state.checkString(1);
    GPExpressionPtr expr = createFromString(state.getContext(), str, positiveIntegerEnumerationEnumeration);
    if (!expr)
      return 0;
    state.pushObject(expr);
    return 1;
  }

  static int compute(LuaState& state)
  {
    GPExpressionPtr expr = state.checkObject(1, gpExpressionClass).staticCast<GPExpression>();
    std::vector<double> inputs;
    for (int i = 2; i <= state.getTop(); ++i)
      inputs.push_back(state.toNumber(i));
    double res = expr->compute(&inputs[0]);
    state.pushNumber(res);
    return 1;
  }
};

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

  virtual size_t getNumSubExpressions() const
    {return 1;}
  
  virtual GPExpressionPtr getSubExpression(size_t index) const
    {return expr;}

  virtual double compute(const double* x) const
  {
    double e = expr->compute(x);
    if (!isNumberValid(e))
      return e;

    switch (pre)
    {
    case gpIdentity: return e;
    case gpLog: return e <= 0.0 || !isNumberValid(e) ? -DBL_MAX : log(e);
    case gpSquareRoot: return e < 0.0 || !isNumberValid(e) ? -DBL_MAX : sqrt(e);
    case gpOpposite: return -e;
    case gpInverse: return e != 0.0 ? 1.0 / e : DBL_MAX;
    case gpExp: return exp(e);
    case gpAbs: return fabs(e);
    default: jassert(false); return 0.0;
    };
  }

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
    {expr->getVariableUseCounts(res);}

  virtual String toString() const
    {return T("U(") + Variable(pre, gpPreEnumeration).toString() + T(", ") + (expr ? expr->toString() : String("<null>")) + T(")");}

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
    if (expr)
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

  virtual size_t getNumSubExpressions() const
    {return 2;}
  
  virtual GPExpressionPtr getSubExpression(size_t index) const
    {return index ? right : left;}

  virtual double compute(const double* x) const
  {
    double l = left->compute(x);
    double r = right->compute(x);
    if (!isNumberValid(l))
      return l;
    if (!isNumberValid(r))
      return r;

    switch (op)
    {
    case gpAddition: return l + r;
    case gpSubtraction: return l - r;
    case gpMultiplication: return l * r;
    case gpDivision: return r ? l / r : DBL_MAX;
    case gpMin: return l < r ? l : r;
    case gpMax: return l > r ? l : r;
    case gpPow: return pow(l, r);
    case gpLessThan: return l < r ? 1 : 0;
    default: jassert(false); return 0.0;
    }
  }

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
  {
    left->getVariableUseCounts(res);
    right->getVariableUseCounts(res);
  }

  virtual String toString() const
  {
    return T("B(") + Variable(op, gpOperatorEnumeration).toString() + T(", ") +
        (left ? left->toString() : String("<null>")) + T(", ") +
        (right ? right->toString() : String("<null>")) + T(")");
  }

  virtual String toShortString() const
  {
    String l = (left ? left->toShortString() : String("<null>"));
    String r = (right ? right->toShortString() : String("<null>"));

    if (op >= gpMin)
      return gpOperatorEnumeration->getElementName(op) + String("(") + l + T(", ") + r + T(")");
    else
    {
      const char* names[] = {"+", "-", "*", "/"};
      return T("(") + l + T(")") +
              names[op] +
             T("(") + r + T(")");
    }
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
    if (left)
      o->left = left->cloneAndCast<GPExpression>(context);
    o->op = op;
    if (right)
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

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
    {res[index]++;}

  virtual String toString() const
    {return T("V(") + Variable(index, enumeration).toString() + T(")");}

  virtual String toShortString() const
    {return enumeration ? Variable(index, enumeration).toShortString() : T("v") + String((int)index);}

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
  ConstantGPExpression(double value = 0.0, bool isLearnable = false)
    : value(value), learnable(isLearnable) {}

  virtual size_t size() const
    {return 1;}

  virtual double compute(const double* x) const
    {return value;}

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
    {}

  virtual String toString() const
    {return T("C(") + String(getValue()) + T(")");}

  virtual String toShortString() const
  {
    String res(getValue());
    return learnable ? T("<cst(") + res + T(")>") : res;
  }

  double getValue() const
    {return value;}
 
  void setValue(double value)
    {this->value = value;}

  virtual int compare(const ObjectPtr& otherObject) const
    {return Variable(value).compare(otherObject.staticCast<ConstantGPExpression>()->value);}
 
  bool isLearnable() const
    {return learnable;}

protected:
  friend class ConstantGPExpressionClass;

  double value;
  bool learnable;
};

typedef ReferenceCountedObjectPtr<ConstantGPExpression> ConstantGPExpressionPtr;

inline GPExpressionPtr GPExpression::createFromString(ExecutionContext& context, const String& str, EnumerationPtr variables, int& position)
{
  if (position >= str.length())
  {
    context.errorCallback(T("Unexpected end of string in ") + str);
    return GPExpressionPtr();
  }

  while (position < str.length() && str[position] == ' ')
    ++position;

  if (str[position] == 'B')
  {
    position += 2;
    int comma = str.indexOfChar(position, ',');
    if (comma < 0) {context.errorCallback(T("Syntax error in ") + str); return GPExpressionPtr();}
    String opString = str.substring(position, comma);
    int op = gpOperatorEnumeration->findElementByName(opString);
    if (op < 0)
    {
      context.errorCallback(T("Could not parse operator ") + opString);
      return GPExpressionPtr();
    }
    position = comma + 1;
    GPExpressionPtr left = createFromString(context, str, variables, position);
    if (!left)
      return GPExpressionPtr();
    jassert(str[position] == ','); ++ position;
    GPExpressionPtr right = createFromString(context, str, variables, position);
    if (!right)
      return GPExpressionPtr();
    jassert(str[position] == ')'); ++position;
    return new BinaryGPExpression(left, (GPOperator)op, right);
  }
  else if (str[position] == 'U')
  {
    position += 2;
    int comma = str.indexOfChar(position, ',');
    if (comma < 0) {context.errorCallback(T("Syntax error in ") + str); return GPExpressionPtr();}
    String opString = str.substring(position, comma);
    int op = gpPreEnumeration->findElementByName(opString);
    if (op < 0)
    {
      context.errorCallback(T("Could not parse operator ") + opString);
      return GPExpressionPtr();
    }
    position = comma + 1;
    GPExpressionPtr expr = createFromString(context, str, variables, position);
    if (!expr)
      return GPExpressionPtr();
    jassert(str[position] == ')'); ++position;
    return new UnaryGPExpression((GPPre)op, expr);
  }
  else if (str[position] == 'V')
  {
    position += 2;
    int paren = str.indexOfChar(position, ')');
    if (paren < 0) {context.errorCallback(T("Syntax error in ") + str); return GPExpressionPtr();}
    String idString = str.substring(position, paren);
    int id;
    if (variables == positiveIntegerEnumerationEnumeration)
    {
      id = idString.getIntValue();
    }
    else
    {
      id = variables->findElementByName(idString);
      if (id < 0)
      {
        context.errorCallback(T("Could not parse variable ") + idString);
        return GPExpressionPtr();
      }
    }
    position = paren + 1;
    return new VariableGPExpression(Variable(id, variables));
  }
  else if (str[position] == 'C')
  {
    position += 2;
    int paren = str.indexOfChar(position, ')');
    if (paren < 0) {context.errorCallback(T("Syntax error in ") + str); return GPExpressionPtr();}
    String cstString = str.substring(position, paren);
    position = paren + 1;
    return new ConstantGPExpression(cstString.getDoubleValue());
  }
  else
  {
    context.errorCallback(T("Could not parse ") + str);
    return GPExpressionPtr();
  }
}


class GPStructureScoreObject : public ScoreObject
{
public:
  GPStructureScoreObject(GPExpressionPtr expression, double score)
    : expression(expression), score(score) {}
  GPStructureScoreObject() : score(0.0) {}

  virtual double getScoreToMinimize() const
    {return score;}

  const GPExpressionPtr& getExpression() const
    {return expression;}

protected:
  friend class GPStructureScoreObjectClass;
  
  GPExpressionPtr expression;
  double score;
};

typedef ReferenceCountedObjectPtr<GPStructureScoreObject> GPStructureScoreObjectPtr;
extern ClassPtr gpStructureScoreObjectClass;

}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_EXPRESSION_H_
