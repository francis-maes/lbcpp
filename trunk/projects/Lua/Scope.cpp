/*-----------------------------------------.---------------------------------.
| Filename: Scope.cpp                      | Scope class                     |
| Author  : Francis Maes                   |                                 |
| Started : 23/07/2011 13:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "Scope.h"
#include "ScopeVisitor.h"
using namespace lbcpp::lua;

/*
** Scope
*/
Scope::Scope(const NodePtr& node, const String& name)
  : NameableObject(name), parent(NULL), node(node) {}

void Scope::addSubScope(const ScopePtr& subScope)
{
  subScope->parent = this;
  subScopes.push_back(subScope);
}

ScopePtr Scope::getSubScope(const NodePtr& ownerNode)
{
  for (size_t i = 0; i < subScopes.size(); ++i)
    if (subScopes[i]->getOwnerNode() == ownerNode)
      return subScopes[i];
  return ScopePtr();
}

void Scope::newVariable(IdentifierPtr identifier, ExpressionPtr initialValue)
  {variables.push_back(new Variable(identifier, initialValue));}

void Scope::variableSet(IdentifierPtr identifier, ExpressionPtr value)
{
  VariablePtr variable = findVariable(identifier);
  if (variable)
    variable->isConstant = false;
  else
    std::cerr << "Could not find variable " << identifier->getIdentifier() << std::endl;
}

void Scope::variableGet(IdentifierPtr identifier)
{
  VariablePtr variable = findVariable(identifier);
  if (variable)
    variable->isUsed = true;
  else
    std::cerr << "Could not find variable " << identifier->getIdentifier() << std::endl;
}

VariablePtr Scope::findVariable(const IdentifierPtr& identifier, bool recursively) const
{
  const String& id = identifier->getIdentifier();
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i]->getIdentifier() == id)
      return variables[i];
  return recursively && parent ? parent->findVariable(identifier) : VariablePtr();
}

/*
** PrintScopesVisitor
*/
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

  virtual void variableGet(IdentifierPtr identifier)
    {std::cout << "getVariable " << identifier->getIdentifier() << std::endl;}
};

void Scope::print(NodePtr tree)
{
  PrintScopesVisitor visitor;
  tree->accept(visitor);
}

/*
** GetScopesVisitor
*/
class GetScopesVisitor : public ScopeVisitor
{
public:
  GetScopesVisitor(ScopePtr rootScope)
    : currentScope(rootScope) {}

  virtual void enterScope(Node& node)
  {
    ScopePtr newScope = new Scope(&node, node.getTag()); // todo: better name
    currentScope->addSubScope(newScope);
    node.setScope(newScope);
    scopes.push_back(currentScope);
    currentScope = newScope;
  }

  virtual void leaveScope()
    {currentScope = scopes.back(); scopes.pop_back();}

  virtual void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue = ExpressionPtr())
  {
    identifier->setScope(currentScope);
    currentScope->newVariable(identifier, initialValue);
  }

  virtual void variableSet(IdentifierPtr identifier, ExpressionPtr value)
  {
    identifier->setScope(currentScope);
    currentScope->variableSet(identifier, value);
  }

  virtual void variableGet(IdentifierPtr identifier)
  {
    identifier->setScope(currentScope);
    currentScope->variableGet(identifier);
  }

  const ScopePtr& getCurrentScope() const
    {return currentScope;}

  virtual void accept(NodePtr& node)
    {node->setScope(currentScope); ScopeVisitor::accept(node);}

protected:
  std::vector<ScopePtr> scopes;
  ScopePtr currentScope;
};

ScopePtr Scope::get(NodePtr tree)
{
  ScopePtr rootScope = new Scope(tree, "<root>");
  GetScopesVisitor visitor(rootScope);
  visitor.accept(tree);
  return rootScope;
}

/*
** ScopeVisitor
*/
void ScopeVisitor::visit(Do& statement)
{
  enterScope(statement);
  acceptChildren(statement);
  leaveScope();
}

void ScopeVisitor::visit(Set& statement)
{
  ListPtr expr = statement.getExpr();
  expr->accept(*this);

  ListPtr lhs = statement.getLhs();

  jassert(expr->getNumSubNodes() == lhs->getNumSubNodes()); // FIXME: multiret
  for (size_t i = 0; i < lhs->getNumSubNodes(); ++i)
  {
    NodePtr lhsi = lhs->getSubNode(i);
    IdentifierPtr identifier = lhsi.dynamicCast<Identifier>();
    if (identifier)
      variableSet(identifier, expr->getSubNode(i));
    else
      lhsi->accept(*this);
  }
}

void ScopeVisitor::visit(While& statement)
{
  statement.getCondition()->accept(*this);
  enterScope(statement);
  statement.getBlock()->accept(*this);
  leaveScope();
}

void ScopeVisitor::visit(Repeat& statement)
{
  enterScope(statement);
  statement.getBlock()->accept(*this);
  statement.getCondition()->accept(*this);
  leaveScope();
}

void ScopeVisitor::visit(If& statement)
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

void ScopeVisitor::visit(ForNum& statement)
{
  enterScope(statement);
  newVariable(statement.getIdentifier());
  for (size_t i = 1; i < statement.getNumSubNodes(); ++i)
    statement.getSubNode(i)->accept(*this);
  leaveScope();
}

void ScopeVisitor::visit(ForIn& statement)
{
  enterScope(statement);

  const ListPtr& identifiers = statement.getIdentifiers();
  for (size_t i = 0; i < identifiers->getNumSubNodes(); ++i)
    newVariable(identifiers->getSubNode(i).staticCast<Identifier>());

  statement.getExpressions()->accept(*this);
  statement.getBlock()->accept(*this);

  leaveScope();
}

void ScopeVisitor::visit(Local& statement)
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

void ScopeVisitor::visit(Function& function)
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

void ScopeVisitor::visit(Identifier& identifier)
  {variableGet(&identifier);}
