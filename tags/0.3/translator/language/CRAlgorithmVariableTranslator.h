/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmVariableTranslator.h| CR-algorithm variable translator|
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2009 14:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_VARIABLE_TRANSLATOR_H_
# define CRALGORITHM_GENERATOR_VARIABLE_TRANSLATOR_H_

# include "../tools/PTreeGenerator.h"
# include "../tools/RewriteVisitor.h"

class CRAlgorithmVariableTranslator : public PTreeGenerator
{
public:
  CRAlgorithmVariableTranslator(SymbolLookup::FunctionScope* crAlgorithmScope, const std::string& className, PTree::Node* returnType);
  
  PTree::Node* translate(PTree::Identifier* id, SymbolLookup::Scope* currentScope, bool translateVariablesInsideCurrentScope, const char* crAlgoPrefix = "__crAlgorithm__.");
  
  SymbolLookup::FunctionScope* getCRAlgorithmScope() const
    {return crAlgorithmScope;}

  std::string getCRAlgorithmClassName() const
    {return className;}
  
  PTree::Node* getCRAlgorithmReturnType() const
    {return returnType;}
    
  bool hasCRAlgorithmReturn() const
    {return PTree::reify(returnType) != "void";}
  
  void addLocalScopeVariable(const SymbolLookup::Scope* scope, const std::string& variableName)
    {scopeVariables[scope] = variableName;}    
 
private:
  SymbolLookup::FunctionScope* crAlgorithmScope;
  std::string className;
  PTree::Node* returnType;
  
  typedef std::map<const SymbolLookup::Scope* , std::string> ScopeVariablesMap;
  ScopeVariablesMap scopeVariables;
};

class CRAlgorithmVariableTranslatorVisitor : public RewriteVisitor
{
public:
  CRAlgorithmVariableTranslatorVisitor(CRAlgorithmVariableTranslator& translator);
  
  PTree::Node* translateVariables(PTree::Node* node, SymbolLookup::Scope* currentScope,
                                bool translateVariablesInsideCurrentScope, const char* crAlgoPrefix = "__crAlgorithm__.");
  
  PTree::Node* rewriteWithoutTranslation(PTree::Node* node);

  /*
  ** RewriteVisitor
  */
  virtual void visit(PTree::DotMemberExpr* expr);
  virtual void visit(PTree::ArrowMemberExpr* expr);
  virtual void visit(PTree::Identifier* node);
  
protected:
  CRAlgorithmVariableTranslator& translator;
  bool enableTranslation;
  
  SymbolLookup::Scope* currentScope;
  bool translateVariablesInsideCurrentScope;
  const char* crAlgoPrefix;
};

#endif // !CRALGORITHM_GENERATOR_VARIABLE_TRANSLATOR_H_
