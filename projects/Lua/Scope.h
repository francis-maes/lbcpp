/*-----------------------------------------.---------------------------------.
| Filename: Scope.h                        | Scope class                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 04:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_SCOPE_H_
# define LBCPP_LUA_SCOPE_H_

# include "Node.h"
# include "Expression.h"

namespace lbcpp {
namespace lua {
  
class Scope;
typedef ReferenceCountedObjectPtr<Scope> ScopePtr;

class Variable : public Object
{
public:
  Variable(const NodePtr& declarator, const IdentifierPtr& declaration, const ExpressionPtr& initialValue)
    : declarator(declarator), declaration(declaration), initialValue(initialValue), isConstant(true), isUsed(false) {}
  Variable() : isConstant(true), isUsed(false) {}

  const String& getIdentifier() const
    {return declaration->getIdentifier();}

  ScopePtr getScope() const
    {return declaration->getScope();}

  const NodePtr& getDeclarator() const
    {return declarator;}

  NodePtr declarator; // ForNum, ForIn, Local, Parameter, Function
  IdentifierPtr declaration;
  ExpressionPtr initialValue;
  bool isConstant;
  bool isUsed;
};

typedef ReferenceCountedObjectPtr<Variable> VariablePtr;

class Scope : public NameableObject
{
public:
  Scope(const NodePtr& node, const String& name = "Scope");
  Scope() : parent(NULL) {}

  static ScopePtr get(NodePtr tree);
  static void print(NodePtr tree);

  void addSubScope(const ScopePtr& subScope);
  ScopePtr getSubScope(const NodePtr& ownerNode);

  void newVariable(NodePtr declarator, IdentifierPtr identifier, ExpressionPtr initialValue);
  void variableSet(IdentifierPtr identifier, ExpressionPtr value);
  void variableGet(IdentifierPtr identifier);

  VariablePtr findVariable(const String& identifier, bool recursively = true) const;
  VariablePtr findVariable(const IdentifierPtr& identifier, bool recursively = true) const
    {return findVariable(identifier->getIdentifier(), recursively);}

  ScopePtr getParentScope() const
    {return parent;}

  NodePtr getOwnerNode() const
    {return node;} // Do, While, Repeat, If, ForNum, ForIn or Function

protected:
  friend class ScopeClass;

  Scope* parent;
  NodePtr node;
  std::vector<VariablePtr> variables;
  std::vector<ScopePtr> subScopes;
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_SCOPE_H_

