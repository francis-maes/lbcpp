/*-----------------------------------------.---------------------------------.
| Filename: PTreeGenerator.cpp             | Base class for code generators  |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2009 19:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "PTreeGenerator.h"

#define INIT_KEYWORD_T(Name, Type, String) \
 Name(new PTree::KeywordT<Token :: Type>(Token(String, strlen(String), Token :: Type)))

#define INIT_ATOM(Name, String) \
 Name(new PTree::Atom(String, strlen(String)))

struct PTreeGeneratorConstants
{
  PTreeGeneratorConstants()
    : ifKeyword(new PTree::Kwd::If("if", 2)),
    switchKeyword(new PTree::Kwd::Switch("switch", 6)), 
    caseKeyword(new PTree::Kwd::Case("case", 4)),
    defaultKeyword(new PTree::Kwd::Default("default", 7)),
    gotoKeyword(new PTree::Kwd::Goto("goto", 4)),
    breakKeyword(new PTree::Kwd::Break("break", 5)),
    returnKeyword(new PTree::Kwd::Return("return", 6)),
    thisKeyword(new PTree::Kwd::This("this", 4)),
    
    INIT_KEYWORD_T(classKeyword, CLASS, "class"),
    INIT_KEYWORD_T(structKeyword, STRUCT, "struct"),
    INIT_KEYWORD_T(publicKeyword, PUBLIC, "public"),
    INIT_KEYWORD_T(protectedKeyword, PROTECTED, "protected"),
    INIT_KEYWORD_T(privateKeyword, PRIVATE, "private"),
    INIT_KEYWORD_T(virtualKeyword, VIRTUAL, "virtual"),
    INIT_KEYWORD_T(staticKeyword, STATIC, "static"),
    INIT_KEYWORD_T(constKeyword, CONST, "const"),
    INIT_KEYWORD_T(voidKeyword, VOID, "void"),
    INIT_KEYWORD_T(intKeyword, INT, "int"),
    INIT_KEYWORD_T(doubleKeyword, DOUBLE, "double"),
    INIT_KEYWORD_T(charKeyword, CHAR, "char"),
    INIT_KEYWORD_T(typedefKeyword, TYPEDEF, "typedef"),
    INIT_KEYWORD_T(newKeyword, NEW, "new"),
    INIT_KEYWORD_T(deleteKeyword, DELETE, "delete"),
    INIT_KEYWORD_T(elseKeyword, ELSE, "else"),
    INIT_KEYWORD_T(continueKeyword, CONTINUE, "continue"),
    INIT_KEYWORD_T(inlineKeyword, INLINE, "inline"),
    INIT_KEYWORD_T(enumKeyword, ENUM, "enum"),
    
    INIT_ATOM(newlineAtom, "\n"),
    INIT_ATOM(commaAtom, ","),
    INIT_ATOM(oParentAtom, "("),
    INIT_ATOM(cParentAtom, ")"),
    INIT_ATOM(colonAtom, ":"),
    INIT_ATOM(semiColonAtom, ";"),
    INIT_ATOM(rightArrowAtom, "->"),
    INIT_ATOM(dotAtom, "."),
    INIT_ATOM(oBraceAtom, "{"),
    INIT_ATOM(cBraceAtom, "}"),
    INIT_ATOM(assignAtom, "="),
    INIT_ATOM(equalAtom, "=="),
    INIT_ATOM(notEqualAtom, "!="),
    INIT_ATOM(doubleSlashAtom, "//"),
    INIT_ATOM(questionMarkAtom, "?")
  {
  
  }
  
  std::map<std::string, const char* > strings;
  
  std::map<const char*, PTree::Atom* > atoms;
  std::map<const char*, PTree::Identifier* > identifiers;
  std::map<const char*, PTree::Literal* > literals;
  
  PTree::Keyword *ifKeyword, *switchKeyword, *caseKeyword, *defaultKeyword;
  PTree::Keyword *gotoKeyword, *breakKeyword, *returnKeyword, *thisKeyword;
  PTree::Keyword *classKeyword, *structKeyword, *publicKeyword, *protectedKeyword;
  PTree::Keyword *privateKeyword, *virtualKeyword, *staticKeyword, *constKeyword;
  PTree::Keyword *voidKeyword, *intKeyword, *doubleKeyword, *charKeyword;
  PTree::Keyword *typedefKeyword, *newKeyword, *deleteKeyword, *boolKeyword;
  PTree::Keyword *elseKeyword, *continueKeyword, *inlineKeyword, *enumKeyword;
  
  PTree::Atom *newlineAtom, *commaAtom, *oParentAtom, *cParentAtom;
  PTree::Atom *colonAtom, *semiColonAtom, *rightArrowAtom, *dotAtom;
  PTree::Atom *oBraceAtom, *cBraceAtom, *assignAtom, *equalAtom;
  PTree::Atom *notEqualAtom, *doubleSlashAtom, *questionMarkAtom;
};

static PTreeGeneratorConstants constants;

const char* PTreeGenerator::registerUserString(const std::string& str)
{
  std::map<std::string, const char* >::const_iterator it = constants.strings.find(str);
  if (it == constants.strings.end())
  {
    const char*& ptr = constants.strings[str];
    ptr = constants.strings.find(str)->first.c_str();
    return ptr;
  }
  else
    return it->second;
}

PTree::Atom* PTreeGenerator::atom(const std::string& text)
{
  const char* ptr = registerUserString(text);
  PTree::Atom*& res = constants.atoms[ptr];
  if (!res)
    res = new PTree::Atom(ptr, text.length());
  return res;
}

PTree::Identifier* PTreeGenerator::identifier(const std::string& text)
{
  const char* ptr = registerUserString(text);
  PTree::Identifier*& res = constants.identifiers[ptr];
  if (!res)
    res = new PTree::Identifier(ptr, text.length());
  return res;
}

PTree::Literal* PTreeGenerator::literal(const std::string& text)
{
  const char* ptr = registerUserString(text);
  PTree::Literal*& res = constants.literals[ptr];
  if (!res)
    res = new PTree::Literal(Token(ptr, text.length(), Token::Constant));
  return res;
}
  
/*
** Keywords
*/
#define IMPLEMENT_KEYWORD(NAME) \
  PTree::Keyword* PTreeGenerator:: NAME () {return constants. NAME ;}

IMPLEMENT_KEYWORD(ifKeyword);
IMPLEMENT_KEYWORD(switchKeyword);
IMPLEMENT_KEYWORD(caseKeyword);
IMPLEMENT_KEYWORD(defaultKeyword);
IMPLEMENT_KEYWORD(gotoKeyword);
IMPLEMENT_KEYWORD(breakKeyword);
IMPLEMENT_KEYWORD(returnKeyword);
IMPLEMENT_KEYWORD(thisKeyword);
IMPLEMENT_KEYWORD(classKeyword);
IMPLEMENT_KEYWORD(structKeyword);
IMPLEMENT_KEYWORD(publicKeyword);
IMPLEMENT_KEYWORD(protectedKeyword);
IMPLEMENT_KEYWORD(privateKeyword);
IMPLEMENT_KEYWORD(virtualKeyword);
IMPLEMENT_KEYWORD(staticKeyword);
IMPLEMENT_KEYWORD(constKeyword);
IMPLEMENT_KEYWORD(voidKeyword);
IMPLEMENT_KEYWORD(intKeyword);
IMPLEMENT_KEYWORD(doubleKeyword);
IMPLEMENT_KEYWORD(charKeyword);
IMPLEMENT_KEYWORD(typedefKeyword);
IMPLEMENT_KEYWORD(newKeyword);
IMPLEMENT_KEYWORD(deleteKeyword);
IMPLEMENT_KEYWORD(elseKeyword);
IMPLEMENT_KEYWORD(continueKeyword);
IMPLEMENT_KEYWORD(inlineKeyword);
IMPLEMENT_KEYWORD(enumKeyword);

#define IMPLEMENT_ATOM(NAME) \
    PTree::Atom* PTreeGenerator:: NAME () {return constants. NAME ;}

IMPLEMENT_ATOM(newlineAtom);
IMPLEMENT_ATOM(commaAtom);
IMPLEMENT_ATOM(oParentAtom);
IMPLEMENT_ATOM(cParentAtom);
IMPLEMENT_ATOM(colonAtom);
IMPLEMENT_ATOM(semiColonAtom);
IMPLEMENT_ATOM(rightArrowAtom);
IMPLEMENT_ATOM(dotAtom);
IMPLEMENT_ATOM(oBraceAtom);
IMPLEMENT_ATOM(cBraceAtom);
IMPLEMENT_ATOM(assignAtom);
IMPLEMENT_ATOM(equalAtom);
IMPLEMENT_ATOM(notEqualAtom);
IMPLEMENT_ATOM(doubleSlashAtom);
IMPLEMENT_ATOM(questionMarkAtom);

/*
** Statements
*/
PTree::IfStatement* PTreeGenerator::ifStatement(PTree::Node* condition, PTree::Node* thenBlock, PTree::Node* elseBlock)
{
  NodeVector nodes;
  nodes.push_back(oParentAtom());
  nodes.push_back(condition);
  nodes.push_back(cParentAtom());
  nodes.push_back(thenBlock);
  if (elseBlock)
  {
    nodes.push_back(elseKeyword());
    nodes.push_back(elseBlock);
  }
  return new PTree::IfStatement(ifKeyword(), list(nodes));
}

/*
** Expressions
*/
PTree::FuncallExpr* PTreeGenerator::funcallExpr(PTree::Node* identifier, PTree::Node* arguments)
  {return new PTree::FuncallExpr(identifier, list(oParentAtom(), arguments, cParentAtom()));}

// [left]->[right]
PTree::ArrowMemberExpr* PTreeGenerator::arrowMemberExpr(PTree::Node* left, PTree::Node* right)
  {return new PTree::ArrowMemberExpr(left, list(rightArrowAtom(), right));}

// [left].[right]
PTree::DotMemberExpr* PTreeGenerator::dotMemberExpr(PTree::Node* left, PTree::Node* right)
  {return new PTree::DotMemberExpr(left, list(dotAtom(), right));}

// [left] = [right]
PTree::AssignExpr* PTreeGenerator::assignExpr(PTree::Node* left, PTree::Node* right)
  {return new PTree::AssignExpr(left, list(assignAtom(), right));}    

// [condition] ? [truePart] : [falsePart]
PTree::CondExpr* PTreeGenerator::condExpr(PTree::Node* condition, PTree::Node* truePart, PTree::Node* falsePart)
{
  return new PTree::CondExpr(condition, 
    list(questionMarkAtom(), truePart, colonAtom(), falsePart));
}

// [left] [operation] [right]
PTree::InfixExpr* PTreeGenerator::infixExpr(PTree::Node* left, PTree::Node* operation, PTree::Node* right)
  {return new PTree::InfixExpr(left, list(operation, right));}

/*
** Declarations
*/
PTree::ParameterDeclaration* PTreeGenerator::parameterDeclaration(PTree::Node* type, PTree::Node* identifier, PTree::Node* initialValue)
{
  NodeVector declarator;
  if (identifier->is_atom())
    declarator.push_back(identifier);
  else
  {
    for (PTree::Iterator it(identifier); !it.empty(); ++it)
      declarator.push_back(*it);
  }
  if (initialValue)
  {
    declarator.push_back(assignAtom());
    declarator.push_back(initialValue);
  }
  return new PTree::ParameterDeclaration(NULL, type, listT<PTree::Declarator>(declarator));
}

// misc
PTreeGenerator::NodeVector PTreeGenerator::removeTokenFromNodeList(PTree::Node* list, const char* tokenToRemove)
{
  NodeVector res;
  if (!list)
    return res;
  for (PTree::Iterator it(list); !it.empty(); ++it)
    if (PTree::reify(*it) != tokenToRemove)
      res.push_back(*it);
  return res;
}

/*
** BlockPTreeGenerator
*/
void BlockPTreeGenerator::add(PTree::Node* node)
  {nodes.push_back(node);}

void BlockPTreeGenerator::add(const NodeVector& nodes)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    add(nodes[i]);
}
  
void BlockPTreeGenerator::addComment(const char* commentLine)
  {nodes.push_back(comment(commentLine));}

void BlockPTreeGenerator::addNewLine()
  {nodes.push_back(newlineAtom());}

void BlockPTreeGenerator::addAccessSpecifier(PTree::Keyword* keyword)
  {nodes.push_back(new PTree::AccessSpec(keyword, list(colonAtom()), NULL));}
  
void BlockPTreeGenerator::addExpressionStatement(PTree::Node* expression)
  {nodes.push_back(new PTree::ExprStatement(expression, list(semiColonAtom())));}

// [type] [identifier];
// or
// [type] [identifier] = [initialValue]; 
void BlockPTreeGenerator::addVariableDeclaration(PTree::Node* type, PTree::Node* identifierOrDeclarator, PTree::Node* initialValue)
{
  PTree::Declarator* declarator = NULL;
  if (!initialValue)
    declarator = dynamic_cast<PTree::Declarator* >(identifierOrDeclarator);
  if (!declarator)
    declarator = new PTree::Declarator(initialValue
      ? list(identifierOrDeclarator, assignAtom(), initialValue)
      : list(identifierOrDeclarator));
  nodes.push_back(new PTree::Declaration(NULL, list(type, list(declarator), semiColonAtom())));
}

PTree::Node* BlockPTreeGenerator::createContent()
{
  if (nodes.size() == 1 && dynamic_cast<PTree::Brace* >(nodes[0]))
    return PTree::second(nodes[0]);
  return list(nodes);
}

PTree::Block* BlockPTreeGenerator::createBlock()
{
  return new PTree::Block(oBraceAtom(), createContent(), cBraceAtom());
}

/*
** ParamDeclarationListPTreeGenerator
*/
void ParamDeclarationListPTreeGenerator::addDeclarator(PTree::Declarator* node)
{
  if (declarators.size())
    declarators.push_back(commaAtom());
  declarators.push_back(node);
}

PTree::Declaration* ParamDeclarationListPTreeGenerator::createDeclaration()
{
  return new PTree::Declaration(list(modifiers), list(type, list(declarators), semiColonAtom()));
}

void ParamDeclarationListPTreeGenerator::addModifier(PTree::Node* modifier)
  {modifiers.push_back(modifier);}

void ParamDeclarationListPTreeGenerator::addModifiers(PTree::Node* modifiers)
{
  for (PTree::Iterator it(modifiers); !it.empty(); ++it)
    addModifier(*it);
}

void ParamDeclarationListPTreeGenerator::addModifiers(const NodeVector& modifiers)
{
  for (size_t i = 0; i < modifiers.size(); ++i)
    addModifier(modifiers[i]);
}

/*
** ClassPTreeGenerator
*/  
ClassPTreeGenerator::ClassPTreeGenerator()
  : keyword(classKeyword()) {}

void ClassPTreeGenerator::setKeyword(PTree::Keyword* structOrClass)
  {keyword = structOrClass;}
  
void ClassPTreeGenerator::addBaseClass(PTree::Keyword* keyword, PTree::Identifier* identifier)
{
  baseClasses.push_back(baseClasses.size() == 0 ? colonAtom() : commaAtom());
  baseClasses.push_back(list(keyword, identifier));
}

PTree::ClassSpec* ClassPTreeGenerator::createSpecification()
{
  NodeVector nodes;
  nodes.push_back(keyword);
  nodes.push_back(ptreeIdentifier);
  if (baseClasses.size())
    nodes.push_back(list(baseClasses));
  if (!body.isEmpty())
    nodes.push_back(new PTree::ClassBody(oBraceAtom(), body.createContent(), cBraceAtom()));
  nodes.push_back(semiColonAtom());
  return new PTree::ClassSpec(nodes[0], list(nodes, 1), NULL);
}

PTree::Declaration* ClassPTreeGenerator::createDeclaration()
  {return new PTree::Declaration(NULL, list(createSpecification()));}

/*
** FunctionPTreeGenerator
*/
FunctionPTreeGenerator::FunctionPTreeGenerator() : isConst(false), returnType(NULL)
{
}

PTree::FunctionDefinition* FunctionPTreeGenerator::createDeclaration()
{
  NodeVector declarator;
  declarator.push_back(ptreeIdentifier);
  declarator.push_back(oParentAtom());
  declarator.push_back(list(parameters));
  declarator.push_back(cParentAtom());
  if (isConst)
    declarator.push_back(constKeyword());
  customizeDeclarator(declarator);
  return new PTree::FunctionDefinition(list(modifiers), 
    list(returnType, list(declarator),
      body.isEmpty() ? (PTree::Node* )semiColonAtom() : body.createBlock()));
}

void FunctionPTreeGenerator::setReturnType(PTree::Node* returnType)
  {this->returnType = returnType;}

void FunctionPTreeGenerator::setConst(bool isConst)
  {this->isConst = isConst;}
  
void FunctionPTreeGenerator::addModifier(PTree::Node* modifier)
  {modifiers.push_back(modifier);}

void FunctionPTreeGenerator::addModifiers(PTree::Node* modifiers)
{
  for (PTree::Iterator it(modifiers); !it.empty(); ++it)
    addModifier(*it);
}
  
void FunctionPTreeGenerator::addParameter(PTree::Node* parameterDeclaration)
{
  if (parameters.size())
    parameters.push_back(commaAtom());
  parameters.push_back(parameterDeclaration);
}

void FunctionPTreeGenerator::addParameter(PTree::Node* type, PTree::Node* identifier, PTree::Node* initialValue)
{
  if (parameters.size())
    parameters.push_back(commaAtom());
  parameters.push_back(parameterDeclaration(type, identifier, initialValue));
}

void FunctionPTreeGenerator::addParameters(const NodeVector& parametersList)
{
  for (size_t i = 0; i < parametersList.size(); ++i)
    addParameter(parametersList[i]);
}

void FunctionPTreeGenerator::addParameters(PTree::Node* parametersList)
{
  for (PTree::Iterator it(parametersList); !it.empty(); ++it)
    if (dynamic_cast<PTree::ParameterDeclaration* >(*it))
      addParameter(*it);
}

/*
** ConstructorPTreeGenerator
*/
void ConstructorPTreeGenerator::addInitializer(PTree::Node* identifier, PTree::Node* value, bool onANewLine)
  {initializationList.push_back(std::make_pair(std::make_pair(identifier, value), onANewLine));}
  
void ConstructorPTreeGenerator::customizeDeclarator(NodeVector& declarator)
{
  assert(returnType == NULL);
  if (initializationList.empty())
    return;

  NodeVector initializers;
  for (size_t i = 0; i < initializationList.size(); ++i)
  {
    if (i == 0)
      initializers.push_back(colonAtom());
    else
      initializers.push_back(commaAtom());
    if (initializationList[i].second)
      initializers.push_back(atom("\n"));
    initializers.push_back(list(initializationList[i].first.first,
      oParentAtom(), list(initializationList[i].first.second), cParentAtom()));
  }
  declarator.push_back(list(initializers));
}

/*
** SwitchPTreeGenerator
*/
SwitchPTreeGenerator::SwitchPTreeGenerator() : condition(NULL) {}
  
void SwitchPTreeGenerator::setCondition(PTree::Node* condition)
  {this->condition = condition;}

void SwitchPTreeGenerator::addCase(PTree::Node* caseValue, PTree::Node* caseImpl)
  {cases.add(new PTree::CaseStatement(caseKeyword(), list(caseValue, colonAtom(), caseImpl)));}

void SwitchPTreeGenerator::setDefault(PTree::Node* defaultImpl)
  {cases.add(new PTree::DefaultStatement(defaultKeyword(), list(colonAtom(), defaultImpl)));}

PTree::Node* SwitchPTreeGenerator::createStatement()
{
  // if there is only a default case, then do not generate the switch
  if (cases.size() == 1 && dynamic_cast<PTree::DefaultStatement* >(cases[0]))
    return PTree::third(cases[0]);
  
  return new PTree::SwitchStatement(switchKeyword(),
    list(oParentAtom(), condition, cParentAtom(), cases.createBlock()));
}

/*
** FuncallPTreeGenerator
*/
void FuncallPTreeGenerator::addArgument(PTree::Node* identifier)
{
  if (arguments.size())
    arguments.push_back(commaAtom());
  arguments.push_back(identifier);
}

void FuncallPTreeGenerator::addArguments(PTree::Node* arguments)
{
  for (PTree::Iterator it(arguments); !it.empty(); ++it)
    if (**it != ",")
      addArgument(*it);
}

PTree::FuncallExpr* FuncallPTreeGenerator::createExpression()
{
  return funcallExpr(ptreeIdentifier, list(arguments));
}
