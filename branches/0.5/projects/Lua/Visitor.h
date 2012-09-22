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
  Visitor(ExecutionContextPtr context = ExecutionContextPtr())
    : context(context) {}
  virtual ~Visitor() {}

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
  virtual void visit(Parameter& statement)
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
  virtual void visit(Subspecified& subspecified)
    {jassert(false);}

protected:
  ExecutionContextPtr context;

  virtual void accept(NodePtr& node)
    {node->accept(*this);}

  virtual void acceptChildren(Node& node)
  {
    size_t n = node.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      accept(node.getSubNode(i));
  }

  void error(Node& node, const String& message)
  {
    jassert(context);
    context->errorCallback(message); // todo: "where" string
  }

  void warning(Node& node, const String& message)
  {
    jassert(context);
    context->warningCallback(message); // todo: "where" string
  }
};

template<class BaseClass>
class DefaultVisitorT : public BaseClass
{
public:
  DefaultVisitorT(ExecutionContextPtr context = ExecutionContextPtr())
    : BaseClass(context) {}

  virtual void visit(List& list)
    {BaseClass::acceptChildren(list);}

  virtual void visit(Block& block)
    {BaseClass::acceptChildren(block);}

  // statements
  virtual void visit(Do& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(Set& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(While& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(Repeat& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(If& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(ForNum& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(ForIn& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(Local& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(Return& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(Break& statement)
    {}
  virtual void visit(ExpressionStatement& statement)
    {BaseClass::acceptChildren(statement);}
  virtual void visit(Parameter& statement)
    {BaseClass::acceptChildren(statement);}

  // expressions
  virtual void visit(Nil& expression)   {}
  virtual void visit(Dots& expression)  {}
  virtual void visit(LiteralBoolean& expression) {}
  virtual void visit(LiteralNumber& expression) {}
  virtual void visit(LiteralString& expression) {}
  virtual void visit(Function& function)
    {BaseClass::acceptChildren(function);}

  virtual void visit(Pair& pair)
    {BaseClass::acceptChildren(pair);}

  virtual void visit(Table& table)
    {BaseClass::acceptChildren(table);}

  virtual void visit(UnaryOperation& operation)
    {BaseClass::acceptChildren(operation);}

  virtual void visit(BinaryOperation& operation)
    {BaseClass::acceptChildren(operation);}

  virtual void visit(Parenthesis& parenthesis)
    {BaseClass::acceptChildren(parenthesis);}

  virtual void visit(Call& call)
    {BaseClass::acceptChildren(call);}

  virtual void visit(Invoke& invoke)
    {BaseClass::acceptChildren(invoke);}

  virtual void visit(Identifier& identifier)  {}

  virtual void visit(Index& index)
    {BaseClass::acceptChildren(index);}

  virtual void visit(Subspecified& subspecified)
    {BaseClass::acceptChildren(subspecified);}
};

class DefaultVisitor : public DefaultVisitorT<Visitor>
{
public:
  DefaultVisitor(ExecutionContextPtr context = ExecutionContextPtr())
    : DefaultVisitorT<Visitor>(context) {}
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_H_

