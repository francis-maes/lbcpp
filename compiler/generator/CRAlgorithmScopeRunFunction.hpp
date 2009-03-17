/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmRunFunction.hpp     | CR-algorithm run function       |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2009 03:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_RUN_FUNCTION_H_
# define CRALGORITHM_GENERATOR_RUN_FUNCTION_H_

class CRAlgorithmScopeRunFunction : public FunctionPTreeGenerator, public RewriteVisitor
{
public:
  CRAlgorithmScopeRunFunction(PTree::Node* block, SymbolLookup::Scope* scope)
  {
    addModifier(atom("template<class __Policy__>"));
    setReturnType(atom("void"));
    setName("run");
    addParameter(atom("__Policy__"), atom("&__policy__"));
    body.add(atom("assert(__state__ == -1);\n"));
    body.add(atom("/*"));
    body.add(rewrite(block));
    body.add(atom("*/"));
  }
  
  virtual void visit(PTree::Typedef* node)
  {
    setResult(NULL); // remove typedefs, since they are generated into the CRAlgorithmScope class.
  }
  
  // break
  virtual void visit(PTree::BreakStatement* node)
    {setResult(returnStatement(atom("cralgo::stateBreak")));}

  // continue
  virtual void visit(PTree::ContinueStatement* node)
    {setResult(returnStatement(atom("cralgo::stateContinue")));}
  
  virtual void visit(PTree::UserStatementExpr* node)
  {
    CRAlgo::ChooseExpression* choose = dynamic_cast<CRAlgo::ChooseExpression* >(node);
    if (choose)
    {
//      CRAlgorithmChoose c(choose, NULL, NULL, 0);
      setResult(funcallExpr(identifier("__policy__.choose"), choose->getArguments()));
    }
    else
      RewriteVisitor::visit(node);
  }
  
  // reward
  virtual void visit(PTree::FuncallExpr* node)
  {
    if (CRAlgo::isReward(PTree::first(node)))
      setResult(funcallExpr(identifier("__policy__.reward"), PTree::third(node)));
    else
      RewriteVisitor::visit(node);
  }
  
  // chooseFunction
  virtual void visit(PTree::UserStatement* node)
  {
    if (dynamic_cast<CRAlgo::StateFundefStatement* >(node)) 
      setResult(NULL); // remove state functions
    else
      RewriteVisitor::visit(node);
  }
  
 // local blocks
  virtual void visit(PTree::Block* node)
  {
    // do not visit
  }  
};

#endif // !CRALGORITHM_GENERATOR_RUN_FUNCTION_H_
