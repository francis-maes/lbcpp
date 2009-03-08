/*-----------------------------------------.---------------------------------.
| Filename: CRAlgoCompilerRewriteVisitor.h | CR-Algorithm Compiler           |
| Author  : Francis Maes                   |    top-level rewrite visitor    |
| Started : 10/02/2009 18:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_COMPILER_REWRITE_VISITOR_H_
# define CRALGO_COMPILER_REWRITE_VISITOR_H_

# include "../tools/PTreeGenerator.h"
# include "../tools/ScopeBasedRewriteVisitor.h"
# include "../language/CRAlgoPTree.h"
# include "../generator/FeatureGeneratorClassGenerator.h"
# include "../generator/CRAlgorithmGenerator.h"

class CRAlgoCompilerFirstPassRewriteVisitor : public RewriteVisitor
{
public: 
  // crAlgorithm<id> => idCRAlgorithm
  virtual void visit(PTree::List* node)
  {
    std::string crAlgorithmId;
    if (CRAlgo::isCRAlgorithmType(node, &crAlgorithmId))
    {
      PTreeGenerator gen;
      setResult(gen.atom(crAlgorithmId + "CRAlgorithm"));
    }
    else
      RewriteVisitor::visit(node);
  }
};

class CRAlgoCompilerRewriteVisitor : public ScopeBasedRewriteVisitor, public PTreeGenerator
{
public:
  CRAlgoCompilerRewriteVisitor(SymbolLookup::Scope* globalScope, bool verbose)
    : ScopeBasedRewriteVisitor(globalScope), verbose(verbose) {}

  // translate CR-algorithm, featureGenerator or normal function
  virtual void visit(PTree::FunctionDefinition* node)
  {
    pushScope(getCurrentScope()->find_scope(node));        
    if (CRAlgo::isCRAlgorithm(node))
      visitCRAlgorithmDefinition(node);
    else if (CRAlgo::isFeatureGenerator(node))
      visitFeatureGeneratorDefinition(node);
    else
      RewriteVisitor::visit(node);
    popScope();
  }
  
  // translate CR-algorithm definition
  void visitCRAlgorithmDefinition(PTree::FunctionDefinition* node)
  {
    SymbolLookup::Scope* scope = getCurrentScope();
    if (!scope)
    {
      std::cerr << "BUG in '" << PTree::reify(node) << "'" << std::endl;
      PTree::display(node, std::cerr, false, true);
      assert(false);
    }
    if (verbose)
      std::cout << "Rewriting CR-algorithm " << PTree::reify(PTree::first(PTree::third(node))) << std::endl;
    
    SymbolLookup::FunctionScope* crAlgoScope = dynamic_cast<SymbolLookup::FunctionScope* >(scope);
    assert(crAlgoScope);
    
    // references to pointers
    //node = dynamic_cast<PTree::FunctionDefinition* >
    //  (ReferenceToPointerRewriteVisitor().transformReferencesIntoPointers(node, crAlgoScope));
    
    setResult(CRAlgorithmGenerator(node, crAlgoScope).createCode());
  }
  
  // translate feature generator definition
  void visitFeatureGeneratorDefinition(PTree::FunctionDefinition* node)
  {
      if (verbose)
        std::cout << "Rewriting FeatureGenerator " << PTree::reify(PTree::first(PTree::third(node))) << std::endl;

    FeatureGeneratorClassGenerator featureGenerator(node, getCurrentScope(), false);
    setResult(featureGenerator.createCode());

    // old
    //OldFeatureGeneratorRewriteVisitor visitor;
    //setResult(visitor.rewrite(node));
  }
  
protected:
  bool verbose;
};

#endif // !CRALGO_COMPILER_REWRITE_VISITOR_H_
