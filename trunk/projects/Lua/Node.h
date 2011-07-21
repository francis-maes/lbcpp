/*-----------------------------------------.---------------------------------.
| Filename: Node.h                         | Lua AST                         |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 12:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_NODE_H_
# define LBCPP_LUA_NODE_H_

# include <lbcpp/Lua/Lua.h>

namespace lbcpp {

enum LuaChunkType
{
  luaExpression = 0,
  luaStatement,
  luaStatementBlock,
  luaOtherChunk,
};

namespace lua {

class Node;
typedef ReferenceCountedObjectPtr<Node> NodePtr;
class List;
typedef ReferenceCountedObjectPtr<List> ListPtr;

class Block;
typedef ReferenceCountedObjectPtr<Block> BlockPtr;
class Statement;
typedef ReferenceCountedObjectPtr<Statement> StatementPtr;
class Expression;
typedef ReferenceCountedObjectPtr<Expression> ExpressionPtr;
class LHSExpression;
typedef ReferenceCountedObjectPtr<LHSExpression> LHSExpressionPtr;
class Identifier;
typedef ReferenceCountedObjectPtr<Identifier> IdentifierPtr;


class Visitor;

class Node : public Object
{
public:
  virtual String getTag() const = 0;
  virtual LuaChunkType getType() const = 0;

  virtual size_t getNumSubNodes() const = 0;
  virtual NodePtr& getSubNode(size_t index) = 0;

  const NodePtr& getSubNode(size_t index) const
    {return const_cast<Node* >(this)->getSubNode(index);}

  virtual void accept(Visitor& visitor) = 0;

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    Object::clone(context, t);
    const NodePtr& target = t.staticCast<Node>();
    size_t n = getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      target->getSubNode(i) = getSubNode(i)->cloneAndCast<Node>();
  }

  String print() const;
};

extern ClassPtr nodeClass;

class List : public Node
{
public:
  List(const std::vector<NodePtr>& nodes)
    : nodes(nodes) {}
  List(const NodePtr& node1, const NodePtr& node2)
    : nodes(2) {nodes[0] = node1; nodes[1] = node2;}
  List(const NodePtr& node)
    : nodes(1, node) {}
  List() {}

  virtual String getTag() const
    {return String::empty;}

  virtual LuaChunkType getType() const
    {return luaOtherChunk;}

  virtual size_t getNumSubNodes() const
    {return nodes.size();}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(index < nodes.size()); return nodes[index];}

  virtual void accept(Visitor& visitor);

private:
  friend class ListClass;

  std::vector<NodePtr> nodes;
};

/*
** Block
*/
class Block : public Node
{
public:
  virtual String getTag() const
    {return String::empty;}

  virtual LuaChunkType getType() const
    {return luaStatementBlock;}

  virtual size_t getNumSubNodes() const
    {return statements.size();}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(index < statements.size()); return (NodePtr& )statements[index];}

  virtual void accept(Visitor& visitor);

private:
  friend class BlockClass;

  std::vector<StatementPtr> statements;
};

/*
** Statement
*/
class Statement : public Node
{
public:
  virtual LuaChunkType getType() const
    {return luaStatement;}
};

class Do : public Statement
{
public:
  virtual String getTag() const
    {return "Do";}

  virtual size_t getNumSubNodes() const
    {return 1;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr& )block;}

  virtual void accept(Visitor& visitor);

protected:
  friend class DoClass;

  BlockPtr block;
};

class Set : public Statement
{
public:
  virtual String getTag() const
    {return "Set";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr& )(index ? expr : lhs);}

  virtual void accept(Visitor& visitor);

  const ListPtr& getLhs() const
    {return lhs;}

  const ListPtr& getExpr() const
    {return expr;}

protected:
  friend class SetClass;
  ListPtr lhs;
  ListPtr expr;
};

class While : public Statement
{
public:
  virtual String getTag() const
    {return "While";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return index ? (NodePtr&)block : (NodePtr&)expr;}

  virtual void accept(Visitor& visitor);

protected:
  friend class WhileClass;

  ExpressionPtr expr;
  BlockPtr block;
};

class Return : public Statement
{
public:
  Return(const std::vector<ExpressionPtr>& expressions)
    : expressions(expressions) {}
  Return() {}

  virtual String getTag() const
    {return "Return";}

  virtual size_t getNumSubNodes() const
    {return expressions.size();}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(index < expressions.size()); return (NodePtr&)expressions[index];}

  virtual void accept(Visitor& visitor);

  size_t getNumReturnValues() const
    {return expressions.size();}

  ExpressionPtr getReturnValue(size_t index) const
    {jassert(index < expressions.size()); return expressions[index];}

protected:
  friend class ReturnClass;

  std::vector<ExpressionPtr> expressions;
};

class CallStatement : public Statement
{
public:
  virtual String getTag() const
    {return "Call";}

  virtual size_t getNumSubNodes() const
    {return 1 + arguments.size();}

  virtual NodePtr& getSubNode(size_t index)
    {return index == 0 ? (NodePtr&)function : (NodePtr&)arguments[index - 1];}

  virtual void accept(Visitor& visitor);

protected:
  friend class CallStatementClass;

  ExpressionPtr function;
  std::vector<ExpressionPtr> arguments;
};

// ...

/*
** Expression
*/
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
extern ExpressionPtr pow(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr parenthesis(const ExpressionPtr& expr);
extern ExpressionPtr unm(const ExpressionPtr& expr);

extern ExpressionPtr lt(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr le(const ExpressionPtr& left, const ExpressionPtr& right);
extern ExpressionPtr not(const ExpressionPtr& expr);

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

class Table : public Expression
{
public:
  Table(const std::vector<ExpressionPtr>& fields)
    : fields(fields) {}
  Table() {}

  virtual String getTag() const
    {return "Table";}

  virtual size_t getNumSubNodes() const
    {return fields.size();}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(index < fields.size()); return (NodePtr&)fields[index];}

  virtual void accept(Visitor& visitor);

  void append(const String& key, const ExpressionPtr& value)
    {fields.push_back(new Pair(new LiteralString(key), value));}

protected:
  friend class TableClass;

  std::vector<ExpressionPtr> fields;
};

typedef ReferenceCountedObjectPtr<Table> TablePtr;

enum UnaryOp
{
  notOp = 0,  lenOp,  unmOp,
};

class UnaryOperation : public Expression
{
public:
  UnaryOperation(UnaryOp op, const ExpressionPtr& expr)
    : op(op), expr(expr) {}
  UnaryOperation() : op(notOp) {}

  virtual String getTag() const
    {return "Op";}

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

class BinaryOperation : public Expression
{
public:
  BinaryOperation(BinaryOp op, const ExpressionPtr& left, const ExpressionPtr& right)
    : op(op), left(left), right(right) {}
  BinaryOperation() {}

  virtual String getTag() const
    {return "Op";}

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
  Call(const ExpressionPtr& function, const ExpressionPtr& argument)
    : function(function), arguments(1, argument) {}
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

protected:
  friend class CallClass;

  ExpressionPtr function;
  std::vector<ExpressionPtr> arguments;
};


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

/*
x block: { stat* }

stat:
x| `Do{ block }
x| `Set{ {lhs+} {expr+} }
x| `While{ expr block }
| `Repeat{ block expr }
| `If{ (expr block)+ block? }
| `Fornum{ ident expr expr expr? block }
| `Forin{ {ident+} {expr+} block }
| `Local{ {ident+} {expr+}? }
| `Localrec{ {ident+} {expr+}? }
| `Goto{string}
| `Label{string}
x| `Return{expr+}
| `Break
p| apply

expr:
x| `Nil | `Dots | `True | `False
x| `Number{ number }
x| `String{ string }
x| `Function{ { ident* `Dots? } block } 
x| `Table{ ( `Pair{ expr expr } | expr )* }
x| `Op{ binopid expr expr } | `Op{ unopid expr }
c| `Paren{ expr }
| `Stat{ block expr }
p| apply
x| lhs

apply:
x| `Call{ expr expr* }
| `Invoke{ expr `String{ string } expr* }

x lhs: ident | `Index{ expr expr }

x ident: `Id{ string }

x binopid: "add" | "sub" | "mul"    | "div"
       | "mod" | "pow" | "concat" | "eq"
       | "lt"  | "le"  | "and"    | "or"

x unopid:  "not" | "len" | "unm"
*/

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_NODE_H_
