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
class Table;
typedef ReferenceCountedObjectPtr<Table> TablePtr;

class Visitor;
class Scope;
typedef ReferenceCountedObjectPtr<Scope> ScopePtr;

class LineInfo : public Object
{
public:

  size_t getLine() const
    {return line;}

  size_t getColumn() const
    {return column;}

protected:
  friend class LineInfoClass;

  size_t line;
  size_t column;
  String filename;
  String comments;
};

extern ClassPtr lineInfoClass;

typedef ReferenceCountedObjectPtr<LineInfo> LineInfoPtr;

class Node : public Object
{
public:
  Node() : scope(NULL) {}

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
    {
      NodePtr subNode = getSubNode(i);
      if (subNode)
        target->getSubNode(i) = subNode->cloneAndCast<Node>();
    }
    target->scope = scope;
  }

  String print() const;

  ScopePtr getScope() const
    {return scope;}

  void setScope(const ScopePtr& scope)
    {this->scope = scope.get();}

  const LineInfoPtr& getFirstLineInfo() const
    {return firstLineInfo;}

  const LineInfoPtr& getLastLineInfo() const
    {return lastLineInfo;}

  static int setLineInfo(LuaState& state);

  static int getNumSubNodes(LuaState& state);
  static int setSubNode(LuaState& state);
  static int print(LuaState& state);

protected:
  Scope* scope;
  LineInfoPtr firstLineInfo;
  LineInfoPtr lastLineInfo;
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

  void addStatement(const StatementPtr& statement)
    {statements.push_back(statement);}

  void setStatements(const std::vector<StatementPtr>& statements)
    {this->statements = statements;}

private:
  friend class BlockClass;

  std::vector<StatementPtr> statements;
};

/*
block: { stat* }

stat:
| `Do{ block }
| `Set{ {lhs+} {expr+} }
| `While{ expr block }
| `Repeat{ block expr }
| `If{ (expr block)+ block? }
| `ForNum{ ident expr expr expr? block }
| `ForIn{ {ident+} {expr+} block }
| `Local{ {ident+} {expr+}? }
| `Localrec{ {ident+} {expr+}? }
| `Goto{string}  -- metalua specific
| `Label{string} -- metalua specific
| `Return{expr+}
| `Break
| apply

expr:
| `Nil | `Dots | `True | `False
| `Number{ number }
| `String{ string }
| `Function{ { ident* `Dots? } block } 
| `Table{ ( `Pair{ expr expr } | expr )* }
| `Op{ binopid expr expr } | `Op{ unopid expr }
| `Paren{ expr }
| `Stat{ block expr } -- metalua specific
| apply
| lhs

apply:
| `Call{ expr expr* }
| `Invoke{ expr `String{ string } expr* }

lhs: ident | `Index{ expr expr }

ident: `Id{ string }

binopid: "add" | "sub" | "mul"    | "div"
       | "mod" | "pow" | "concat" | "eq"
       | "lt"  | "le"  | "and"    | "or"

unopid:  "not" | "len" | "unm"
*/

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_NODE_H_
