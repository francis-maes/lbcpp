/*-----------------------------------------.---------------------------------.
| Filename: GetScopesVisitor.h             | A visitor to create Scopes      |
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 04:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_VISITOR_GET_SCOPES_H_
# define LBCPP_LUA_VISITOR_GET_SCOPES_H_

# include "Scope.h"
# include "Visitor.h"

namespace lbcpp {
namespace lua {
  
class ScopeVisitor : public DefaultVisitor
{
public:
  virtual void enterScope(Node& node) {}
  virtual void leaveScope() {}
  virtual void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue = ExpressionPtr()) {}

  virtual void visit(Do& statement)
  {
    enterScope(statement);
    visitChildren(statement);
    leaveScope();
  }

  virtual void visit(Set& statement)
  {
    ListPtr expr = statement.getExpr();
    expr->accept(*this);

    ListPtr lhs = statement.getLhs();
    for (size_t i = 0; i < lhs->getNumSubNodes(); ++i)
    {
      NodePtr lhsi = lhs->getSubNode(i);
      IdentifierPtr identifier = lhsi.dynamicCast<Identifier>();
      if (identifier) // FIXME: multiret
        newVariable(identifier, expr->getSubNode(i));
      else
        lhsi->accept(*this);
    }
  }

  virtual void visit(While& statement)
  {
    statement.getCondition()->accept(*this);
    enterScope(statement);
    statement.getBlock()->accept(*this);
    leaveScope();
  }

  /*virtual void visit(Repeat& statement)
  {
    enterScope(statement);
    statement.getBlock()->accept(*this);
    statement.getCondition()->accept(*this);
    leaveScope();
  }*/

  virtual void visit(If& statement)
    {jassert(false);}

  virtual void visit(Local& statement)
  {
    const ListPtr& expressions = statement.getExpressions();
    size_t numExpressions = 0;
    if (expressions)
    {
      expressions->accept(*this);
      numExpressions = expressions->getNumSubNodes();
    }

    const ListPtr& identifiers = statement.getIdentifiers();
    for (size_t i = 0; i < identifiers->getNumSubNodes(); ++i)
    {
      IdentifierPtr identifier = identifiers->getSubNode(i).dynamicCast<Identifier>();
      newVariable(identifier, i < numExpressions ? expressions->getSubNode(i) : ExpressionPtr());
    }
  }

  virtual void visit(Return& statement)
    {visitChildren(statement);}
  virtual void visit(ExpressionStatement& statement)
    {visitChildren(statement);}

  virtual void visit(Function& function)
  {
    enterScope(function);
    for (size_t i = 0; i < function.getNumParameters(); ++i)
    {
      IdentifierPtr parameter = function.getParameterIdentifier(i);
      if (parameter)
        newVariable(parameter);
    }
    function.getBlock()->accept(*this);
    leaveScope();
  }

};

class GetScopesVisitor : public ScopeVisitor
{
public:
  virtual void enterScope(Node& node)
  {
    std::cout << "Enter Scope " << node.getTag() << std::endl;
  }

  virtual void leaveScope()
  {
    std::cout << "Leave Scope" << std::endl;
  }
  virtual void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue = ExpressionPtr())
  {
    std::cout << "  New Variable " << identifier->getIdentifier();
    if (initialValue)
      std::cout << " = " << initialValue->print();
    std::cout << std::endl;
  }

  ScopePtr getRootScope() const
    {return ScopePtr();}
};

ScopePtr Scope::get(NodePtr tree)
{
  GetScopesVisitor visitor;
  tree->accept(visitor);
  return visitor.getRootScope();
}

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_GET_SCOPES_H_

