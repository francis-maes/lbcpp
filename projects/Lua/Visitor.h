/*-----------------------------------------.---------------------------------.
| Filename: Visitor.h                      | Base class for writing visitors |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 16:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_VISITOR_H_
# define LBCPP_LUA_VISITOR_H_

# include "Node.h"

namespace lbcpp {
namespace lua {
  
class Visitor
{
public:
  virtual void visit(List& list)
    {jassert(false);}
  virtual void visit(Block& block)
    {jassert(false);}

  // statements
  virtual void visit(Do& statement)
    {jassert(false);}
  virtual void visit(Set& statement)
    {jassert(false);}
  virtual void visit(While& statement)
    {jassert(false);}
  virtual void visit(If& statement)
    {jassert(false);}
  virtual void visit(Return& statement)
    {jassert(false);}
  virtual void visit(CallStatement& statement)
    {jassert(false);}

  // expressions
  virtual void visit(Nil& expression)
    {jassert(false);}
  virtual void visit(Dots& expression)
    {jassert(false);}
  virtual void visit(LiteralBoolean& expression)
    {jassert(false);}
  virtual void visit(LiteralNumber& expression)
    {jassert(false);}
  virtual void visit(LiteralString& expression)
    {jassert(false);}
  virtual void visit(Function& function)
    {jassert(false);}
  virtual void visit(Pair& pair)
    {jassert(false);}
  virtual void visit(Table& table)
    {jassert(false);}
  virtual void visit(UnaryOperation& operation)
    {jassert(false);}
  virtual void visit(BinaryOperation& operation)
    {jassert(false);}
  virtual void visit(Parenthesis& parenthesis)
    {jassert(false);}
  virtual void visit(Call& call)
    {jassert(false);}
  virtual void visit(Invoke& invoke)
    {jassert(false);}
  virtual void visit(Identifier& identifier)
    {jassert(false);}
  virtual void visit(Index& index)
    {jassert(false);}
};

class Rewriter : public Visitor
{
public:
  const NodePtr& getResult() const
    {return result;}

  void rewriteChildren(Node& node)
  {
    size_t n = node.getNumSubNodes();
    NodePtr prevResult = result;
    for (size_t i = 0; i < n; ++i)
    {
      result = NodePtr();
      NodePtr& subNode = node.getSubNode(i);
      subNode->accept(*this);
      if (result)
        subNode = result;
    }
    result = prevResult;
  }

  NodePtr rewrite(const NodePtr& node)
  {
    NodePtr prevResult = result;
    result = NodePtr();
    node->accept(*this);
    NodePtr res = result ? result : node;
    result = prevResult;
    return res;
  }

  void apply(NodePtr& node)
    {node = rewrite(node);}

protected:
  NodePtr result;

  void setResult(NodePtr result)
    {this->result = result;}
};

class DefaultRewriter : public Rewriter
{
public:
  virtual void visit(List& list)
    {rewriteChildren(list);}

  virtual void visit(Block& block)
    {rewriteChildren(block);}

  // statements
  virtual void visit(Do& statement)
    {rewriteChildren(statement);}
  virtual void visit(Set& statement)
    {rewriteChildren(statement);}
  virtual void visit(While& statement)
    {rewriteChildren(statement);}
  virtual void visit(If& statement)
    {rewriteChildren(statement);}
  virtual void visit(Return& statement)
    {rewriteChildren(statement);}
  virtual void visit(CallStatement& statement)
    {rewriteChildren(statement);}

  // expressions
  virtual void visit(Nil& expression)   {}
  virtual void visit(Dots& expression)  {}
  virtual void visit(LiteralBoolean& expression) {}
  virtual void visit(LiteralNumber& expression) {}
  virtual void visit(LiteralString& expression) {}
  virtual void visit(Function& function)
    {rewriteChildren(function);}

  virtual void visit(Pair& pair)
    {rewriteChildren(pair);}

  virtual void visit(Table& table)
    {rewriteChildren(table);}

  virtual void visit(UnaryOperation& operation)
    {rewriteChildren(operation);}

  virtual void visit(BinaryOperation& operation)
    {rewriteChildren(operation);}

  virtual void visit(Parenthesis& parenthesis)
    {rewriteChildren(parenthesis);}

  virtual void visit(Call& call)
    {rewriteChildren(call);}

  virtual void visit(Invoke& invoke)
    {rewriteChildren(invoke);}

  virtual void visit(Identifier& identifier)  {}

  virtual void visit(Index& index)
    {rewriteChildren(index);}
};

class RemoveParenthesisRewriter : public DefaultRewriter
{
public:
  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}
};

class RemoveUnmLiteralRewriter : public DefaultRewriter
{
public:
  virtual void visit(UnaryOperation& operation)
  {
    if (operation.getOp() == unmOp)
    {
      LiteralNumberPtr number = operation.getExpr().dynamicCast<LiteralNumber>();
      if (number)
        setResult(new LiteralNumber(-number->getValue()));
    }
  }
};

class TransformInvokeIntoCallRewriter : public DefaultRewriter
{
public:
  // a:b(...)  ==> a.b(a, ...)
  virtual void visit(Invoke& invoke)
  {
    CallPtr call = new Call(new Index(invoke.getObject(), invoke.getFunction()));
    call->addArgument(invoke.getObject());
    call->addArguments(invoke.getArguments());
    setResult(call);
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_H_

