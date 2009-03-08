/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeStepFunct..hpp | CR-algorithm step function      |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2009 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_SCOPE_STEP_FUNCTION_H_
# define CRALGORITHM_GENERATOR_SCOPE_STEP_FUNCTION_H_

class CRAlgorithmScopeStepFunction : public FunctionPTreeGenerator, public CRAlgorithmVariableTranslatorVisitor
{
public:
  CRAlgorithmScopeStepFunction(CRAlgorithmVariableTranslator& translator)
    : FunctionPTreeGenerator(), CRAlgorithmVariableTranslatorVisitor(translator),
      localScopes(NULL), chooses(NULL), variablesInitializedByAssignment(NULL), 
      rootNode(NULL), isInsideBreakableStatement(false) {}

  PTree::Node* rewrite(PTree::Node* node, SymbolLookup::Scope* scope)
  {
    singleChooseInStatement = NULL;
    isInsideBreakableStatement = false;
    rootNode = node;
    // note that the following call does both
    //    - the variable translation
    //    - the Choose/Reward transformation described in this class
    return CRAlgorithmVariableTranslatorVisitor::translateVariables(node, scope, true);
  }
  
  void prepare(PTree::Node* node, SymbolLookup::Scope* scope, const std::string& scopeClassName,
      const std::vector<CRAlgorithmLocalScope>& localScopes,
      const std::vector<CRAlgorithmChoose>& chooses,
      const std::set<std::string>& variablesInitializedByAssignment)
  {
    this->localScopes = &localScopes;
    this->chooses = &chooses;
    this->scope = scope;
    this->variablesInitializedByAssignment = &variablesInitializedByAssignment;
    
    addModifier(atom("template<class __Callback__>"));
    addModifier(staticKeyword());
    setReturnType(atom("int"));
    setName("step");
    addParameter(atom(translator.getCRAlgorithmClassName()), atom("&__crAlgorithm__"));
    addParameter(atom(scopeClassName), atom("&__crAlgorithmScope__"));
    addParameter(atom("__Callback__"), atom("&__callback__"));
    addParameter(atom("const void"), atom("*__choice__"));

    StateSwitchGenerator switchgen;
    switchgen.prepare(localScopes, chooses);
    body.add(switchgen.createStatement());
    body.add(rewrite(node, scope));
    body.add(exprStatement(atom("__crAlgorithmScope__.__state__ = " + size2str(localScopes.size() + chooses.size()))));
    body.add(returnStatement(atom("cralgo::stateFinish"))); // execution finished normally
  }
  
  virtual void visit(PTree::Typedef* node)
  {
    setResult(NULL); // remove typedefs, since they are generated into the CRAlgorithmScope class.
  }

  // reward
  virtual void visit(PTree::FuncallExpr* node)
  {
    if (CRAlgo::isReward(PTree::first(node)))
      setResult(list(identifier("__callback__.reward"), PTree::second(node), PTree::third(node), PTree::nth(node, 3)));
    else
      RewriteVisitor::visit(node);
  }
  
  // stateFunction
  virtual void visit(PTree::UserStatement* node)
  {
    if (dynamic_cast<CRAlgo::StateFundefStatement* >(node)) 
      setResult(NULL); // remove state functions
    else
      RewriteVisitor::visit(node);
  }

  // return
  virtual void visit(PTree::ReturnStatement* node)
  {
    BlockPTreeGenerator block;
    createReturnCode(PTree::second(node), block);
    setResult(block.createBlock());
  }
  
  // break
  virtual void visit(PTree::BreakStatement* node)
    {setResult(returnStatement(atom("cralgo::stateBreak")));}

  // continue
  virtual void visit(PTree::ContinueStatement* node)
    {setResult(returnStatement(atom("cralgo::stateContinue")));}
  
  // variable declaration
  virtual void visit(PTree::Declaration* node)
  {
    DeclarationPTreeAnalyser input(node);
    std::vector<ParameterPTreeAnalyser> variables = input.getVariables();
    
    BlockPTreeGenerator block;
    for (size_t i = 0; i < variables.size(); ++i)
    {
      ParameterPTreeAnalyser variable = variables[i];
      
      if (variablesInitializedByAssignment->find(variable.getIdentifierString()) != variablesInitializedByAssignment->end())
      {
          // remove variable declarations, e.g. "int i;" => ""
        PTree::Node* arg = variable.getInitializationArguments(true);
        if (arg)
        {
          // replace variable declaration by assignment, e.g. "int i = 51;" => "i = 51"
          PTree::Node* newStatement = exprStatement(assignExpr(variable.getIdentifier(), arg));
          replaceChooseParentStatementOrDeclaration(node, newStatement);
          block.add(RewriteVisitor::rewrite(newStatement));
        }
      }
    }
    if (!block.size())
      setResult(semiColonAtom());
    else
      setResult(block.createContent());
  }
  
  // local blocks
  virtual void visit(PTree::Block* node)
  {
    const CRAlgorithmLocalScope* localScope = getCRAlgorithmLocalScope(node);
    if (localScope)
    {
      BlockPTreeGenerator block;
      createLocalScopeCode(*localScope, block);
      setResult(block.createBlock());
    }
    else
    {
      assert(node == rootNode);
      RewriteVisitor::visit(node);
    }
  }

  // choose
  virtual void visit(PTree::UserStatementExpr* node)
  {
    CRAlgo::ChooseExpression* choose = dynamic_cast<CRAlgo::ChooseExpression* >(node);
    if (choose)
    {
      if (singleChooseInStatement)
      {
        assert(singleChooseInStatement->getPTree() == node);
        setResult(list(atom("*("), singleChooseInStatement->getChooseType(), atom("* )__choice__")));
      }
      else
      {
        const CRAlgorithmChoose* c = getCRAlgorithmChoose(node);
        assert(c);
        setResult(atom(c->getResultVariableName()));
      }
    }
    else
      RewriteVisitor::visit(node);
  }

  // switch
  virtual void visit(PTree::SwitchStatement* node)
  {
    bool oisInsideBreakableStatement = isInsideBreakableStatement;
    isInsideBreakableStatement = true;
    visitStatement(node);
    isInsideBreakableStatement = oisInsideBreakableStatement;
  }

  // while
  virtual void visit(PTree::WhileStatement* node)
  {
    bool oisInsideBreakableStatement = isInsideBreakableStatement;
    isInsideBreakableStatement = true;
    visitStatement(node);
    isInsideBreakableStatement = oisInsideBreakableStatement;
  }
  
  // do
  virtual void visit(PTree::DoStatement* node)
  {
    bool oisInsideBreakableStatement = isInsideBreakableStatement;
    isInsideBreakableStatement = true;
    visitStatement(node);
    isInsideBreakableStatement = oisInsideBreakableStatement;
  }

  // for
  virtual void visit(PTree::ForStatement* node)
  {
    bool oisInsideBreakableStatement = isInsideBreakableStatement;
    isInsideBreakableStatement = true;
    visitStatement(node);
    isInsideBreakableStatement = oisInsideBreakableStatement;
  }

  virtual void visit(PTree::ExprStatement* node) {visitStatement(node);}
  
  // TODO: chooses inside if/switch/while/do/for/try/cases
    
  template<class T_node>
  void visitStatement(T_node* node)
  {
    std::vector<CRAlgorithmChoose> chooses = getChoosesContainedByStatementOrDeclaration(node);
    
    if (chooses.size())
    {
      // 1) Single choose in the statement
        // code du choose
        // statement qui utilise *(type* )choice
      
      // 2) Multiple chooses in the statement
        // code du choose1
        // choice1TemporaryIdentifier <- *(type1* )choice1
        // code du choose2
        // choice2TemporaryIdentifier <- *(type2* )choice2
        // [...]
        // statement qui utilise les variables choice*TemporaryIdentifier
      
      BlockPTreeGenerator block;
      for (size_t i = 0; i < chooses.size(); ++i)
      {
        CRAlgorithmChoose& choose = chooses[i];
        createChooseCode(choose, block);
        if (chooses.size() > 1)
        {
          // Create temporary variable to store the result
          // __choose0_result__ = *(ChoiceType* )__choice__;
          block.addVariableDeclaration(choose.getChooseType(), atom(choose.getResultVariableName()),
              list( atom("*("), choose.getChooseType(), atom("* )__choice__")));
        }
        else
          singleChooseInStatement = &choose;
      }
      RewriteVisitor::visit(node);
      singleChooseInStatement = NULL;

      block.add(getResult());
      setResult(block.createContent());
    }
    else
      RewriteVisitor::visit(node);
  }
    
protected:
  size_t getFinalStateNumber() const
    {return localScopes->size() + chooses->size();}
    
  struct StateSwitchGenerator : public SwitchPTreeGenerator
  {
    void prepare(const std::vector<CRAlgorithmLocalScope>& localScopes,
                  const std::vector<CRAlgorithmChoose>& chooses)
    {
      setCondition(atom("__crAlgorithmScope__.__state__"));

      addCase(literal("-1"), breakStatement());
      size_t numLocalScopes = localScopes.size();
      for (size_t i = 0; i < numLocalScopes; ++i)
        addCase(literal(i), gotoStatement(atom(localScopes[i].getLabelName())));
      for (size_t i = 0; i < chooses.size(); ++i)
        addCase(literal(numLocalScopes + i), gotoStatement(atom(chooses[i].getLabelName())));

      setDefault(exprStatement(atom("assert(false)")));
    }
  };

  void createLocalScopeCode(const CRAlgorithmLocalScope& localScope, BlockPTreeGenerator& block)
  {
    // __crAlgorithmScope__.__localScope0__ = new __LocalScope0__();
    block.add(exprStatement(assignExpr(atom("__crAlgorithmScope__." + localScope.getVariableName()), atom("new " + localScope.getClassName() + "(__crAlgorithm__)"))));
  
    // __callback__.enterLocalScope(0, *__crAlgorithmScope__.__localScope0__);
    block.add(exprStatement(atom("__callback__.enterLocalScope(" + localScope.getName() + ", *__crAlgorithmScope__." + localScope.getVariableName() + ")")));
    
    // update __state__
    block.add(exprStatement(assignExpr(atom("__crAlgorithmScope__.__state__"), literal(localScope.getNumber()))));

    // label __localScope0_label__:
    block.add(labelStatement(atom(localScope.getLabelName())));
    
    // int __stepState__ = LocalScope0::step(__crAlgorithm__, *__crAlgorithmScope__.__localScope1__, __callback__, __choice__)
    PTree::Node* subStepFuncall = atom(localScope.getClassName() + "::step(__crAlgorithm__, *__crAlgorithmScope__." 
        + localScope.getVariableName() + ", __callback__, __choice__)");
    PTree::Node* subStepReturn = atom("__stepState__");
    block.addVariableDeclaration(intKeyword(), subStepReturn, subStepFuncall);
    
    // if (__stepState__ == cralgo::stateChoose) return cralgo::stateChoose
    block.add(ifStatement(infixExpr(subStepReturn, equalAtom(), atom("cralgo::stateChoose")),
            returnStatement(atom("cralgo::stateChoose"))));
    
    // __callback__.leaveLocalScope();
    block.add(atom("__callback__.leaveLocalScope();"));
    
    // delete __localScope0__; __localScope0__ = NULL;
    block.add(exprStatement(atom("delete __crAlgorithmScope__." + localScope.getVariableName())));
    block.add(exprStatement(atom("__crAlgorithmScope__." + localScope.getVariableName() + "= NULL")));
    
    // if (__stepState == cralgo::stateReturn) {
    //      __crAlgorithm__.__state__ = ENDSTATE;
    //      return cralgo::stateReturn; 
    // }
    ifStatement(infixExpr(subStepReturn, equalAtom(), atom("cralgo::stateReturn")),
      atom("{ __crAlgorithm__.__state__ = " + size2str(getFinalStateNumber()) + "; return cralgo::stateReturn;}"));

    if (isInsideBreakableStatement)
    {
      // if (__stepState__ == cralgo::stateBreak) break;
      // else if (__stepState__ == cralgo::stateContinue) continue;
      block.add(ifStatement(infixExpr(subStepReturn, equalAtom(), atom("cralgo::stateBreak")), 
        breakStatement(), 
        ifStatement(infixExpr(subStepReturn, equalAtom(), atom("cralgo::stateContinue")),
          continueStatement())));
    }
    else
    {
      // forward stateBreak and stateContinue
      // continue if the sub-block finished normally (stateFinish)
      
      // if (__stepState__ != cralgo::stateFinish)
      //    return __stepState__;
      block.add(ifStatement(infixExpr(subStepReturn, notEqualAtom(), atom("cralgo::stateFinish")), returnStatement(subStepReturn)));
    }
  }

  void createChooseCode(const CRAlgorithmChoose& choose, BlockPTreeGenerator& block)
  {
    // __callback__.choose(*(__Choose0Parameters__)0, choices)
    FuncallPTreeGenerator funcall;
    funcall.setName("__callback__.choose");
    funcall.addArgument(RewriteVisitor::rewrite(choose.getChoiceNode()));
    funcall.addArgument(list(atom("*(" + choose.getParametersClassName() + "* )0")));
    block.addExpressionStatement(funcall.createExpression());
    
    // update __state__
    block.add(exprStatement(assignExpr(atom("__crAlgorithmScope__.__state__"), literal(localScopes->size() + choose.getChooseNumber()))));

    // return
    block.add(returnStatement(atom("cralgo::stateChoose")));
    
    // label
    block.add(labelStatement(atom(choose.getLabelName())));

    // assert that a choice has been performed
    block.add(exprStatement(atom("assert(__choice__)")));  
  }

  void createReturnCode(PTree::Node* returnArgument, BlockPTreeGenerator& block)
  {
    if (*returnArgument != ";")
    {
      bool isSimpleIdentifier = dynamic_cast<PTree::Identifier* >(returnArgument) != NULL;
      returnArgument = RewriteVisitor::rewrite(returnArgument);
      if (isSimpleIdentifier)
      {
        // return res; => __crAlgorithm__.__return__ = &__crAlgorithm__.res; __crAlgorithm__.__isReturnOwner__ = false;
        
        block.add(exprStatement(assignExpr(atom("__crAlgorithm__.__return__"),
          PTree::list(atom("&"), returnArgument))));
        block.add(exprStatement(assignExpr(atom("__crAlgorithm__.__isReturnOwner__"), atom("false"))));
      }
      else
      {    
        // return 51; => __crAlgorithm__.__return__ = new int(51); __crAlgorithm__.__isReturnOwner__ = true;
        block.add(exprStatement(assignExpr(atom("__crAlgorithm__.__return__"),
          PTree::list(atom("new"), translator.getCRAlgorithmReturnType(), atom("("), returnArgument, atom(")")))));
        block.add(exprStatement(assignExpr(atom("__crAlgorithm__.__isReturnOwner__"), atom("true"))));
      }
    }
    
    // final state, return false
    block.add(exprStatement(atom("__crAlgorithmScope__.__state__ = " + size2str(getFinalStateNumber()))));
    block.add(returnStatement(atom("cralgo::stateReturn")));
  }

private:
  const std::vector<CRAlgorithmLocalScope>* localScopes;
  const std::vector<CRAlgorithmChoose>* chooses;
  const std::set<std::string>* variablesInitializedByAssignment;
  CRAlgorithmChoose* singleChooseInStatement; // NULL if multiple chooses in statement, the current choose otherwise
  PTree::Node* rootNode;
  SymbolLookup::Scope* scope;
  bool isInsideBreakableStatement;
  
  const CRAlgorithmLocalScope* getCRAlgorithmLocalScope(PTree::Node* block) const
  {
    assert(localScopes);
    for (size_t i = 0; i < localScopes->size(); ++i)
      if ((*localScopes)[i].getPTree() == block)
        return &((*localScopes)[i]);
    return NULL;
  }
  
  const CRAlgorithmChoose* getCRAlgorithmChoose(PTree::UserStatementExpr* node) const
  {
    assert(chooses);
    for (size_t i = 0; i < chooses->size(); ++i)
      if ((*chooses)[i].getPTree() == node)
        return &((*chooses)[i]);
    return NULL;
  }
  
  std::vector<CRAlgorithmChoose> getChoosesContainedByStatementOrDeclaration(PTree::Node* statement) const
  {
    std::vector<CRAlgorithmChoose> res;
    for (size_t i = 0; i < chooses->size(); ++i)
      if ((*chooses)[i].getParentStatementOrDeclaration() == statement)
        res.push_back((*chooses)[i]);
    return res;
  }
  
  void replaceChooseParentStatementOrDeclaration(PTree::Node* oldNode, PTree::Node* newNode)
  {
    for (size_t i = 0; i < chooses->size(); ++i)
      if ((*chooses)[i].getParentStatementOrDeclaration() == oldNode)
        (*const_cast<std::vector<CRAlgorithmChoose>* >(chooses))[i].setParentStatementOrDeclaration(newNode);
  }

};

#endif // !CRALGORITHM_GENERATOR_SCOPE_STEP_FUNCTION_H_
