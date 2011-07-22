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
  Scope(const String& name)
    : NameableObject(name), parent(NULL) {}
  Scope() : parent(NULL) {}

  static ScopePtr get(NodePtr tree);
  static void print(NodePtr tree);

  void addSubScope(const ScopePtr& subScope)
  {
    subScope->parent = this;
    subScopes.push_back(subScope);
  }

  void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue)
    {variables.push_back(new Variable(identifier, initialValue));}

  void variableSet(IdentifierPtr identifier, ExpressionPtr value)
  {
    VariablePtr variable = findVariable(identifier);
    if (variable)
      variable->isConstant = false;
    else
      std::cout << "Could not find variable " << identifier->getIdentifier() << std::endl;
  }

  void variableGet(IdentifierPtr identifier)
  {
    VariablePtr variable = findVariable(identifier);
    if (variable)
      variable->isUsed = true;
    else
      std::cout << "Could not find variable " << identifier->getIdentifier() << std::endl;
    // todo...
  }

  VariablePtr findVariable(const IdentifierPtr& identifier) const
  {
    const String& id = identifier->getIdentifier();
    for (size_t i = 0; i < variables.size(); ++i)
      if (variables[i]->getIdentifier() == id)
        return variables[i];
    return parent ? parent->findVariable(identifier) : VariablePtr();
  }

protected:
  friend class ScopeClass;

  Scope* parent;
  StatementPtr node;
  std::vector<VariablePtr> variables;
  std::vector<ScopePtr> subScopes;
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_SCOPE_H_

