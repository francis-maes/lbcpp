/*-----------------------------------------.---------------------------------.
| Filename: Expression.h                   | Lua Expressions                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 17:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_AST_EXPRESSION_H_
# define LBCPP_LUA_AST_EXPRESSION_H_

# include "Node.h"

namespace lbcpp {
namespace lua {

class Expression : public Node
{
public:
  virtual LuaChunkType getType() const
    {return luaExpression;}
};

// construction function to simplify the tree on the fly 
extern ExpressionPtr sub(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr add(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr multiply(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr div(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr pow(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr unm(const ExpressionPtr& expr);

extern ExpressionPtr lt(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr le(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr notExpr(const ExpressionPtr& expr);

class AtomicExpression : public Expression
{
public:
  virtual size_t getNumSubNodes() const
    {return 0;}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(false); return *(NodePtr* )0;}
};

class Nil : public AtomicExpression
{
public:
  virtual String getTag() const {return "Nil";}
  virtual void accept(Visitor& visitor);
};

class Dots : public AtomicExpression
{
public:
  virtual String getTag() const {return "Dots";}
  virtual void accept(Visitor& visitor);
};

class LiteralBoolean : public AtomicExpression
{
public:
  virtual String getTag() const {return value ? "True" : "False";}
  virtual void accept(Visitor& visitor);

  bool getValue() const
    {return value;}

  void setValue(bool v)
    {value = v;}

protected:
  friend class LiteralBooleanClass;
  bool value;
};

class LiteralNumber : public AtomicExpression
{
public:
  LiteralNumber(double value = 0.0)
    : value(value) {}

  virtual String getTag() const
    {return "Number";}

  virtual void accept(Visitor& visitor);

  double getValue() const
    {return value;}

protected:
  friend class LiteralNumberClass;

  double value;
};

typedef ReferenceCountedObjectPtr<LiteralNumber> LiteralNumberPtr;

class LiteralString : public AtomicExpression
{
public:
  LiteralString(const String& value)
    : value(value) {}
  LiteralString() {}

  virtual String getTag() const
    {return "String";}

  virtual void accept(Visitor& visitor);

  const String& getValue() const
    {return value;}

protected:
  friend class LiteralStringClass;

  String value;
};

typedef ReferenceCountedObjectPtr<LiteralString> LiteralStringPtr;


/*
** LHS Expression
*/
class LHSExpression : public Expression {};

class Identifier : public LHSExpression
{
public:
  Identifier(const String& identifier)
    : identifier(identifier), derivable(false) {}
  Identifier() : derivable(false)  {}

  virtual String getTag() const
    {return "Id";}

  virtual size_t getNumSubNodes() const
    {return 0;}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(false); return *(NodePtr* )0;}

  virtual void accept(Visitor& visitor);

  const String& getIdentifier() const
    {return identifier;}

  bool hasDerivableFlag() const
    {return derivable;}

protected:
  friend class IdentifierClass;

  String identifier;
  bool derivable; // extension 'derivable'
};

typedef ReferenceCountedObjectPtr<Identifier> IdentifierPtr;

class Index : public LHSExpression
{
public:
  Index(const ExpressionPtr& left, const ExpressionPtr& right)
    : left(left), right(right) {}
  Index(const String& left, const ExpressionPtr& right)
    : left(new Identifier(left)), right(right) {}
  Index(const String& left, const String& right)
    : left(new Identifier(left)), right(new LiteralString(right)) {}
  Index() {}

  virtual String getTag() const
    {return "Index";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)(index ? right : left);}

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getLeft() const
    {return left;}

  const ExpressionPtr& getRight() const
    {return right;}

protected:
  friend class IndexClass;

  ExpressionPtr left;
  ExpressionPtr right;
};

class FunctionClass; // trick, because FunctionClass is already declared in the lbcpp namespace
class Function : public Expression
{
public:
  Function(const ListPtr& prototype, const BlockPtr& block)
    : prototype(prototype), block(block) {}
  Function() {}

  virtual String getTag() const
    {return "Function";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return index ? (NodePtr&)block : (NodePtr&)prototype;}

  virtual void accept(Visitor& visitor);

  const ListPtr& getPrototype() const
    {return prototype;}

  const BlockPtr& getBlock() const
    {return block;}

  size_t getNumParameters() const
    {return prototype->getNumSubNodes();}

  IdentifierPtr getParameterIdentifier(size_t index) const
    {return prototype->getSubNode(index).dynamicCast<Identifier>();}

  void setParameterIdentifier(size_t index, const IdentifierPtr& identifier)
    {prototype->getSubNode(index) = identifier;}

protected:
  friend class FunctionClass;

  ListPtr prototype;
  BlockPtr block;
};

typedef ReferenceCountedObjectPtr<Function> FunctionPtr;

class PairClass; // trick, because lbcpp::PairClass already exists
class Pair : public Expression
{
public:
  Pair(const ExpressionPtr& first, const ExpressionPtr& second)
    : first(first), second(second) {}
  Pair() {}

  virtual String getTag() const
    {return "Pair";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return index ? (NodePtr&)second : (NodePtr&)first;}

  virtual void accept(Visitor& visitor);

protected:
  friend class PairClass;

  ExpressionPtr first;
  ExpressionPtr second;
};

typedef ReferenceCountedObjectPtr<Pair> PairPtr;

class Table : public Expression
{
public:
  Table(const std::vector<NodePtr>& fields)
    : fields(fields) {}
  Table() {}

  virtual String getTag() const
    {return "Table";}

  virtual size_t getNumSubNodes() const
    {return fields.size();}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(index < fields.size()); return fields[index];}

  virtual void accept(Visitor& visitor);

  void append(const String& key, const ExpressionPtr& value)
    {append(new LiteralString(key), value);}

  void append(const ExpressionPtr& key, const ExpressionPtr& value)
    {fields.push_back(new Pair(key, value));}

protected:
  friend class TableClass;

  std::vector<NodePtr> fields; // Expression, Pair or Parameter
};

class Operation : public Expression
{
public:
  virtual String getTag() const
    {return "Op";}

  // low number = low priority
  virtual int getPrecendenceRank() const = 0;
};

typedef ReferenceCountedObjectPtr<Operation> OperationPtr;

enum UnaryOp
{
  notOp = 0,  lenOp,  unmOp,
};

class UnaryOperation : public Operation
{
public:
  UnaryOperation(UnaryOp op, const ExpressionPtr& expr)
    : op(op), expr(expr) {}
  UnaryOperation() : op(notOp) {}

  virtual int getPrecendenceRank() const;

  virtual size_t getNumSubNodes() const
    {return 1;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)expr;}

  virtual void accept(Visitor& visitor);

  UnaryOp getOp() const
    {return op;}

  const ExpressionPtr& getExpr() const
    {return expr;}

  void setExpr(const ExpressionPtr& expr)
    {this->expr = expr;}

protected:
  friend class UnaryOperationClass;

  UnaryOp op;
  ExpressionPtr expr;
};

typedef ReferenceCountedObjectPtr<UnaryOperation> UnaryOperationPtr;

enum BinaryOp
{
  addOp = 0,  subOp,    mulOp,    divOp,
  modOp,      powOp,    concatOp, eqOp,
  ltOp,       leOp,     andOp,    orOp
};

class BinaryOperation : public Operation
{
public:
  BinaryOperation(BinaryOp op, const ExpressionPtr& left, const ExpressionPtr& right)
    : op(op), left(left), right(right) {}
  BinaryOperation() {}

  virtual int getPrecendenceRank() const;

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)(index ? right : left);}

  virtual void accept(Visitor& visitor);

  BinaryOp getOp() const
    {return op;}

  const ExpressionPtr& getLeft() const
    {return left;}

  const ExpressionPtr& getRight() const
    {return right;}

protected:
  friend class BinaryOperationClass;

  BinaryOp op;
  ExpressionPtr left;
  ExpressionPtr right;
};

typedef ReferenceCountedObjectPtr<BinaryOperation> BinaryOperationPtr;

class Parenthesis : public Expression
{
public:
  Parenthesis(const ExpressionPtr& expr)
    : expr(expr) {}
  Parenthesis() {}

  virtual String getTag() const
    {return "Paren";}

  virtual size_t getNumSubNodes() const
    {return 1;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)expr;}

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getExpr() const
    {return expr;}

protected:
  friend class ParenthesisClass;

  ExpressionPtr expr;
};

/*
** Apply Expression
*/
class ApplyExpression : public Expression {};

class Call : public ApplyExpression
{
public:
  Call(const ExpressionPtr& function, const std::vector<ExpressionPtr>& arguments)
    : function(function), arguments(arguments) {}
  Call(const ExpressionPtr& function, const ExpressionPtr& argument1, const ExpressionPtr& argument2, const ExpressionPtr& argument3)
    : function(function), arguments(3) {arguments[0] = argument1; arguments[1] = argument2; arguments[2] = argument3;}
  Call(const ExpressionPtr& function, const ExpressionPtr& argument1, const ExpressionPtr& argument2)
    : function(function), arguments(2) {arguments[0] = argument1; arguments[1] = argument2;}
  Call(const String& function, const ExpressionPtr& argument1, const ExpressionPtr& argument2)
    : function(new Identifier(function)), arguments(2) {arguments[0] = argument1; arguments[1] = argument2;}
  Call(const ExpressionPtr& function, const ExpressionPtr& argument)
    : function(function), arguments(1, argument) {}
  Call(const String& function, const ExpressionPtr& argument)
    : function(new Identifier(function)), arguments(1, argument) {}
  Call(const ExpressionPtr& function)
    : function(function) {}
  Call(const String& function)
    : function(new Identifier(function)) {}
  Call() {}

  virtual String getTag() const
    {return "Call";}

  virtual size_t getNumSubNodes() const
    {return 1 + arguments.size();}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)(index == 0 ? function : arguments[index - 1]);}

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getFunction() const
    {return function;}

  size_t getNumArguments() const
    {return arguments.size();}

  const ExpressionPtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const std::vector<ExpressionPtr>& getArguments() const
    {return arguments;}

  void addArgument(const ExpressionPtr& argument)
    {arguments.push_back(argument);}
    
  void addArguments(const std::vector<ExpressionPtr>& arguments)
  {
    this->arguments.reserve(this->arguments.size() + arguments.size());
    for (size_t i = 0; i < arguments.size(); ++i)
      this->arguments.push_back(arguments[i]);
  }

protected:
  friend class CallClass;

  ExpressionPtr function;
  std::vector<ExpressionPtr> arguments;
};

typedef ReferenceCountedObjectPtr<Call> CallPtr;

class Invoke : public ApplyExpression
{
public:
  virtual String getTag() const
    {return "Invoke";}

  virtual size_t getNumSubNodes() const
    {return 2 + arguments.size();}

  virtual NodePtr& getSubNode(size_t index)
  {
    if (index == 0)
      return (NodePtr&)object;
    if (index == 1)
      return (NodePtr&)function;
    return (NodePtr&)arguments[index - 2];
  }

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getObject() const
    {return object;}

  const LiteralStringPtr& getFunction() const
    {return function;}

  size_t getNumArguments() const
    {return arguments.size();}

  const ExpressionPtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const std::vector<ExpressionPtr>& getArguments() const
    {return arguments;}

protected:
  friend class InvokeClass;

  ExpressionPtr object;
  LiteralStringPtr function;
  std::vector<ExpressionPtr> arguments;
};

class Subspecified : public Expression
{
public:
  Subspecified(const ExpressionPtr& expr)
    : expr(expr) {}
  Subspecified() {}

  virtual String getTag() const
    {return "Subspecified";}

  virtual size_t getNumSubNodes() const
    {return 1;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)expr;}

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getExpr() const
    {return expr;}

protected:
  friend class SubspecifiedClass;

  ExpressionPtr expr;
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_EXPRESSION_H_
