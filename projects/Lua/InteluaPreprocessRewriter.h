/*-----------------------------------------.---------------------------------.
| Filename: InteluaPreprocessRewriter.h    | Intelua Preprocessor            |
| Author  : Francis Maes                   |                                 |
| Started : 03/08/2011 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef INTELUA_REWRITER_PREPROCESS_H_
# define INTELUA_REWRITER_PREPROCESS_H_

# include "Node.h"
# include "Scope.h"

namespace lbcpp {
namespace lua {

class InteluaPreprocessRewriter : public DefaultRewriter
{
public:
  // remove parenthesis:
  // Parenthesis(x) => x
  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}

  // replace unary minus literals:
  // - LiteralNumber(x) => LiteralNumber(-x)
  virtual void visit(UnaryOperation& operation)
  {
    if (operation.getOp() == unmOp)
    {
      LiteralNumberPtr number = operation.getExpr().dynamicCast<LiteralNumber>();
      if (number)
        setResult(new LiteralNumber(-number->getValue()));
    }
  }

  // transform invokation into call
  // a:b(...)  ==> a.b(a, ...)
  virtual void visit(Invoke& invoke)
  {
    CallPtr call = new Call(new Index(invoke.getObject(), invoke.getFunction()));
    call->addArgument(invoke.getObject());
    call->addArguments(invoke.getArguments());
    setResult(call);
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !INTELUA_REWRITER_PREPROCESS_H_
