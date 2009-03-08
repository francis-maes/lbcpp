/*-----------------------------------------.---------------------------------.
| Filename: CheckCRAlgoSyntaxVisitor.h     | Check syntax visitor            |
| Author  : Francis Maes                   |                                 |
| Started : 11/02/2009 15:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_COMPILER_CHECK_CR_ALGO_SYNTAX_VISITOR_H_
# define CRALGORITHM_COMPILER_CHECK_CR_ALGO_SYNTAX_VISITOR_H_

# include "../tools/RecursiveVisitor.h"
# include "../tools/PTreeAnalyser.h"

#if 0
class CRAlgoVisitor : public PTree::Visitor
{
public:
  virtual void visit(CRAlgo::StateFundefStatement* node) {}
  virtual void visit(CRAlgo::FeatureScopeStatement* node) {}
  virtual void visit(CRAlgo::FeatureGeneratorCallStatement* node) {}

  virtual void visitFeatureSense(PTree::FuncallExpr* node) {}
  virtual void visitReward(PTree::FuncallExpr* node) {}
  
  virtual void visit(CRAlgo::CRAlgorithmCallExpr* node) {}
  
  virtual void visitFeatureGenerator(PTree::FunctionDefinition* node) {}
  virtual void visitCRAlgorithm(PTree::FunctionDefinition* node) {}

  // stateFunction, featureScope, featureCall
  virtual void visit(PTree::UserStatement* node)
  {
    CRAlgo::StateFundefStatement* stateFunction = dynamic_cast<CRAlgo::StateFundefStatement* >(node);
    if (stateFunction)
    {
      visitStateFunction(stateFunction);
      return;
    }
    CRAlgo::FeatureScopeStatement* featureScope = dynamic_cast<CRAlgo::FeatureScopeStatement* >(node);
    if (featureScope)
    {
      visitFeatureScope(featureScope);
      return;
    }
    CRAlgo::FeatureGeneratorCallStatement* featureGeneratorCall = dynamic_cast<CRAlgo::FeatureGeneratorCallStatement* >(node);
    if (featureGeneratorCall)
    {
      visitFeatureGeneratorCall(featureGeneratorCall);
      return;
    }
    PTree::Visitor::visit(node);
  }

  //  reward, featureSense
  virtual void visit(PTree::FuncallExpr* node)
  {
    PTree::Node* identifier = PTree::first(node);
    if (CRAlgo::isReward(identifier))
    {
      visitReward(node);
      return;
    }
    if (CRAlgo::isFeatureSense(identifier))
    {
      visitFeatureSense(node);
      return;
    }
    PTree::Visitor::visit(node);
  }

  // crAlgorithmCall
  virtual void visit(PTree::UserStatementExpr* node)
  {
    CRAlgo::CRAlgorithmCallExpr* crAlgorithmCall = dynamic_cast<CRAlgo::CRAlgorithmCallExpr* >(node);
    if (crAlgorithmCall)
    {
      visitCRAlgorithmCall(crAlgorithmCall);
      return;
    }
    CRAlgo::ChooseExpression* chooseExpr = dynamic_cast<CRAlgo::ChooseExpression* >(node);
    {
      visitChoose(chooseExpr);
      return;
    }
    PTree::Visitor::visit(node);    
  }

  //  CR-algorithm, featureGenerator or normal function
  virtual void visit(PTree::FunctionDefinition* node)
  {
    if (CRAlgo::isCRAlgorithm(node))
      visitCRAlgorithm(node);
    else if (CRAlgo::isFeatureGenerator(node))
      visitFeatureGenerator(node);
    else
      PTree::Visitor::visit(node);
  }
};

#endif // 0

class CheckCRAlgoSyntaxVisitor : public RecursiveVisitor
{
public:
  bool checkSyntax(PTree::Node* node)
  {
    isSyntaxCorrect = true;
    
    isCurrentlyInCRAlgorithm = false;
    isCurrentlyInFeatureGenerator = false;
    
    visitRecursively(node);
    return isSyntaxCorrect;
  }
    
  /*
  ** Feature Generators
  */
  void visitFeatureGenerator(PTree::FunctionDefinition* node)
  {
    FunctionPTreeAnalyser input(node);
    if (input.getModifiers().contains("inline"))
      addWarning("All featureGenerators are automatically inline functions; remove the 'inline' keyword", node);

    if (isCurrentlyInFeatureGenerator)
    {
      addError("Cannot declare a featureGenerator inside a featureGenerator", node);
      RecursiveVisitor::visit(node);
    }
    else
    {
      isCurrentlyInFeatureGenerator = true;
      RecursiveVisitor::visit(node);
      isCurrentlyInFeatureGenerator = false;
    }
  }
  
  void visitFeatureSense(PTree::FuncallExpr* node)
    {checkInsideFeatureGenerator("featureSense", node);}

  void visitFeatureScope(CRAlgo::FeatureScopeStatement* node)
    {checkInsideFeatureGenerator("featureScope", node);}

  void visitFeatureGeneratorCall(CRAlgo::FeatureGeneratorCallStatement* node)
  {
    checkInsideFeatureGenerator("featureCall", node);
    if (node->isInlineCall() && !node->isFunctionCall())
      addError("Only function call expressions can be inlined", node);
  }


  /*
  ** CR-algorithms
  */
  void visitCRAlgorithm(PTree::FunctionDefinition* node)
  {
    // todo: check forbidden keywords (e.g. virtual, featureGenerator, actionValue ...)
    
    if (isCurrentlyInCRAlgorithm)
    {
      addError("Cannot declare a CR-algorithm inside a CR-algorithm", node);
      RecursiveVisitor::visit(node);
    }
    else
    {
      isCurrentlyInCRAlgorithm = true;
      RecursiveVisitor::visit(node);
      isCurrentlyInCRAlgorithm = false;
    }
  }
  
  void visitReward(PTree::FuncallExpr* node)
    {}//checkInsideCRAlgorithm("reward", node);}
    
  void visitChoose(CRAlgo::ChooseExpression* node)
    {checkInsideCRAlgorithm("choose", node);}
  
  void visitStateFunction(CRAlgo::StateFundefStatement* node)
    {checkInsideCRAlgorithm("actionValue", node);}
  
  void visitCRAlgorithmCall(CRAlgo::CRAlgorithmCallExpr* node)
    {checkInsideCRAlgorithm("crAlgorithmCall", node);}
  
  virtual void visit(PTree::Identifier* node)
  {
    std::string id = PTree::reify(node);
    static const char* forbiddenIdentifiers[] = {
      "__crAlgorithm__", "__state__", "__choice__", "__policy__", "__returnType__",
      "__Param Type__", "__localScope __", "__localScope label__", "__choose _label__", "__choose _result__",
      "__crAlgorithmScope__", "__isReturnOwner__", "__return__", "__stepState__", 
      "__this__", "__featureVisitor__"
    };
    
    for (size_t i = 0; i < sizeof (forbiddenIdentifiers) / sizeof (const char* ); ++i)
      if (matchForbiddenIdentifier(id, forbiddenIdentifiers[i]))
      {
        addError("The identifier '" + id + "' is reserved for generated code", node);
        break;
      }
  }
    
protected:
  void addError(const std::string& message, PTree::Node* node)
    {ErrorManager::getInstance().addError(message, node); isSyntaxCorrect = false;}

  void addWarning(const std::string& message, PTree::Node* node)
    {ErrorManager::getInstance().addWarning(message, node);}

  void addContextError(const std::string& what, const std::string& shouldBeWhere, const std::string& missingKeyword, PTree::Node* node)
    {addError("'" + what + "' can only be used inside " + shouldBeWhere + "; use the '" + missingKeyword + "' keyword to declare " + shouldBeWhere, node);}
  
  bool checkInsideFeatureGenerator(const std::string& what, PTree::Node* node)
  {
    if (!isCurrentlyInFeatureGenerator)
    {
      addContextError(what, "feature generators", "featureGenerator", node);
      return false;
    }
    return true;
  }
  
  bool checkInsideCRAlgorithm(const std::string& what, PTree::Node* node)
  {
    if (!isCurrentlyInCRAlgorithm)
    {
      addContextError(what, "CR-algorithms", "crAlgorithm", node);
      return false;
    }
    return true;
  }
  
  bool matchForbiddenIdentifier(const std::string& id, const std::string& forbidden)
  {
    size_t i = 0;
    for (size_t j = 0; j < forbidden.size(); ++j)
    {
      if (i == std::string::npos || i >= id.length())
        return false;
      if (forbidden[j] == ' ')
        i = id.find_first_not_of("0123456789");
      else
      {
        if (id[i] != forbidden[j])
          return false;
        ++i;
      }
    }
    return i == id.length();
  }
  
protected:
  // stateFunction, featureScope, featureCall
  virtual void visit(PTree::UserStatement* node)
  {
    CRAlgo::StateFundefStatement* stateFunction = dynamic_cast<CRAlgo::StateFundefStatement* >(node);
    if (stateFunction)
    {
      visitStateFunction(stateFunction);
      return;
    }
    CRAlgo::FeatureScopeStatement* featureScope = dynamic_cast<CRAlgo::FeatureScopeStatement* >(node);
    if (featureScope)
    {
      visitFeatureScope(featureScope);
      return;
    }
    CRAlgo::FeatureGeneratorCallStatement* featureGeneratorCall = dynamic_cast<CRAlgo::FeatureGeneratorCallStatement* >(node);
    if (featureGeneratorCall)
    {
      visitFeatureGeneratorCall(featureGeneratorCall);
      return;
    }
    RecursiveVisitor::visit(node);
  }

  //  reward, featureSense
  virtual void visit(PTree::FuncallExpr* node)
  {
    PTree::Node* identifier = PTree::first(node);
    if (CRAlgo::isReward(identifier))
    {
      visitReward(node);
      return;
    }
    if (CRAlgo::isFeatureSense(identifier))
    {
      visitFeatureSense(node);
      return;
    }
    RecursiveVisitor::visit(node);
  }

  // crAlgorithmCall
  virtual void visit(PTree::UserStatementExpr* node)
  {
    CRAlgo::CRAlgorithmCallExpr* crAlgorithmCall = dynamic_cast<CRAlgo::CRAlgorithmCallExpr* >(node);
    if (crAlgorithmCall)
    {
      visitCRAlgorithmCall(crAlgorithmCall);
      return;
    }
    CRAlgo::ChooseExpression* chooseExpr = dynamic_cast<CRAlgo::ChooseExpression* >(node);
    {
      visitChoose(chooseExpr);
      return;
    }
    RecursiveVisitor::visit(node);    
  }

  //  CR-algorithm, featureGenerator or normal function
  virtual void visit(PTree::FunctionDefinition* node)
  {
    if (CRAlgo::isCRAlgorithm(node))
      visitCRAlgorithm(node);
    else if (CRAlgo::isFeatureGenerator(node))
      visitFeatureGenerator(node);
    else
      RecursiveVisitor::visit(node);
  }

private:
  bool isSyntaxCorrect;
  
  bool isCurrentlyInCRAlgorithm;
  bool isCurrentlyInFeatureGenerator;
};

#endif // !CRALGORITHM_COMPILER_CHECK_CR_ALGO_SYNTAX_VISITOR_H_

