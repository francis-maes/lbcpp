/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmVariableTransl...cpp| CR-algorithm variable translator|
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 22:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CRAlgorithmVariableTranslator.h"
#include "../tools/ErrorManager.h"
#include "../tools/PTreeAnalyser.h"

/*
** CRAlgorithmVariableTranslator
*/
CRAlgorithmVariableTranslator::CRAlgorithmVariableTranslator(SymbolLookup::FunctionScope* crAlgorithmScope, const std::string& className, PTree::Node* returnType)
  : crAlgorithmScope(crAlgorithmScope), className(className), returnType(returnType)
{
}

PTree::Node* CRAlgorithmVariableTranslator::translate(PTree::Identifier* id, SymbolLookup::Scope* currentScope, bool translateVariablesInsideCurrentScope, const char* crAlgoPrefix)
{
  const SymbolLookup::Symbol* symbol = simpleSymbolLookup(currentScope, id);
  if (!symbol || !dynamic_cast<const SymbolLookup::VariableName* >(symbol))
    return id; // only translate variable names
    
  SymbolLookup::Scope* scope = symbol->scope();
  if (!translateVariablesInsideCurrentScope && isSubScope(currentScope, scope))
  {
    //std::cout << PTree::reify(id) << " = Local variable -> unchanged." << std::endl;
    return id;
  }
  else if (isSubScope(crAlgorithmScope, scope))
  {
    // create path from currentScope to crAlgorithmScope
    std::string res = PTree::reify(id);

    const SymbolLookup::Scope* s = scope;
    while (s != crAlgorithmScope)
    {
      assert(s);
      if (dynamic_cast<const SymbolLookup::LocalScope* >(s))
      {
        ScopeVariablesMap::iterator it = scopeVariables.find(s);
        assert(it != scopeVariables.end());
        res = it->second + "->" + res;
      }
      else
      {
        ErrorManager::getInstance().addError("Internal error, unsupported kind of scopes");
      }
      s = s->outer_scope();
    }
    res = crAlgoPrefix + res;
    
    //std::cout << PTree::reify(id) << " = CRAlgorithm variable -> changed to " << res << std::endl;
    return identifier(res);
  }
  else
  {
    // std::cout << PTree::reify(id) << " = Global variable -> unchanged." << std::endl;
    return id;
  }
}

/*
** CRAlgorithmVariableTranslatorVisitor
*/
CRAlgorithmVariableTranslatorVisitor::CRAlgorithmVariableTranslatorVisitor(CRAlgorithmVariableTranslator& translator)
  : translator(translator), enableTranslation(true), currentScope(NULL), translateVariablesInsideCurrentScope(false), crAlgoPrefix(NULL) {}

PTree::Node* CRAlgorithmVariableTranslatorVisitor::translateVariables(PTree::Node* node, SymbolLookup::Scope* currentScope,
                              bool translateVariablesInsideCurrentScope, const char* crAlgoPrefix)
{
  this->currentScope = currentScope;
  this->translateVariablesInsideCurrentScope = translateVariablesInsideCurrentScope;
  this->crAlgoPrefix = crAlgoPrefix;
  PTree::Node* res = RewriteVisitor::rewrite(node);
  this->crAlgoPrefix = NULL;
  this->currentScope = NULL;
  return res;
}

//. [postfix].[name] => rewrite([postfix]).[name]
void CRAlgorithmVariableTranslatorVisitor::visit(PTree::DotMemberExpr* expr)
{
  setResult(new PTree::DotMemberExpr(rewrite(PTree::first(expr)),
      PTree::list(rewriteWithoutTranslation(PTree::second(expr)),
                  rewriteWithoutTranslation(PTree::third(expr)))));
}

//. [postfix]->[name] => rewrite([postfix])->[name]
void CRAlgorithmVariableTranslatorVisitor::visit(PTree::ArrowMemberExpr* expr)
{
  setResult(new PTree::ArrowMemberExpr(rewrite(PTree::first(expr)),
      PTree::list(rewriteWithoutTranslation(PTree::second(expr)),
                  rewriteWithoutTranslation(PTree::third(expr)))));
}

void CRAlgorithmVariableTranslatorVisitor::visit(PTree::Identifier* node)
{
  if (enableTranslation)
  {
    assert(currentScope && crAlgoPrefix);
    setResult(translator.translate(node, currentScope, translateVariablesInsideCurrentScope, crAlgoPrefix));
  }
  else
    setResult(node);
}

PTree::Node* CRAlgorithmVariableTranslatorVisitor::rewriteWithoutTranslation(PTree::Node* node)
{
  bool oEnableTranslation = enableTranslation;
  enableTranslation = false;
  PTree::Node* res = rewrite(node);
  enableTranslation = oEnableTranslation;
  return res;
}
