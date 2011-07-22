/*-----------------------------------------.---------------------------------.
| Filename: Rewriter.h                     | Base class for writing rewriters|
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 04:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_REWRITER_H_
# define LBCPP_LUA_REWRITER_H_

# include "Visitor.h"

namespace lbcpp {
namespace lua {

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

class DefaultRewriter : public DefaultVisitorT<Rewriter>
{
public:
  virtual void visitChildren(Node& node)
    {rewriteChildren(node);}
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

#endif // !LBCPP_LUA_REWRITER_H_

