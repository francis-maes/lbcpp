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
  NodePtr rewrite(const NodePtr& node)
    {NodePtr res = node; accept(res); return res;}

protected:
  NodePtr result;

  void setResult(NodePtr result)
    {this->result = result;}

  virtual void accept(NodePtr& node)
  {
    NodePtr prevResult = result;
    result = NodePtr();
    node->accept(*this);
    if (result)
    {
      if (!result->getScope() && node->getScope())
        result->setScope(node->getScope()); // forward scope information
      node = result;
    }
    result = prevResult;
  }
};

class DefaultRewriter : public DefaultVisitorT<Rewriter>
{
public:
  virtual void visit(Block& block)
  {
    size_t n = block.getNumSubNodes();
    NodePtr prevResult = result;
    bool hasNewSubBlocks = false;
    for (size_t i = 0; i < n; ++i)
    {
      result = NodePtr();
      NodePtr& subNode = block.getSubNode(i);
      if (!subNode)
        continue; // skip non existing sub trees
      subNode->accept(*this);
      if (result)
      {
        subNode = result;
        hasNewSubBlocks |= result.isInstanceOf<Block>();
      }
    }
    result = prevResult;

    // flatten blocks
    if (hasNewSubBlocks)
    {
      std::vector<StatementPtr> statements;
      fillStatementsRecursively(&block, statements);
      block.setStatements(statements);
    }
  }

private:
  void fillStatementsRecursively(const BlockPtr& block, std::vector<StatementPtr>& res)
  {
    size_t n = block->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      BlockPtr subBlock = block->getSubNode(i).dynamicCast<Block>();
      if (subBlock)
        fillStatementsRecursively(subBlock, res);
      else
      {
        StatementPtr statement = block->getSubNode(i).dynamicCast<Statement>();
        jassert(statement);
        res.push_back(statement);
      }
    }
  }
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

