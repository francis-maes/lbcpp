/*-----------------------------------------.---------------------------------.
| Filename: ScopeVisitor.h                 | A visitor to create Scopes      |
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 04:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_VISITOR_SCOPE_H_
# define LBCPP_LUA_VISITOR_SCOPE_H_

# include "Scope.h"
# include "Visitor.h"

namespace lbcpp {
namespace lua {
  
class ScopeVisitor : public DefaultVisitor
{
public:
  /*
  ** Statements
  */
  virtual void visit(Do& statement);
  virtual void visit(Set& statement);
  virtual void visit(While& statement);
  virtual void visit(Repeat& statement);
  virtual void visit(If& statement);
  virtual void visit(ForNum& statement);
  virtual void visit(ForIn& statement);
  virtual void visit(Local& statement);
  // Default implementations for Break, Return and ExpressionStatement

  /*
  ** Expressions
  */
  virtual void visit(Function& function);
  virtual void visit(Identifier& identifier);
  // default implementation for:
  // Table, Pair, UnaryOperation, BinaryOperation
  // Parenthesis, Call, Invoke, Index,
  // Nil, Dots, LiteralBoolean, LiteralNumber, LiteralString

protected:
  virtual void enterScope(Node& node) {}
  virtual void newVariable(IdentifierPtr identifier, ExpressionPtr initialValue = ExpressionPtr()) {}
  virtual void variableSet(IdentifierPtr identifier, ExpressionPtr value) {}
  virtual void variableGet(IdentifierPtr identifier) {}
  virtual void leaveScope() {}
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_SCOPE_H_

