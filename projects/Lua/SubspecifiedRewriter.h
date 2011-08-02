/*-----------------------------------------.---------------------------------.
| Filename: SubspecifiedRewriter.h         | Subspecified Rewriter           |
| Author  : Francis Maes                   |                                 |
| Started : 02/08/2011 19:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_REWRITER_SUBSPECIFIED_H_
# define LBCPP_LUA_REWRITER_SUBSPECIFIED_H_

# include "Rewriter.h"
# include "Scope.h"
# include "ScopeVisitor.h"

namespace lbcpp {
namespace lua {

// Top-level rewriter : replace all occurences of (in a whole block)
// subspecified [expr]
//  ==>
/*
  setmetatable({
   parameter = {...},
   functor = function (__parameters) return [...] end
  }, Subspecified.MT)
*/

class SubspecifiedRewriter : public DefaultRewriter
{
public:
  SubspecifiedRewriter(ExecutionContextPtr context)
    : DefaultRewriter(context) {}

  static void applyExtension(ExecutionContext& context, BlockPtr& block)
    {SubspecifiedRewriter(&context).acceptChildren((Node&)*block);}

  virtual void visit(Subspecified& subspecified)
  {
    ExpressionPtr expression = subspecified.getExpr();

    TablePtr table = new Table();

    TablePtr parameters = makeParameters(expression);
    if (parameters->getNumSubNodes() == 0)
      warning(*expression, "subspecified expression has no parameters");
    table->append("parameters", parameters);
    table->append("functor", makeFunctor(expression));

    setResult(new Call("setmetatable", table, new Index("Subspecified", "MT")));
  }

private:
  struct GetParametersVisitor : public DefaultVisitor
  {
    GetParametersVisitor() : parameters(new Table()) {}

    TablePtr parameters;

    virtual void visit(Parameter& statement)
    {
      parameters->append(statement.getIdentifier(), statement.getProperties());
      DefaultVisitor::visit(statement);
    }
  };

  static TablePtr makeParameters(ExpressionPtr expression)
  {
    GetParametersVisitor visitor;
    expression->accept(visitor);
    return visitor.parameters;
  }
  
  struct MakeFunctorRewriter : public DefaultRewriter
  {
    virtual void visit(Parameter& statement)
      {setResult(new EmptyNode());} // remove parameter declarations

    // replace param by __parameters.param 
    virtual void visit(Identifier& identifier)
    {
      ScopePtr thisScope = identifier.getScope();
      jassert(thisScope);
      VariablePtr variable = thisScope->findVariable(identifier.getIdentifier());
      if (variable)
      {
        ParameterPtr parameter = variable->getDeclarator().dynamicCast<Parameter>();
        if (parameter)
          setResult(new Index("__parameters", identifier.getIdentifier()));
      }
    }
  };

  static FunctionPtr makeFunctor(ExpressionPtr expression)
  {
    ExpressionPtr modifiedExpression = MakeFunctorRewriter().rewrite(expression).staticCast<Expression>();

    BlockPtr block = new Block();
    block->addStatement(new Return(modifiedExpression));
    return new Function(new List(new Identifier("__parameters")), block);
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_SUBSPECIFIED_H_

