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
  
class GetScopesVisitor : public DefaultVisitor
{
public:
  
  ScopePtr getRootScope() const
    {return ScopePtr();}

protected:


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

