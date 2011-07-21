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
  virtual void visit(Return& statement)
    {jassert(false);}
  virtual void visit(CallStatement& statement)
    {jassert(false);}

  // expressions
  virtual void visit(Nil& expression)
    {jassert(false);}
  virtual void visit(Dots& expression)
    {jassert(false);}
  virtual void visit(True& expression)
    {jassert(false);}
  virtual void visit(False& expression)
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
    result = NodePtr();
    node->accept(*this);
    return result ? result : node;
  }

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
  virtual void visit(Return& statement)
    {rewriteChildren(statement);}
  virtual void visit(CallStatement& statement)
    {rewriteChildren(statement);}

  // expressions
  virtual void visit(Nil& expression)   {}
  virtual void visit(Dots& expression)  {}
  virtual void visit(True& expression)  {}
  virtual void visit(False& expression) {}
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

  virtual void visit(Identifier& identifier)  {}

  virtual void visit(Index& index)
    {rewriteChildren(index);}
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_H_
