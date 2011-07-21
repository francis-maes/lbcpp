/*-----------------------------------------.---------------------------------.
| Filename: LuaAST.h                       | Lua AST                         |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 12:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_AST_H_
# define LBCPP_LUA_AST_H_

# include <lbcpp/Lua/Lua.h>

namespace lbcpp
{

enum LuaChunkType
{
  luaExpression = 0,
  luaStatement,
  luaStatementBlock,
  luaOtherChunk,
};

namespace lua
{
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

  class Node : public Object
  {
  public:
    virtual String getTag() const = 0;
    virtual LuaChunkType getType() const = 0;

    virtual size_t getNumSubNodes() const = 0;
    virtual NodePtr getSubNode(size_t index) const = 0;
  };

  extern ClassPtr nodeClass;

  class List : public Node
  {
  public:
    virtual String getTag() const
      {return String::empty;}

    virtual LuaChunkType getType() const
      {return luaOtherChunk;}

    virtual size_t getNumSubNodes() const
      {return nodes.size();}

    virtual NodePtr getSubNode(size_t index) const
      {jassert(index < nodes.size()); return nodes[index];}

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

    virtual NodePtr getSubNode(size_t index) const
      {jassert(index < statements.size()); return statements[index];}

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

    virtual NodePtr getSubNode(size_t index) const
      {return block;}

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

    virtual NodePtr getSubNode(size_t index) const
      {return index ? expr : lhs;}

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

    virtual NodePtr getSubNode(size_t index) const
      {return index ? (NodePtr)block : (NodePtr)expr;}

  protected:
    friend class WhileClass;

    ExpressionPtr expr;
    BlockPtr block;
  };

  class Return : public Statement
  {
  public:
    virtual String getTag() const
      {return "Return";}

    virtual size_t getNumSubNodes() const
      {return 1;}

    virtual NodePtr getSubNode(size_t index) const
      {return expr;}

  protected:
    friend class ReturnClass;

    ExpressionPtr expr;
  };

  class CallStatement : public Statement
  {
  public:
    virtual String getTag() const
      {return "Call";}

    virtual size_t getNumSubNodes() const
      {return 1 + arguments.size();}

    virtual NodePtr getSubNode(size_t index) const
      {return index == 0 ? function : arguments[index - 1];}

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

  class AtomicExpression : public Expression
  {
  public:
    virtual size_t getNumSubNodes() const
      {return 0;}

    virtual NodePtr getSubNode(size_t index) const
      {jassert(false); return NodePtr();}
  };

  class Nil : public AtomicExpression
  {
  public:
    virtual String getTag() const {return "Nil";}
  };

  class Dots : public AtomicExpression
  {
  public:
    virtual String getTag() const {return "Dots";}
  };

  class True : public AtomicExpression
  {
  public:
    virtual String getTag() const {return "True";}
  };

  class False : public AtomicExpression
  {
  public:
    virtual String getTag() const {return "False";}
  };

  class LiteralNumber : public AtomicExpression
  {
  public:
    virtual String getTag() const
      {return "Number";}

  protected:
    friend class LiteralNumberClass;

    double value;
  };

  class LiteralString : public AtomicExpression
  {
  public:
    virtual String getTag() const
      {return "String";}

  protected:
    friend class LiteralStringClass;

    String value;
  };

  class FunctionClass; // trick, because FunctionClass is already declared in the lbcpp namespace
  class Function : public Expression
  {
  public:
    virtual String getTag() const
      {return "Function";}
  
    virtual size_t getNumSubNodes() const
      {return 2;}

    virtual NodePtr getSubNode(size_t index) const
      {return index ? (NodePtr)block : (NodePtr)prototype;}

  protected:
    friend class FunctionClass;

    ListPtr prototype;
    BlockPtr block;
  };

  enum UnaryOp
  {
    notOp = 0,  lenOp,  unmOp,
  };
  
  class UnaryOperation : public Expression
  {
  public:
    virtual String getTag() const
      {return "Op";}

    virtual size_t getNumSubNodes() const
      {return 1;}

    virtual NodePtr getSubNode(size_t index) const
      {return expr;}

  protected:
    friend class UnaryOperationClass;

    UnaryOp op;
    ExpressionPtr expr;
  };

  enum BinaryOp
  {
    addOp = 0,  subOp,    mulOp,    divOp,
    modOp,      powOp,    concatOp, eqOp,
    ltOp,       leOp,     andOp,    orOp
  };

  class BinaryOperation : public Expression
  {
  public:
    virtual String getTag() const
      {return "Op";}

    virtual size_t getNumSubNodes() const
      {return 2;}

    virtual NodePtr getSubNode(size_t index) const
      {return index ? right : left;}

  protected:
    friend class BinaryOperationClass;

    BinaryOp op;
    ExpressionPtr left;
    ExpressionPtr right;
  };

  class Parenthesis : public Expression
  {
  public:
    virtual String getTag() const
      {return "Paren";}

    virtual size_t getNumSubNodes() const
      {return 1;}

    virtual NodePtr getSubNode(size_t index) const
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
    virtual String getTag() const
      {return "Call";}

    virtual size_t getNumSubNodes() const
      {return 1 + arguments.size();}

    virtual NodePtr getSubNode(size_t index) const
      {return index == 0 ? function : arguments[index - 1];}

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
    virtual String getTag() const
      {return "Id";}

    virtual size_t getNumSubNodes() const
      {return 0;}

    virtual NodePtr getSubNode(size_t index) const
      {jassert(false); return NodePtr();}

  protected:
    friend class IdentifierClass;

    String identifier;
  };

  class Index : public LHSExpression
  {
  public:
    virtual String getTag() const
      {return "Index";}

    virtual size_t getNumSubNodes() const
      {return 2;}

    virtual NodePtr getSubNode(size_t index) const
      {return index ? right : left;}

  protected:
    friend class IndexClass;

    ExpressionPtr left;
    ExpressionPtr right;
  };

}; /* namespace lua */

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
x| `Return{expr}
| `Break
p| apply

expr:
x| `Nil | `Dots | `True | `False
x| `Number{ number }
x| `String{ string }
x| `Function{ { ident* `Dots? } block } 
| `Table{ ( `Pair{ expr expr } | expr )* }
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

class LuaASTNode;
typedef ReferenceCountedObjectPtr<LuaASTNode> LuaASTNodePtr;

class LuaASTNode : public Object
{
public:
  LuaASTNode(const String& tag, const std::vector<Variable>& variables, const std::vector<LuaASTNodePtr>& childNodes)
    : tag(tag), variables(variables), childNodes(childNodes) {}
  LuaASTNode(const String& tag, const std::vector<LuaASTNodePtr>& childNodes)
    : tag(tag), childNodes(childNodes) {}
  LuaASTNode(const std::vector<LuaASTNodePtr>& childNodes)
    : childNodes(childNodes) {}
  LuaASTNode(const String& rawCode)
    : tag(T("Raw")), variables(1, rawCode) {}
  LuaASTNode() {}

  static LuaASTNodePtr newOp(const String& opid, const LuaASTNodePtr& operand)
  {
    std::vector<LuaASTNodePtr> childNodes(1);
    childNodes[0] = operand;
    return new LuaASTNode(T("Op"), std::vector<Variable>(1, opid), childNodes);
  }

  static LuaASTNodePtr newOp(const String& opid, const LuaASTNodePtr& left, const LuaASTNodePtr& right)
  {
    std::vector<LuaASTNodePtr> childNodes(2);
    childNodes[0] = left;
    childNodes[1] = right;
    return new LuaASTNode(T("Op"), std::vector<Variable>(1, opid), childNodes);
  }

  static LuaASTNodePtr newParen(const LuaASTNodePtr& content)
    {return new LuaASTNode(T("Paren"), std::vector<LuaASTNodePtr>(1, content));}

  static LuaASTNodePtr newNumber(double value)
    {return new LuaASTNode(T("Number"), std::vector<Variable>(1, value), std::vector<LuaASTNodePtr>());}

  const String& getTag() const
    {return tag;}

  size_t getNumChildNodes() const
    {return childNodes.size();}

  LuaASTNodePtr getChildNode(size_t index) const
    {jassert(index < childNodes.size()); return childNodes[index];}

  const std::vector<LuaASTNodePtr>& getChildNodes() const
    {return childNodes;}

  size_t getNumArguments() const
    {return variables.size();}

  Variable getArgument(size_t index) const
    {return variables[index];}

  const std::vector<Variable>& getArguments() const
    {return variables;}

  virtual String toString() const
  {
    String res = tag;
    if (tag.isNotEmpty())
      res += T("(");

    bool needComma = false;
    for (size_t i = 0; i < variables.size(); ++i)
    {
      if (needComma)
        res += T(",");
      res += variables[i].toString();
      needComma = true;
    }
    for (size_t i = 0; i < childNodes.size(); ++i)
    {
      if (needComma)
        res += T(",");
      res += childNodes[i]->toString();
      needComma = true;
    }

    if (tag.isNotEmpty())
      res += T(")");
    return res;
  }

  virtual String toShortString() const
    {return toString();}

  String print() const;

protected:
  friend class LuaASTNodeClass;

  String tag;
  std::vector<Variable> variables;
  std::vector<LuaASTNodePtr> childNodes;
};

extern ClassPtr luaASTNodeClass;



}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_H_
