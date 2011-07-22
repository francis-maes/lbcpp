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
  virtual void variableSet(IdentifierPtr identifier, ExpressionPtr value) {}
  virtual void variableGet(IdentifierPtr identifier) {}

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
        variableSet(identifier, expr->getSubNode(i));
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

  virtual void visit(Repeat& statement)
  {
    enterScope(statement);
    statement.getBlock()->accept(*this);
    statement.getCondition()->accept(*this);
    leaveScope();
  }

  virtual void visit(If& statement)
  {
    for (size_t i = 0; i < statement.getNumBlocks(); ++i)
    {
      if (i < statement.getNumConditions())
        statement.getCondition(i)->accept(*this);
      enterScope(statement);
      statement.getBlock(i)->accept(*this);
      leaveScope();
    }
  }

  virtual void visit(ForNum& statement)
  {
    enterScope(statement);
    newVariable(statement.getIdentifier());
    for (size_t i = 1; i < statement.getNumSubNodes(); ++i)
      statement.getSubNode(i)->accept(*this);
    leaveScope();
  }

  virtual void visit(ForIn& statement)
  {
    enterScope(statement);

    const ListPtr& identifiers = statement.getIdentifiers();
    for (size_t i = 0; i < identifiers->getNumSubNodes(); ++i)
      newVariable(identifiers->getSubNode(i).staticCast<Identifier>());

    statement.getExpressions()->accept(*this);
    statement.getBlock()->accept(*this);

    leaveScope();
  }

  virtual void visit(Local& statement)
  {
    // the order between variable declarations and expression evaluation depends
    //  on wheter we declare a local function or anything else
    const ListPtr& expressions = statement.getExpressions();
    size_t numExpressions = expressions ? expressions->getNumSubNodes() : 0;
    
    if (!statement.isFunction() && numExpressions)
      expressions->accept(*this);

    const ListPtr& identifiers = statement.getIdentifiers();
    for (size_t i = 0; i < identifiers->getNumSubNodes(); ++i)
    {
      IdentifierPtr identifier = identifiers->getSubNode(i).dynamicCast<Identifier>();
      newVariable(identifier, i < numExpressions ? expressions->getSubNode(i) : ExpressionPtr());
    }

    if (statement.isFunction() && numExpressions)
      expressions->accept(*this);
  }

  virtual void visit(Return& statement)
    {visitChildren(statement);}

  virtual void visit(ExpressionStatement& statement)
    {visitChildren(statement);}

  /*
  ** Expressions
  */
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

  // default implementation for:
  // Table, Pair, UnaryOperation, BinaryOperation
  // Parenthesis, Call, Invoke, Index

  virtual void visit(Identifier& identifier)
    {variableGet(&identifier);}

  /*
  ** Terminal Nodes
  */
  // statements
  virtual void visit(Break& statement)            {}

  // expressions
  virtual void visit(Nil& expression)             {}
  virtual void visit(Dots& expression)            {}
  virtual void visit(LiteralBoolean& expression)  {}
  virtual void visit(LiteralNumber& expression)   {}
  virtual void visit(LiteralString& expression)   {}
};

class PrintScopesVisitor : public ScopeVisitor
{
public:
  virtual void enterScope(Node& node)
    {std::cout << "Enter Scope " << node.getTag() << std::endl;}

  virtual void leaveScope()
    {std::cout << "Leave Scope" << std::endl;}

  virtual void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue = ExpressionPtr())
  {
    std::cout << "newVariable " << identifier->getIdentifier();
    if (initialValue)
      std::cout << " = " << initialValue->print();
    std::cout << std::endl;
  }
  virtual void variableSet(IdentifierPtr identifier, ExpressionPtr value)
    {std::cout << "setVariable " << identifier->getIdentifier() << " = " << value->print() << std::endl;}

  virtual void variableUsed(IdentifierPtr identifier)
    {std::cout << "getVariable " << identifier->getIdentifier() << std::endl;}
};

void Scope::print(NodePtr tree)
{
  PrintScopesVisitor visitor;
  tree->accept(visitor);
}

class GetScopesVisitor : public ScopeVisitor
{
public:
  GetScopesVisitor() : currentScope(new Scope(T("<root>"))) {}

  virtual void enterScope(Node& node)
  {
    ScopePtr newScope = new Scope(node.getTag()); // todo: better name
    currentScope->addSubScope(newScope);
    scopes.push_back(currentScope);
    currentScope = newScope;
  }

  virtual void leaveScope()
    {currentScope = scopes.back(); scopes.pop_back();}

  virtual void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue = ExpressionPtr())
    {currentScope->newVariable(identifier, initialValue);}

  virtual void variableSet(IdentifierPtr identifier, ExpressionPtr value)
    {currentScope->variableSet(identifier, value);}

  virtual void variableGet(IdentifierPtr identifier)
    {currentScope->variableGet(identifier);}

  const ScopePtr& getCurrentScope() const
    {return currentScope;}

protected:
  std::vector<ScopePtr> scopes;
  ScopePtr currentScope;
};

ScopePtr Scope::get(NodePtr tree)
{
  GetScopesVisitor visitor;
  tree->accept(visitor);
  return visitor.getCurrentScope();
}

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_GET_SCOPES_H_

