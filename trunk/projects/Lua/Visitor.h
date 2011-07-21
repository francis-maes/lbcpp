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
  virtual void visit(List& list) {}
  virtual void visit(Block& block) {}

  // statements
  virtual void visit(Do& statement) {}
  virtual void visit(Set& statement) {}
  virtual void visit(While& statement) {}
  virtual void visit(Return& statement) {}
  virtual void visit(CallStatement& statement) {}

  // expressions
  virtual void visit(Nil& expression) {}
  virtual void visit(Dots& expression) {}
  virtual void visit(True& expression) {}
  virtual void visit(False& expression) {}
  virtual void visit(LiteralNumber& expression) {}
  virtual void visit(LiteralString& expression) {}
  virtual void visit(Function& function) {}
  virtual void visit(UnaryOperation& operation) {}
  virtual void visit(BinaryOperation& operation) {}
  virtual void visit(Parenthesis& parenthesis) {}
  virtual void visit(Call& call) {}
  virtual void visit(Identifier& identifier) {}
  virtual void visit(Index& index) {}
};

class RewriteVisitor : public Visitor
{
public:
  const NodePtr& getResult() const
    {return result;}

  NodePtr rewrite(const NodePtr& node)
    {node->accept(*this); return result;}

protected:
  NodePtr result;

  void setResult(NodePtr result)
    {this->result = result;}
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_H_

