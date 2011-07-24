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
  
class Variable : public Object
{
public:
  Variable(const IdentifierPtr& declaration, const ExpressionPtr& initialValue)
    : declaration(declaration), initialValue(initialValue), isConstant(true), isUsed(false) {}
  Variable() : isConstant(true), isUsed(false) {}

  const String& getIdentifier() const
    {return declaration->getIdentifier();}

  IdentifierPtr declaration;
  ExpressionPtr initialValue;
  bool isConstant;
  bool isUsed;
};

typedef ReferenceCountedObjectPtr<Variable> VariablePtr;

class Scope;
typedef ReferenceCountedObjectPtr<Scope> ScopePtr;

class Scope : public NameableObject
{
public:
  Scope(const NodePtr& node, const String& name = "Scope");
  Scope() : parent(NULL) {}

  static ScopePtr get(NodePtr tree);
  static void print(NodePtr tree);

  void addSubScope(const ScopePtr& subScope);
  ScopePtr getSubScope(const NodePtr& ownerNode);

  void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue);
  void variableSet(IdentifierPtr identifier, ExpressionPtr value);
  void variableGet(IdentifierPtr identifier);

  VariablePtr findVariable(const IdentifierPtr& identifier, bool recursively = true) const;

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

