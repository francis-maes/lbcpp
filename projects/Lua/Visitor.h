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
# include "Expression.h"
# include "Statement.h"

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
  virtual void visit(Repeat& statement)
    {jassert(false);}
  virtual void visit(If& statement)
    {jassert(false);}
  virtual void visit(ForNum& statement)
    {jassert(false);}
  virtual void visit(ForIn& statement)
    {jassert(false);}
  virtual void visit(Local& statement)
    {jassert(false);}
  virtual void visit(Return& statement)
    {jassert(false);}
  virtual void visit(Break& statement)
    {jassert(false);}
  virtual void visit(ExpressionStatement& statement)
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

template<class BaseClass>
class DefaultVisitorT : public BaseClass
{
public:
  virtual void visitChildren(Node& node) = 0;

  virtual void visit(List& list)
    {visitChildren(list);}

  virtual void visit(Block& block)
    {visitChildren(block);}

  // statements
  virtual void visit(Do& statement)
    {visitChildren(statement);}
  virtual void visit(Set& statement)
    {visitChildren(statement);}
  virtual void visit(While& statement)
    {visitChildren(statement);}
  virtual void visit(Repeat& statement)
    {visitChildren(statement);}
  virtual void visit(If& statement)
    {visitChildren(statement);}
  virtual void visit(ForNum& statement)
    {visitChildren(statement);}
  virtual void visit(ForIn& statement)
    {visitChildren(statement);}
  virtual void visit(Local& statement)
    {visitChildren(statement);}
  virtual void visit(Return& statement)
    {visitChildren(statement);}
  virtual void visit(Break& statement)
    {}
  virtual void visit(ExpressionStatement& statement)
    {visitChildren(statement);}

  // expressions
  virtual void visit(Nil& expression)   {}
  virtual void visit(Dots& expression)  {}
  virtual void visit(LiteralBoolean& expression) {}
  virtual void visit(LiteralNumber& expression) {}
  virtual void visit(LiteralString& expression) {}
  virtual void visit(Function& function)
    {visitChildren(function);}

  virtual void visit(Pair& pair)
    {visitChildren(pair);}

  virtual void visit(Table& table)
    {visitChildren(table);}

  virtual void visit(UnaryOperation& operation)
    {visitChildren(operation);}

  virtual void visit(BinaryOperation& operation)
    {visitChildren(operation);}

  virtual void visit(Parenthesis& parenthesis)
    {visitChildren(parenthesis);}

  virtual void visit(Call& call)
    {visitChildren(call);}

  virtual void visit(Invoke& invoke)
    {visitChildren(invoke);}

  virtual void visit(Identifier& identifier)  {}

  virtual void visit(Index& index)
    {visitChildren(index);}
};

class DefaultVisitor : public DefaultVisitorT<Visitor>
{
public:
  virtual void visitChildren(Node& node)
  {
    size_t n = node.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      node.getSubNode(i)->accept(*this);
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_H_

