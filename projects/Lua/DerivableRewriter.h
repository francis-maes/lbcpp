/*-----------------------------------------.---------------------------------.
| Filename: DerivableRewriter.h            | Derivative Rewriter             |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 16:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_REWRITER_DERIVATIVE_H_
# define LBCPP_LUA_REWRITER_DERIVATIVE_H_

# include "Rewriter.h"

namespace lbcpp {
namespace lua {

// compute the derivate of an expression w.r.t a variable
class ExpressionDerivateRewriter : public Rewriter
{
public:
  ExpressionDerivateRewriter(const IdentifierPtr& variable)
    : variable(variable) {}

  static ExpressionPtr computeWrtVariable(const ExpressionPtr& expr, const IdentifierPtr& variableentifier)
  {
    ExpressionDerivateRewriter visitor(variableentifier);
    return visitor.rewrite(expr).staticCast<Expression>();
  }

  static ExpressionPtr ternaryOperator(const ExpressionPtr& condition, const ExpressionPtr& valueIfTrue, const ExpressionPtr& valueIfFalse)
  {
    if (valueIfTrue->print() == valueIfFalse->print())
      return valueIfTrue;
    return new Call(new Identifier("LuaChunk.ternaryOperator"), condition, valueIfTrue, valueIfFalse);
  }

  virtual void visit(LiteralNumber& expression)
    {setResult(new LiteralNumber(0.0));}
 
  virtual void visit(Identifier& identifier)
  {
    if (identifier.getIdentifier() == variable->getIdentifier())
      setResult(new LiteralNumber(1.0)); // d(x)/dx = 1
    else
    {
      setResult(new LiteralNumber(0.0)); // bouh, pas bien. C'est ici que ca devient plus compliqué avec les statements ...
    }
  }

  virtual void visit(UnaryOperation& operation)
  {
    switch (operation.getOp())
    {
    case unmOp:
      // (-u)' = - u'
      setResult(unm(rewrite(operation.getExpr())));
      break;

    case notOp:
    case lenOp:
      jassert(false); // not yet implemented
    }
  }

  virtual void visit(BinaryOperation& operation)
  {
    ExpressionPtr u = operation.getLeft();
    ExpressionPtr v = operation.getRight();
    ExpressionPtr uprime = rewrite(u);
    ExpressionPtr vprime = rewrite(v);

    switch (operation.getOp())
    {
    case addOp: // (u + v)' = u' + v'
      setResult(add(uprime, vprime));
      return;  

    case subOp: // (u - v)' = u' - v'
      setResult(sub(uprime, vprime));
      return;

    case mulOp: // (uv)' = u'v + uv'
      setResult(add(multiply(uprime, v), multiply(u, vprime)));
      return;

    case divOp: // (u/v)' = (u'v - uv') / v^2
      {
        // function (_v) return (u'_v - uv') / _v^2 end (v)
        setResult(div(
            sub(multiply(uprime, v), multiply(u, vprime)),
            pow(v, new LiteralNumber(2))));
          return;
      }
    case powOp:
      {
        LiteralNumberPtr vnumber = v.dynamicCast<LiteralNumber>();
        if (vnumber)
        {
          // u ^ n = (n u^{n-1}) u'
          double n = vnumber->getValue();
          setResult(multiply(multiply(vnumber, pow(u, new LiteralNumber(n - 1.0))), uprime));
          return;
        }
        else
          jassert(false); // not yet implemented
        break;
      }

    case orOp:
      setResult(ternaryOperator(u, uprime, vprime));
      return;
      
    case andOp:
      setResult(ternaryOperator(u, vprime, new LiteralNumber(0.0)));
      return;

    case concatOp:
    case modOp:
    case eqOp:
    case ltOp:
    case leOp:
      std::cerr << "Warning: Unsuported binary operation : " << operation.toString() << std::endl;
      jassert(false); // not yet implemented
    }
  }
  
  virtual void visit(Parenthesis& p)
    {setResult(rewrite(p.getExpr()));}

  virtual void visit(Call& call)
  {
    String fun = call.getFunction()->print();
    if (fun == T("math.exp") && call.getNumArguments() == 1)
    {
      // (exp(u))' = u' exp(u)
      ExpressionPtr u = call.getArgument(0);
      setResult(multiply(rewrite(u), new Call(call.getFunction(), u)));
      return;
    }

    if (fun == T("math.log") && call.getNumArguments() == 1)
    {
      // (log(u))' = u' / u
      ExpressionPtr u = call.getArgument(0);
      setResult(div(rewrite(u), u));
      return;
    }


    if ((fun == T("math.max") || fun == T("math.min")) && call.getNumArguments() == 2)
    {
      // max(u,v)' = (u >= v ? u' : v')
      // min(u,v)' = (u <= v ? u' : v')

      const ExpressionPtr& u = call.getArgument(0);
      const ExpressionPtr& v = call.getArgument(1);
      setResult(ternaryOperator(fun == T("math.max") ? not(lt(u, v)) : le(u, v),
                    rewrite(u), rewrite(v)));
      return;
    }

    if (call.getNumArguments() == 0)
    {
      // no changes
      return;
    }
    else
    {
      // f(u)' = f'(u) u'
      // f(u, v)' = f'1(u, v) u' + f'2(u, v) v'
      // ...

      std::vector<ExpressionPtr> terms;
      terms.reserve(call.getNumArguments());
      for (size_t i = 0; i < call.getNumArguments(); ++i)
      {
        const ExpressionPtr& u = call.getArgument(i);

        // (f[1] or (function (...) return 0 end))(...)
        ExpressionPtr derivateFunction = new Index(call.getFunction(), new LiteralNumber(i + 1));
        derivateFunction = new BinaryOperation(orOp, derivateFunction, new Identifier("LuaChunk.returnZeroFunction"));
        ExpressionPtr newCall = new Call(derivateFunction, call.getArguments());
        ExpressionPtr term = multiply(newCall, rewrite(call.getArgument(i)));
        
        LiteralNumberPtr termNumber = term.dynamicCast<LiteralNumber>();
        if (!termNumber || termNumber->getValue() != 0.0)
          terms.push_back(term); // skip zeros
      }
      if (terms.size())
      {
        ExpressionPtr result = terms.back();
        for (int i = (int)terms.size() - 2; i >= 0; --i) // fold_left
          result = new BinaryOperation(addOp, terms[i], result);
        setResult(result);
      }
      else
        setResult(new LiteralNumber(0.0));
      return;
    }    
  }

protected:
  IdentifierPtr variable;
};

// compute the derivate of a function w.r.t one of its parameters
class FunctionDerivateRewriter : public DefaultRewriter
{
public:
  FunctionDerivateRewriter(const IdentifierPtr& variable)
    : variable(variable), expressionRewriter(variable) {}

  virtual void visit(Return& statement)
  {
    std::vector<ExpressionPtr> newExpressions(statement.getNumReturnValues());
    for (size_t i = 0; i < newExpressions.size(); ++i)
      newExpressions[i] = rewriteExpression(statement.getReturnValue(i));
    setResult(new Return(newExpressions));
  }

  ExpressionPtr rewriteExpression(const ExpressionPtr& expression)
    {return expressionRewriter.rewrite(expression).staticCast<Expression>();}

protected:
  IdentifierPtr variable;
  ExpressionDerivateRewriter expressionRewriter;
};

// Top-level rewriter : replace all occurences of (in a whole block)
// function (derivable x, y) ... end
//  ==>
/*
  setmetatable({
   f = function (x) return 2 * x end,
   dx = function (x) return 2 end
  }, DerivableFunction)
*/
class DerivableRewriter : public DefaultRewriter
{
public:
  static void applyExtension(BlockPtr& block)
    {DerivableRewriter().rewriteChildren((Node&)*block);}

  FunctionPtr computeFunctionDerivate(const FunctionPtr& function, const IdentifierPtr& variable) const
  {
    FunctionDerivateRewriter derivateFunction(variable);
    BlockPtr newBlock = derivateFunction.rewrite(function->getBlock()->cloneAndCast<Block>());
    return new Function(function->getPrototype(), newBlock);
  }

  ExpressionPtr transformFunction(Function& function, const std::vector<IdentifierPtr>& derivables) const
  {
    TablePtr table = new Table();
    
    table->append("f", &function);
    for (size_t i = 0; i < derivables.size(); ++i)
    {
      ExpressionPtr derivateFunction = computeFunctionDerivate(&function, derivables[i]);
      table->append("d" + derivables[i]->getIdentifier(), derivateFunction);
    }

    std::vector<ExpressionPtr> prototype(function.getNumParameters());
    for (size_t i = 0; i < prototype.size(); ++i)
    {
      IdentifierPtr id = function.getParameterIdentifier(i);
      if (id->hasDerivableFlag())
      {
        prototype[i] = new LiteralString("d" + id->getIdentifier());
        function.setParameterIdentifier(i, new Identifier(id->getIdentifier())); // remove "derivable flag"
      }
      else
        prototype[i] = new Nil();
    }
    table->append("prototype", new Table(prototype));

    return new Call(new Identifier("setmetatable"), table, new Index(new Identifier("LuaChunk"), new Identifier("DerivableFunction")));
  }

  virtual void visit(Function& function)
  {
    DefaultRewriter::visit(function); // apply recursively first

    bool atLeastOneDerivable = false;
    std::vector<IdentifierPtr> derivables;
    size_t n = function.getNumParameters();
    for (size_t i = 0; i < n; ++i)
    {
      IdentifierPtr id = function.getParameterIdentifier(i);
      if (id->hasDerivableFlag())
        derivables.push_back(id);
    }
    if (derivables.size() > 0)
      setResult(transformFunction(function, derivables));
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_DERIVATIVE_H_

