/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeEnumerator.hpp | CR-algorithm scope enumerator   |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2009 19:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_SCOPE_ENUMERATOR_HPP_
# define CRALGORITHM_GENERATOR_SCOPE_ENUMERATOR_HPP_

# include "../tools/RecursiveVisitor.h"
# include "../tools/PTreeAnalyser.h"

class CRAlgorithmScopeEnumerator : public RecursiveVisitor
{
public:
  struct EnumerateReferencedIdentifiers : public RecursiveVisitor
  {
    virtual void visit(PTree::DotMemberExpr* expr)
      {visitRecursively(PTree::first(expr));}
      
    virtual void visit(PTree::ArrowMemberExpr* expr)
      {visitRecursively(PTree::first(expr));}
      
    virtual void visit(PTree::Identifier* node)
      {res.insert(PTree::reify(node));}

    std::set<std::string> res;
  };

  CRAlgorithmScopeEnumerator()
    : chooses(NULL), typedefs(NULL), variables(NULL), variablesInitializedByAssignment(NULL), scope(NULL),
      currentStatementOrDeclaration(NULL), currentVariableDeclaration(NULL), rootNode(NULL) {}

  void enumerate(PTree::Node* node, SymbolLookup::Scope* scope,
          std::vector<CRAlgorithmChoose>& chooses,
          std::vector<PTree::Node* >& typedefs,
          std::vector<ParameterPTreeAnalyser>& variables,
          std::set<std::string>& variablesInitializedByAssignment)
  {
    currentStatementOrDeclaration = NULL;
    currentVariableDeclaration = NULL;
    rootNode = node;
    this->scope = scope;
    this->chooses = &chooses;
    this->typedefs = &typedefs;
    this->variables = &variables;
    this->variablesInitializedByAssignment = &variablesInitializedByAssignment;
    visitRecursively(node);
    this->chooses = NULL;
    this->typedefs = NULL;
    rootNode = node;
  }
  
  // typedef
  virtual void visit(PTree::Typedef* node)
  {
    assert(typedefs);
    typedefs->push_back(node);
    RecursiveVisitor::visit(node);
  }
  
  // choose
  virtual void visit(PTree::UserStatementExpr* node)
  {
    CRAlgo::ChooseExpression* choose = dynamic_cast<CRAlgo::ChooseExpression*>(node);
    if (choose)
    {
      assert(chooses);
      chooses->push_back(CRAlgorithmChoose(choose, scope, currentStatementOrDeclaration, chooses->size()));
      if (currentVariableDeclaration)
      {
        // a variable that relies on a choose cannot be initialized in the constructor
        // for example: int i = choose<int>(...); => i cannot be initialized in the constructor
        variablesInitializedByAssignment->insert(currentVariableDeclaration->getIdentifierString());
      }
    }
    RecursiveVisitor::visit(node);
  }
  
  // chooseFunction
  virtual void visit(PTree::UserStatement* node)
  {
    if (dynamic_cast<CRAlgo::StateFundefStatement* >(node)) 
      return; // don't visit state functions
    else
      RecursiveVisitor::visit(node);
  }
  
  // block
  virtual void visit(PTree::Block* node)
  {
    if (node == rootNode)
      RecursiveVisitor::visit(node);

    /* !! don't go inside block */
  }
  
  // variable declaration
  virtual void visit(PTree::Declaration* node)
  {
    DeclarationPTreeAnalyser input(node);
    std::vector<ParameterPTreeAnalyser> variables = input.getVariables();
    ParameterPTreeAnalyser* previousVariableDeclaration = currentVariableDeclaration;
    PTree::Node* previousStatementOrDeclaration = currentStatementOrDeclaration;
    for (size_t i = 0; i < variables.size(); ++i)
    {
      ParameterPTreeAnalyser& var = variables[i];
      
      bool initializeByAssignment = false;
      if (currentStatementOrDeclaration)
      {
        // a variable declared into a statement should not be initialized in the constructor
        // example: for(size_t i = 0; i < 10; ++i) for(size_t j = 0; j < 10; ++j) => j cannot be initialized in the constructor
        // here, the rule is less precise and could be improved: all the declarations that appear inside a statement are flaged
        initializeByAssignment = true;
      }
      else
      {
        /// initialize by assignment if the declarator depends on another variable initialized by assignment
        PTree::Node* declaratorTail = var.getDeclarator()->cdr();
        if (declaratorTail)
        {
          EnumerateReferencedIdentifiers v;
          v.visitRecursively(declaratorTail);
          for (std::set<std::string>::const_iterator it = v.res.begin(); it != v.res.end(); ++it)
            if (variablesInitializedByAssignment->find(*it) != variablesInitializedByAssignment->end())
            {
              initializeByAssignment = true;
              break;
            }
        }
      }
      if (initializeByAssignment)
        variablesInitializedByAssignment->insert(var.getIdentifierString());
              
      currentVariableDeclaration = &var;
      currentStatementOrDeclaration = node;
      visitRecursively(var.getDeclarator());
      currentVariableDeclaration = previousVariableDeclaration;
      currentStatementOrDeclaration = previousStatementOrDeclaration;

      this->variables->push_back(var);
    }
  }
  
  // statements
  virtual void visit(PTree::IfStatement* node) {visitStatement(node);}
  virtual void visit(PTree::SwitchStatement* node) {visitStatement(node);}
  virtual void visit(PTree::WhileStatement* node) {visitStatement(node);}
  virtual void visit(PTree::DoStatement* node) {visitStatement(node);}
  virtual void visit(PTree::ForStatement* node) {visitStatement(node);}
  virtual void visit(PTree::TryStatement* node) {visitStatement(node);}
  virtual void visit(PTree::BreakStatement* node) {visitStatement(node);}
  virtual void visit(PTree::ContinueStatement* node) {visitStatement(node);}
  virtual void visit(PTree::ReturnStatement* node) {visitStatement(node);}
  virtual void visit(PTree::GotoStatement* node) {visitStatement(node);}
  virtual void visit(PTree::CaseStatement* node) {visitStatement(node);}
  virtual void visit(PTree::DefaultStatement* node) {visitStatement(node);}
  virtual void visit(PTree::LabelStatement* node) {visitStatement(node);}
  virtual void visit(PTree::ExprStatement* node) {visitStatement(node);}
  
  template<class T_node>
  void visitStatement(T_node* node)
  {
    PTree::Node* previousStatementOrDeclaration = currentStatementOrDeclaration;
    currentStatementOrDeclaration = node;
    RecursiveVisitor::visit(node);
    currentStatementOrDeclaration = previousStatementOrDeclaration;
  }

private:
  std::vector<CRAlgorithmChoose>* chooses;
  std::vector<PTree::Node* >* typedefs;
  std::vector<ParameterPTreeAnalyser>* variables;
  
  /*
  ** Some variables can be initialized in the constructor of a cr-algo class
  **  while some others must be initialized by a classical assignment
  **    (which makes it mendatory for the user to provide default constructors for these variables)
  ** Variables must be initialized by assignment in the following cases:
  **   - If a choose is used into the declaration of the variable
  **   - If the declaration of the variables relies on another variable initialized by assignment
  **   - If the variable is used to initialize a "for" loop nested in another "for" ... (this can be improved, see visit(PTree::Declaration*) )
  */
  std::set<std::string>* variablesInitializedByAssignment;
  
  SymbolLookup::Scope* scope;
  PTree::Node* currentStatementOrDeclaration;
  ParameterPTreeAnalyser* currentVariableDeclaration;
  PTree::Node* rootNode;
};

#endif // !CRALGORITHM_GENERATOR_SCOPE_ENUMERATOR_HPP_
