/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeGenerator.h    | CR-algorithm scope generator    |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2009 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_SCOPE_GENERATOR_H_
# define CRALGORITHM_SCOPE_GENERATOR_H_

# include "../language/CRAlgoPTree.h"
# include "../language/CRAlgorithmVariableTranslator.h"
# include "../language/CRAlgorithmLocalScope.h"
# include "../language/CRAlgorithmChoose.h"
# include "../tools/PTreeAnalyser.h"
# include "../tools/PTreeGenerator.h"

class CRAlgorithmScopeGenerator : public ClassPTreeGenerator
{
public:
  virtual ~CRAlgorithmScopeGenerator() {}
  
  void prepare(CRAlgorithmVariableTranslator& translator, PTree::Node* block, SymbolLookup::Scope* scope, const std::string& className);
      
  void prepare(CRAlgorithmVariableTranslator& translator, CRAlgorithmLocalScope& scope)
    {prepare(translator, const_cast<PTree::Node* >(scope.getPTree()), scope.getScope(), scope.getClassName());}
  
  virtual void customizeCopyConstructor(ConstructorPTreeGenerator& generator) {}
  virtual void customizeDefaultConstructor(ConstructorPTreeGenerator& generator) {}
  virtual void customizeDestructor(FunctionPTreeGenerator& generator) {}
  
protected:
  SymbolLookup::Scope* scope;
  
  std::vector<ParameterPTreeAnalyser> parameters;
  std::vector<ParameterPTreeAnalyser> variables;
  std::vector<ParameterPTreeAnalyser> parametersAndVariables;
  std::set<std::string> variablesInitializedByAssignment;
  
  std::vector<CRAlgorithmLocalScope> localScopes;
  std::vector<CRAlgorithmChoose> chooses;
      
  void beginPart(const char* comment, PTree::Keyword* accessKeyword)
  {
    body.addAccessSpecifier(accessKeyword);
    body.addComment(comment);
  }
  
  void endPart()
  {
    body.addNewLine();
  }
  
  void addIntrospectionMembers();
  void addLocalScopeMembers();
  void addLocalVariableMembers();
};

#endif // !CRALGORITHM_SCOPE_GENERATOR_H_
