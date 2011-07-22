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
  Variable() : isConstant(true), isUsed(false) {}

  IdentifierPtr declaration;
  ExpressionPtr initialValue;
  bool isConstant;
  bool isUsed;
};
typedef ReferenceCountedObjectPtr<Variable> VariablePtr;

class Scope;
typedef ReferenceCountedObjectPtr<Scope> ScopePtr;

class Scope : public Object
{
public:
  static ScopePtr get(NodePtr tree);

protected:
  friend class ScopeClass;

  StatementPtr node;
  ScopePtr parent;
  std::vector<VariablePtr> variables;
  std::vector<ScopePtr> subScopes;
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_SCOPE_H_

