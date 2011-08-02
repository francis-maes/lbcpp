/*-----------------------------------------.---------------------------------.
| Filename: Statement.h                    | Lua Statements                  |
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 17:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_AST_STATEMENT_H_
# define LBCPP_LUA_AST_STATEMENT_H_

# include "Node.h"

namespace lbcpp {
namespace lua {

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
  Set(const ListPtr& lhs, const ListPtr& expr)
    : lhs(lhs), expr(expr) {}
  Set(const IdentifierPtr& identifier, const ExpressionPtr& expression)
    : lhs(new List(identifier)), expr(new List(expression)) {}
  Set() {}

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
    {return index ? (NodePtr&)block : (NodePtr&)condition;}

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getCondition() const
    {return condition;}

  const BlockPtr& getBlock() const
    {return block;}

protected:
  friend class WhileClass;

  ExpressionPtr condition;
  BlockPtr block;
};

class Repeat : public Statement
{
public:
  virtual String getTag() const
    {return "Repeat";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return index ? (NodePtr&)condition : (NodePtr&)block;}

  virtual void accept(Visitor& visitor);

  const BlockPtr& getBlock() const
    {return block;}

  const ExpressionPtr& getCondition() const
    {return condition;}

protected:
  friend class RepeatClass;

  BlockPtr block;
  ExpressionPtr condition;
};

class If : public Statement
{
public:
  virtual String getTag() const
    {return "If";}

  virtual size_t getNumSubNodes() const
    {return conditions.size() + blocks.size();}

  virtual NodePtr& getSubNode(size_t index)
  {
    bool cond = (index % 2 == 0);
    index /= 2;
    return cond ? (NodePtr&)blocks[index] : (NodePtr&)conditions[index];
  }

  virtual void accept(Visitor& visitor);

  size_t getNumConditions() const
    {return conditions.size();}

  const ExpressionPtr& getCondition(size_t index) const
    {jassert(index < conditions.size()); return conditions[index];}

  size_t getNumBlocks() const
    {return blocks.size();}

  const BlockPtr& getBlock(size_t index) const
    {jassert(index < blocks.size()); return blocks[index];}

protected:
  friend class IfClass;

  std::vector<ExpressionPtr> conditions;
  std::vector<BlockPtr> blocks;
};

class ForNum : public Statement
{
public:
  virtual String getTag() const
    {return "ForNum";}

  virtual size_t getNumSubNodes() const
    {return step ? 5 : 4;}

  virtual NodePtr& getSubNode(size_t index)
  {
    if (index == 0)
      return (NodePtr&)identifier;
    if (index == 1)
      return (NodePtr&)from;
    if (index == 2)
      return (NodePtr&)to;
    if (index == 3)
      return step ? (NodePtr&)step : (NodePtr&)block;
    return (NodePtr&)block;
  }

  virtual void accept(Visitor& visitor);

  const IdentifierPtr& getIdentifier() const
    {return identifier;}

  const ExpressionPtr& getFrom() const
    {return from;}

  const ExpressionPtr& getTo() const
    {return to;}

  const ExpressionPtr& getStep() const
    {return step;}

  const BlockPtr& getBlock() const
    {return block;}

protected:
  friend class ForNumClass;

  IdentifierPtr identifier;
  ExpressionPtr from;
  ExpressionPtr to;
  ExpressionPtr step;
  BlockPtr block;
};

// `ForIn{ {ident+} {expr+} block }
class ForIn : public Statement
{
public:
  virtual String getTag() const
    {return "ForIn";}

  virtual size_t getNumSubNodes() const
    {return 3;}

  virtual NodePtr& getSubNode(size_t index)
  {
    if (index == 0)
      return (NodePtr&)identifiers;
    else if (index == 1)
      return (NodePtr&)expressions;
    else
      return (NodePtr&)block;
  }

  virtual void accept(Visitor& visitor);

  const ListPtr& getIdentifiers() const
    {return identifiers;}

  const ListPtr& getExpressions() const
    {return expressions;}

  const BlockPtr& getBlock() const
    {return block;}

protected:
  friend class ForInClass;

  ListPtr identifiers;
  ListPtr expressions;
  BlockPtr block;
};

class Local : public Statement
{
public:
  Local(const ListPtr& identifiers, const ListPtr& expressions)
    : identifiers(identifiers), expressions(expressions), localFunction(false) {}
  Local(const IdentifierPtr& identifier, const ExpressionPtr& expression)
    : identifiers(new List(identifier)), expressions(new List(expression)), localFunction(false) {}
  Local() : localFunction(false) {}

  virtual String getTag() const
    {return localFunction ? "Localrec" : "Local";}

  virtual size_t getNumSubNodes() const
    {return expressions ? 2 : 1;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)(index ? expressions : identifiers);}

  virtual void accept(Visitor& visitor);

  const ListPtr& getIdentifiers() const
    {return identifiers;}

  const ListPtr& getExpressions() const
    {return expressions;}

  bool isFunction() const
    {return localFunction;} // scopes do not behave the same

protected:
  friend class LocalClass;

  ListPtr identifiers;
  ListPtr expressions;
  bool localFunction;
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

class Break : public Statement
{
public:
  virtual String getTag() const
    {return "Break";}

  virtual size_t getNumSubNodes() const
    {return 0;}

  virtual NodePtr& getSubNode(size_t index)
    {return *(NodePtr* )0;}

  virtual void accept(Visitor& visitor);
};

class ExpressionStatement : public Statement
{
public:
  virtual String getTag() const
    {return Node::getSubNode(0)->getTag();}

  virtual size_t getNumSubNodes() const
    {return 1;}

  virtual NodePtr& getSubNode(size_t index)
    {return (NodePtr&)expression;}

  virtual void accept(Visitor& visitor);

  const ExpressionPtr& getExpression() const
    {return expression;}

protected:
  friend class ExpressionStatementClass;

  ExpressionPtr expression;
};

class Parameter : public Statement
{
public:
  virtual String getTag() const
    {return "Parameter";}

  virtual size_t getNumSubNodes() const
    {return 2;}

  virtual NodePtr& getSubNode(size_t index)
    {return index ? (NodePtr&)properties : (NodePtr&)identifier;}

  virtual void accept(Visitor& visitor);

  const IdentifierPtr& getIdentifier() const
    {return identifier;}

  const TablePtr& getProperties() const
    {return properties;}

protected:
  friend class ParameterClass;

  IdentifierPtr identifier;
  TablePtr      properties;
};

typedef ReferenceCountedObjectPtr<Parameter> ParameterPtr;

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_STATEMENT_H_
