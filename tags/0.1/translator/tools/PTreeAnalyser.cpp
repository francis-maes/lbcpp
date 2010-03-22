/*-----------------------------------------.---------------------------------.
| Filename: PTreeAnalyser.cpp              | PTree analysers                 |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2009 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "PTreeAnalyser.h"
#include "PTreeGenerator.h"
#include "ErrorManager.h"

const SymbolLookup::Symbol* simpleSymbolLookup(SymbolLookup::Scope* currentScope, PTree::Identifier* id)
{
  PTree::Encoding name = PTree::Encoding::simple_name(id);
  SymbolLookup::SymbolSet symbols = currentScope->lookup(name);
  SymbolLookup::SymbolDisplay display(std::cout, 0);
  if (symbols.size() == 0)
  {
    // could not find this symbol, this can either be an error or a global variable declared in a #included file
    return NULL;
  }
  if (symbols.size() > 1)
  {
    ErrorManager::getInstance().addError("Multiple symbols for name '" + name.unmangled() + "'", id);
    return NULL;
  }
  return *symbols.begin();
}

/*
** PTreeAnalyser
*/
void PTreeAnalyser::assertFalse(PTree::Node* node)
{
  std::cerr << "ERROR: unexpected format" << std::endl;
  PTree::display(node, std::cerr, false, true);
  assert(false);
}
void PTreeAnalyser::assertWithNode(bool condition, PTree::Node* node)
{
  if (!condition)
    assertFalse(node);
}

/*
** ParameterPTreeAnalyser 
*/
ParameterPTreeAnalyser::ParameterPTreeAnalyser(PTree::ParameterDeclaration* node)
  : PTreeAnalyser(node), identifierNumber(-1)
{
  setDeclaration(node);
  setDeclarator(PTree::third(node));
}

ParameterPTreeAnalyser::ParameterPTreeAnalyser(PTree::Declaration* declaration, PTree::Declarator* declarator)
  : PTreeAnalyser(declarator), identifierNumber(-1)
{
  setDeclaration(declaration);
  setDeclarator(declarator);
}

void ParameterPTreeAnalyser::setDeclaration(PTree::Node* declaration)
{
  this->declaration = declaration;
  modifiers = declaration ? PTree::first(declaration) : NULL;
  type = declaration ? PTree::second(declaration) : NULL; // fixme: ne gere pas * et &
}

void ParameterPTreeAnalyser::setDeclarator(PTree::Node* node)
{
  declarator = dynamic_cast<PTree::Declarator* >(node);
  assert(!node || declarator);
  if (declarator)
  {
    identifierNumber = 0;
    for (PTree::Iterator it(declarator); !it.empty(); ++it)
    {
      identifier = dynamic_cast<PTree::Identifier* >(*it);
      if (identifier)
        break;
      ++identifierNumber;
    }
    assertWithNode(identifier != NULL, declarator);
  }
}

PTree::Node* ParameterPTreeAnalyser::getRecomposedType(bool includeReferences) const
{
  PTreeGenerator::NodeVector res;
  if (type->is_atom())
    res.push_back(type);
  else
    for (PTree::Iterator it(type); !it.empty(); ++it)
      res.push_back(*it);
  for (int i = 0; i < identifierNumber; ++i)
  {
    PTree::Node* n = PTree::nth(declarator, i);
    assert(n);
    if (includeReferences || !n->is_atom() || *n != "&")
      res.push_back(n);
  }
  return PTreeGenerator().list(res);
}

PTree::Declarator* ParameterPTreeAnalyser::getDeclaratorWithoutInitialValue() const
{
  PTreeGenerator::NodeVector res;
  for (int i = 0; i <= identifierNumber; ++i)
    res.push_back(PTree::nth(declarator, i));
  return PTreeGenerator().listT<PTree::Declarator>(res);
}

PTree::Node* ParameterPTreeAnalyser::getInitializationArguments(bool includeTypeName) const
{
//  std::cout << "DECLARATOR identifierNumber = " << identifierNumber << std::endl;
//  PTree::display(declarator, std::cout, false, true);
  PTree::Node* firstArg = PTree::nth(declarator, identifierNumber + 1);
  PTree::Node* secondArg = PTree::nth(declarator, identifierNumber + 2);
  
  // [type] [var] = [value];
  if (firstArg && secondArg && *firstArg == "=")
    return secondArg;
  else if (firstArg && !secondArg)
  {
    // [type] [var] ([constructor_arguments]);
    PTree::List* init = dynamic_cast<PTree::List* >(firstArg);
    if (init && PTree::first(init) && *PTree::first(init) == "(" && PTree::third(init) && *PTree::third(init) == ")")
    {
      PTree::Node* res = PTree::second(init);
      if (res && includeTypeName)
        return PTreeGenerator().funcallExpr(getRecomposedType(), res);
      else
        return res;
    }
  }
  return NULL;
}

/*
** BlockPTreeAnalyser
*/
BlockPTreeAnalyser::BlockPTreeAnalyser(PTree::Node* node)
  : PTreeAnalyser(node)
{
  if (!node->is_atom() && PTree::length(node) == 3 && *PTree::first(node) == "{" && *PTree::third(node) == "}")
    content = PTree::second(node);
  else
    content = node;
}

/*
** KeywordsPTreeAnalyser
*/
KeywordsPTreeAnalyser::KeywordsPTreeAnalyser(PTree::Node* node)
  : PTreeAnalyser(node) {}

bool KeywordsPTreeAnalyser::contains(const char* keyword) const
{
  for (PTree::Iterator it(node); !it.empty(); ++it)
  {
    PTree::Keyword* k = dynamic_cast<PTree::Keyword* >(*it);
    if (k && *k == keyword)
      return true;
  }
  return false;
}

/*
** ParameterListPTreeAnalyser
*/
ParameterListPTreeAnalyser::ParameterListPTreeAnalyser(PTree::Node* node)
  : PTreeAnalyser(node)
{
  assertWithNode(dynamic_cast<PTree::List* >(node) != NULL, node);
  for (PTree::Iterator it(node); !it.empty(); ++it)
  {
    PTree::ParameterDeclaration* paramDecl = dynamic_cast<PTree::ParameterDeclaration* >(*it);
    if (paramDecl)
      v.push_back(paramDecl);
  }
}

/*
** FunctionPTreeAnalyser
*/
FunctionPTreeAnalyser::FunctionPTreeAnalyser(PTree::FunctionDefinition* node)
  : PTreeAnalyser(node)
{
  assertWithNode(node && PTree::length(node) == 4, node);
  modifiers = PTree::first(node);
  returnType = PTree::second(node);
  declarator = PTree::third(node);
    int n = PTree::length(declarator);
    assertWithNode(n >= 4, declarator);
    int parentIndex = 1;
    for (int i = 0; i < n; ++i)
      if (PTree::reify(PTree::nth(declarator, i)) == "(")
      {
        parentIndex = i;
        break;
      }
    identifier = PTree::nth(declarator, parentIndex - 1);
    assertWithNode(*PTree::nth(declarator, parentIndex) == "(", declarator);
    parameters = PTree::nth(declarator, parentIndex + 1);
    assertWithNode(*PTree::nth(declarator, parentIndex + 2) == ")", declarator);
    constKeyword = PTree::nth(declarator, parentIndex + 3);
  body = PTree::nth(node, 3);
}

/*
** DeclarationPTreeAnalyser
*/
DeclarationPTreeAnalyser::DeclarationPTreeAnalyser(PTree::Declaration* node) 
  : PTreeAnalyser(node)
{
  assertWithNode(node && PTree::length(node) == 4, node);
  modifiers = PTree::first(node);
  type = PTree::second(node);
  declarators = PTree::third(node);
  assertWithNode(*PTree::nth(node, 3) == ";", node);
}

std::vector<ParameterPTreeAnalyser> DeclarationPTreeAnalyser::getVariables() const
{
  std::vector<ParameterPTreeAnalyser> res;
  for (PTree::Iterator it(declarators); !it.empty(); ++it)
  {
    PTree::Declarator* decl = dynamic_cast<PTree::Declarator* >(*it);
    if (decl)
      res.push_back(ParameterPTreeAnalyser((PTree::Declaration* )node, decl));
  }
  return res;
}
