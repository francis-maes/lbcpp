/*-----------------------------------------.---------------------------------.
| Filename: GPExpression.cpp               | Genetic Programming Expression  |
| Author  : Francis Maes                   |                                 |
| Started : 29/09/2011 10:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "GPExpression.h"
using namespace lbcpp;

int GPExpression::compare(const ObjectPtr& otherObject) const
{
  jassert(false);
  return true;
}

int GPExpression::createFromString(LuaState& state)
{
  const char* str = state.checkString(1);
  GPExpressionPtr expr = createFromString(state.getContext(), str, positiveIntegerEnumerationEnumeration);
  if (!expr)
    return 0;
  state.pushObject(expr);
  return 1;
}

int GPExpression::compute(LuaState& state)
{
  GPExpressionPtr expr = state.checkObject(1, gpExpressionClass).staticCast<GPExpression>();
  std::vector<double> inputs;
  for (int i = 2; i <= state.getTop(); ++i)
    inputs.push_back(state.toNumber(i));
  double res = expr->compute(&inputs[0]);
  state.pushNumber(res);
  return 1;
}

GPExpressionPtr GPExpression::createFromString(ExecutionContext& context, const String& str, EnumerationPtr variables, int& position)
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

/*
** ConstantGPExpression
*/
ConstantGPExpression::ConstantGPExpression(double value, bool isLearnable)
  : value(value), learnable(isLearnable)
{
}

String ConstantGPExpression::toString() const
  {return T("C(") + String(getValue()) + T(")");}

String ConstantGPExpression::toShortString() const
{
  String res(getValue());
  return learnable ? T("<cst(") + res + T(")>") : res;
}

int ConstantGPExpression::compare(const ObjectPtr& other) const
{
  ConstantGPExpressionPtr otherConstant = other.dynamicCast<ConstantGPExpression>();
  if (otherConstant)
    return value < otherConstant->value ? -1 : (value > otherConstant->value ? 1 : 0);
  else
    return -1; // *constant* < variable < unary < binary
}

/*
** VariableGPExpression
*/
VariableGPExpression::VariableGPExpression(size_t index)
  : index(index)
{
}

VariableGPExpression::VariableGPExpression(const Variable& index)
  : index((size_t)index.getInteger()), enumeration(index.getType())
{
}

String VariableGPExpression::toString() const
  {return T("V(") + Variable(index, enumeration).toString() + T(")");}

String VariableGPExpression::toShortString() const
  {return enumeration ? Variable(index, enumeration).toShortString() : T("v") + String((int)index);}

int VariableGPExpression::compare(const ObjectPtr& other) const
{
  VariableGPExpressionPtr otherVariable = other.dynamicCast<VariableGPExpression>();
  if (otherVariable)
    return (int)index - (int)otherVariable->index;
  else
    return other.isInstanceOf<ConstantGPExpression>() ? 1 : -1; // constant < *variable* < unary < binary
}

/*
** UnaryGPExpression
*/
UnaryGPExpression::UnaryGPExpression(GPPre pre, GPExpressionPtr expr)
  : pre(pre), expr(expr)
{
}

double UnaryGPExpression::compute(const double* x) const
{
  double e = expr->compute(x);
  if (!isNumberValid(e))
    return e;

  switch (pre)
  {
  case gpIdentity: return e;
  case gpLog: return log(e);// e <= 0.0 ? -DBL_MAX : log(e);
  case gpSquareRoot: return sqrt(e); //e < 0.0 ? -DBL_MAX : sqrt(e);
  case gpOpposite: return -e;
  case gpInverse: return 1.0/e; //e != 0.0 ? 1.0 / e : DBL_MAX;
  case gpExp: return exp(e);
  case gpAbs: return fabs(e);
  default: jassert(false); return 0.0;
  };
}

String UnaryGPExpression::toString() const
  {return T("U(") + Variable(pre, gpPreEnumeration).toString() + T(", ") + (expr ? expr->toString() : String("<null>")) + T(")");}

String UnaryGPExpression::toShortString() const
  {return Variable(pre, gpPreEnumeration).toShortString() + T("(") + (expr ? expr->toShortString() : String("<null>")) + T(")");}

int UnaryGPExpression::compare(const ObjectPtr& other) const
{
  UnaryGPExpressionPtr otherUnary = other.dynamicCast<UnaryGPExpression>();
  if (otherUnary)
  {
    if (pre != otherUnary->pre)
      return (int)pre - (int)otherUnary->pre;
    else
      return expr->compare(otherUnary->expr);
  }
  else
    return other.isInstanceOf<BinaryGPExpression>() ? -1 : 1; // constant < variable < *unary* < binary
}

void UnaryGPExpression::clone(ExecutionContext& context, const ObjectPtr& o) const
{
  const UnaryGPExpressionPtr& other = o.staticCast<UnaryGPExpression>();
  other->pre = pre;
  if (expr)
    other->expr = expr->cloneAndCast<GPExpression>(context);
}

/*
** BinaryGPExpression
*/
BinaryGPExpression::BinaryGPExpression(GPExpressionPtr left, GPOperator op, GPExpressionPtr right)
  : left(left), op(op), right(right)
{
}

double BinaryGPExpression::compute(const double* x) const
{
  double l = left->compute(x);
  if (!isNumberValid(l))
    return l;

  double r = right->compute(x);
  if (!isNumberValid(r))
    return r;

  switch (op)
  {
  case gpAddition: return l + r;
  case gpSubtraction: return l - r;
  case gpMultiplication: return l * r;
  case gpDivision: return l / r; //r ? l / r : DBL_MAX;
  case gpMin: return l < r ? l : r;
  case gpMax: return l > r ? l : r;
  case gpPow: return pow(l, r);
  case gpLessThan: return l < r ? 1 : 0;
  default: jassert(false); return 0.0;
  }
}

void BinaryGPExpression::getVariableUseCounts(std::map<size_t, size_t>& res) const
{
  left->getVariableUseCounts(res);
  right->getVariableUseCounts(res);
}

String BinaryGPExpression::toString() const
{
  return T("B(") + Variable(op, gpOperatorEnumeration).toString() + T(", ") +
      (left ? left->toString() : String("<null>")) + T(", ") +
      (right ? right->toString() : String("<null>")) + T(")");
}

String BinaryGPExpression::toShortString() const
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

int BinaryGPExpression::compare(const ObjectPtr& other) const
{
  BinaryGPExpressionPtr otherBinary = other.dynamicCast<BinaryGPExpression>();
  if (otherBinary)
  {
    if (op != otherBinary->op)
      return (int)op - (int)otherBinary->op;
    else
    {
      int res = left->compare(otherBinary->left);
      if (res != 0)
        return res;
      else
        return right->compare(otherBinary->right);
    }
  }
  else
    return 1; // constant < variable < unary < *binary*
}

void BinaryGPExpression::clone(ExecutionContext& context, const ObjectPtr& other) const
{
  const BinaryGPExpressionPtr& o = other.staticCast<BinaryGPExpression>();
  if (left)
    o->left = left->cloneAndCast<GPExpression>(context);
  o->op = op;
  if (right)
    o->right = right->cloneAndCast<GPExpression>(context);
}