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

  virtual int compare(const ObjectPtr& otherObject) const;

  static int createFromString(LuaState& state);
  static int compute(LuaState& state);
};

extern ClassPtr constantGPExpressionClass;
extern ClassPtr variableGPExpressionClass;
extern ClassPtr unaryGPExpressionClass;
extern ClassPtr binaryGPExpressionClass;

/*
** Constants
*/
class ConstantGPExpression : public GPExpression
{
public:
  ConstantGPExpression(double value = 0.0, bool isLearnable = false);

  virtual size_t size() const
    {return 1;}

  virtual double compute(const double* x) const
    {return value;}

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
    {}

  double getValue() const
    {return value;}
 
  void setValue(double value)
    {this->value = value;}

  bool isLearnable() const
    {return learnable;}

  // Object
  virtual String toString() const;
  virtual String toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;

protected:
  friend class ConstantGPExpressionClass;

  double value;
  bool learnable;
};

typedef ReferenceCountedObjectPtr<ConstantGPExpression> ConstantGPExpressionPtr;

/*
** Variables
*/
class VariableGPExpression : public GPExpression
{
public:
  VariableGPExpression(size_t index = 0);
  VariableGPExpression(const Variable& index);

  virtual size_t size() const
    {return 1;}

  virtual double compute(const double* x) const
    {return x[index];}

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
    {res[index]++;}

  size_t getIndex() const
    {return index;}

  // Object
  virtual String toString() const;
  virtual String toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;

protected:
  friend class VariableGPExpressionClass;

  size_t index;
  EnumerationPtr enumeration;
};

typedef ReferenceCountedObjectPtr<VariableGPExpression> VariableGPExpressionPtr;

/*
** Unary operations
*/
class UnaryGPExpression : public GPExpression
{
public:
  UnaryGPExpression(GPPre pre, GPExpressionPtr expr);
  UnaryGPExpression() {}

  virtual size_t size() const
    {return 1 + expr->size();}

  virtual size_t getNumSubExpressions() const
    {return 1;}
  
  virtual GPExpressionPtr getSubExpression(size_t index) const
    {return expr;}

  virtual double compute(const double* x) const;

  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const
    {expr->getVariableUseCounts(res);}

  GPPre getOperator() const
    {return pre;}

  GPExpressionPtr& getExpression()
    {return expr;}

  const GPExpressionPtr& getExpression() const
    {return expr;}

  // Object
  virtual String toString() const;
  virtual String toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& o) const;
 
protected:
  friend class UnaryGPExpressionClass;

  GPPre pre;
  GPExpressionPtr expr;
};

typedef ReferenceCountedObjectPtr<UnaryGPExpression> UnaryGPExpressionPtr;

/*
** Binary operations
*/
class BinaryGPExpression : public GPExpression
{
public:
  BinaryGPExpression(GPExpressionPtr left, GPOperator op, GPExpressionPtr right);
  BinaryGPExpression() {}

  virtual size_t size() const
    {return left->size() + 1 + right->size();}

  virtual size_t getNumSubExpressions() const
    {return 2;}
  
  virtual GPExpressionPtr getSubExpression(size_t index) const
    {return index ? right : left;}

  virtual double compute(const double* x) const;
  virtual void getVariableUseCounts(std::map<size_t, size_t>& res) const;

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

  // Object
  virtual String toString() const;
  virtual String toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& other) const;

protected:
  friend class BinaryGPExpressionClass;

  GPExpressionPtr left;
  GPOperator op;
  GPExpressionPtr right;
};

typedef ReferenceCountedObjectPtr<BinaryGPExpression> BinaryGPExpressionPtr;

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
