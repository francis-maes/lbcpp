/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmChoose.cpp          | CR-algorithm choose             |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 22:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CRAlgorithmChoose.h"
#include "../tools/RewriteVisitor.h"
#include "../tools/ErrorManager.h"
    
CRAlgorithmChoose::CRAlgorithmChoose(CRAlgo::ChooseExpression* node, SymbolLookup::Scope* scope, PTree::Node* parentStatementOrDeclaration, size_t number)
  : node(node), scope(scope), parentStatementOrDeclaration(parentStatementOrDeclaration), number(number), choiceNode(NULL)
{
  bool choicesAlreadyDeclared = false;

  std::vector<PTree::Node* > arguments = getArguments();
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    PTree::Node* argument = arguments[i];
    std::string id = PTree::reify(argument);
    CRAlgo::StateFundefStatement* stateFunction = findStateFunctionRecursively(id, scope);
    if (stateFunction)
      stateFunctions.push_back(std::make_pair(argument, stateFunction));
    else
    {
      if (choicesAlreadyDeclared)
        ErrorManager::getInstance().addError("Invalid parameter in choice: '" + id + "'", node);
      else
      {
        choiceNode = argument;
        choicesAlreadyDeclared = true;
      }
    }
  }
  if (!choicesAlreadyDeclared)
    ErrorManager::getInstance().addError("Missing choices in choose", node);

  if (!stateFunctions.size())
    ErrorManager::getInstance().addWarning("No arguments in choose", node);
}

std::vector<PTree::Node* > CRAlgorithmChoose::getArguments() const
{
  std::vector<PTree::Node* > res;
  PTree::Node* arguments = PTree::nth(node, 5);
  for (PTree::Iterator it(arguments); !it.empty(); ++it)
    if (**it != ",")
      res.push_back(*it);
  return res;
}

CRAlgo::StateFundefStatement*
CRAlgorithmChoose::findStateFunctionRecursively(const std::string& identifier, const SymbolLookup::Scope* scope)
{
  if (!scope)
    return NULL;
  for (SymbolLookup::Scope::scope_iterator it = scope->scopes_begin(); it != scope->scopes_end(); ++it)
  {
    const CRAlgo::StateFundefStatement* st = dynamic_cast<const CRAlgo::StateFundefStatement* >(it->first);
    if (st && st->getIdentifier() == identifier.c_str())
    {
//      std::cout << "'" << identifier << "' is a '" << PTree::reify(PTree::first(st)) << "'." << std::endl;
      return const_cast<CRAlgo::StateFundefStatement* >(st);
    }
  }
  return findStateFunctionRecursively(identifier, scope->outer_scope());
}    


/*
** TypeEvaluator, todo: move in tools/
*/
#include "../tools/RecursiveVisitor.h"

class TypeEvaluator : public RecursiveVisitor
{
public:
  TypeEvaluator() : scope(NULL) {}
  
  std::string evaluate(PTree::Node* node, SymbolLookup::Scope* scope)
  {
    res = "";
    this->scope = scope;
    visitRecursively(node);
    return res;
  }
  
  virtual void visit(PTree::Identifier* node)
  {
    PTree::Encoding name = PTree::Encoding::simple_name(node);
    SymbolLookup::SymbolSet symbols = scope->lookup(name);
    if (symbols.size() == 1)
    {
      SymbolLookup::Symbol const *symbol = *symbols.begin();
      SymbolLookup::VariableName const* variable = dynamic_cast<SymbolLookup::VariableName const *>(symbol);
      if (variable)
        setResult(variable->type(), variable->scope());
      else
        ErrorManager::getInstance().addError("Could not find definition of variable '" + name.unmangled() + "'", node);
    }
  }
  
protected:
  void setResult(const PTree::Encoding& encoding, SymbolLookup::Scope* scope)
    {res = encoding.unmangled();}
  
private:
  SymbolLookup::Scope* scope;
  std::string res;
};

std::string CRAlgorithmChoose::getContainerType() const
{
  TypeEvaluator evaluator;
  std::string res = evaluator.evaluate(choiceNode, scope);
  //std::cout << "Typeof() = " << res << std::endl;
  
  size_t n;
  while (true)
  {
    n = res.find("const");
    if (n != std::string::npos)
      res.erase(n, 5);
    else
    {
      n = res.find_first_of("*&");
      if (n != std::string::npos)
        res.erase(n, 1);
      else
        break;
    }
  }
  return res;
}

