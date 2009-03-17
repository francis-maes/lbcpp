/*-----------------------------------------.---------------------------------.
| Filename: CRAlgoParser.cpp               | CR-Algorithm Parser             |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "CRAlgoParser.h"
#include "CRAlgoLexer.h"
#include "CRAlgoPTree.h"
#include "../tools/ErrorManager.h"
#include "../tools/PTreeAnalyser.h"

/*
** CRAlgoLexer
*/
const char* CRAlgoToken::crAlgorithm = "crAlgorithm";
const char* CRAlgoToken::crAlgorithmCall = "crAlgorithmCall";
const char* CRAlgoToken::choose = "choose";
const char* CRAlgoToken::reward = "reward";
const char* CRAlgoToken::chooseFunction = "chooseFunction";
const char* CRAlgoToken::featureGenerator = "featureGenerator";
const char* CRAlgoToken::featureGeneratorCall = "featureCall";
const char* CRAlgoToken::featureScope = "featureScope";
const char* CRAlgoToken::featureSense = "featureSense";

CRAlgoLexer::CRAlgoLexer(Buffer* buffer, int tokenset)
  : Lexer(buffer, tokenset)
{
  my_keywords[CRAlgoToken::crAlgorithmCall] = CRAlgoToken::CR_ALGORITHM_CALL;
//  my_keywords[CRAlgoToken::choose] = CRAlgoToken::CHOOSE;
//  my_keywords[CRAlgoToken::reward] = CRAlgoToken::REWARD;
  my_keywords[CRAlgoToken::chooseFunction] = CRAlgoToken::STATE_FUNCTION;
  my_keywords[CRAlgoToken::featureGeneratorCall] = CRAlgoToken::FEATURE_GENERATOR_CALL;
  my_keywords[CRAlgoToken::featureScope] = CRAlgoToken::FEATURE_SCOPE;
}

static Token featureScopeToken(CRAlgoToken::featureScope, strlen(CRAlgoToken::featureScope), CRAlgoToken::FEATURE_SCOPE);

/*
** CRAlgoPTree
*/
std::string CRAlgo::StateFundefStatement::getKind() const
{
  // todo: finish and make more tests
  FunctionPTreeAnalyser input(getFunctionDefinition());
  
  std::string returnType = input.getReturnTypeString();
  size_t numParams = input.getParameters().size();
  if (returnType == "std :: string")
  {
    if (numParams == 0)
      return "StateDescription";
    else if (numParams == 1)
      return "ActionDescription";
  }
  if (returnType == "featureGenerator")
  {
    if (numParams == 0)
      return "StateFeatures";
    else if (numParams == 1)
      return "ActionFeatures";
  }
  else if (returnType == "double")
  {
    if (numParams == 0)
      return "StateValue";
    else if (numParams == 1)
      return "ActionValue";
  }
  return "Choose";
}

namespace CRAlgo
{

bool isFeatureSense(PTree::Node* identifier)
{
  return identifier && *identifier == CRAlgoToken::featureSense;
}


bool isReward(PTree::Node* identifier)
{
  return identifier && *identifier == CRAlgoToken::reward;
}

bool isCRAlgorithm(PTree::FunctionDefinition* node)
{
  return FunctionPTreeAnalyser(node).getModifiers().contains(CRAlgoToken::crAlgorithm);
}

bool isFeatureGenerator(PTree::FunctionDefinition* node)
{
  return PTree::reify(PTree::second(node)) == CRAlgoToken::featureGenerator 
    || FunctionPTreeAnalyser(node).getModifiers().contains(CRAlgoToken::featureGenerator);
     // for a mysterious reason, featureGenerator inside classes are not parsed in the same way as local featureGenerators.
     // inside class => featureGenerator is a function modifier
     // otherwise => featureGenerator is the return type
}

// crAlgorithm<BaseType>
bool isCRAlgorithmType(PTree::Node* type, std::string* crAlgorithmIdentifier)
{
  if (PTree::length(type) < 2) return false;
  if (PTree::first(type) && PTree::first(type)->is_atom() && *PTree::first(type) != CRAlgoToken::crAlgorithm) return false;
  PTree::Node* templ = PTree::second(type);
  if (!templ) return false;
  if (PTree::length(templ) != 3) return false;
  if (!PTree::first(templ) || !PTree::first(templ)->is_atom() || *PTree::first(templ) != "<" || *PTree::third(templ) != ">") return false;
  PTree::Node* templArgs = PTree::second(templ);
  if (!templArgs || PTree::length(templArgs) != 1) return false;
  PTree::Node* templArg = PTree::first(templArgs);
  if (!templArg) return false;
  if (crAlgorithmIdentifier)
  {
    std::string str = PTree::reify(templArg);
    size_t n = str.find_last_not_of(" \t\r\n");
    assert(n != std::string::npos);
    *crAlgorithmIdentifier = str.substr(0, n + 1);
  }
  return true;
}

};

/*
** CRAlgoParser
*/
CRAlgoParser::CRAlgoParser(Lexer& lexer, SymbolFactory& symbols, int ruleset)
  : Parser(lexer, symbols, ruleset), insideCRAlgorithm(false) {}

bool CRAlgoParser::parse_user_statement(PTree::Node*& st)
{
  Token tk;
  int k = my_lexer.look_ahead(0, tk);
  if (k == CRAlgoToken::STATE_FUNCTION)
    return chooseFunction(st);
  if (k == CRAlgoToken::FEATURE_SCOPE)
    return featureScope(st);
  if (k == CRAlgoToken::FEATURE_GENERATOR_CALL)
    return featureCall(st);
  return false;
}

bool CRAlgoParser::parse_user_postfix_expr(PTree::Node*& st)
{
  Token tk;
  int k = my_lexer.look_ahead(0, tk);
  if (tk == CRAlgoToken::choose)
  {
    Token tk2;
    my_lexer.look_ahead(1, tk2);
    if (tk2 == '<')
      return choose(st);
  }
  if (k == CRAlgoToken::CR_ALGORITHM_CALL)
    return crAlgorithmCall(st);
  return false;
}

bool CRAlgoParser::is_user_opt_member_spec(const Token& token)  
  {return token == CRAlgoToken::crAlgorithm || token == CRAlgoToken::featureGenerator;}

// crAlgorithmCall [funcall-expr] 
bool CRAlgoParser::crAlgorithmCall(PTree::Node*& st)
{
  Trace trace("CRAlgoParser::crAlgorithmCall", Trace::PARSING);
  Token tk1;
  PTree::Node* n;
  
  if (my_lexer.get_token(tk1) != CRAlgoToken::CR_ALGORITHM_CALL) return false;
  if (!postfix_expr(n)) return false;
  st = new CRAlgo::CRAlgorithmCallExpr(new PTree::UserKeyword(tk1), PTree::list(n));
  return true;
}


// choose < type_id > ( arguments )
bool CRAlgoParser::choose(PTree::Node*& st)
{
  Trace trace("CRAlgoParser::choose", Trace::PARSING);
  Token tk1, tk2, tk3, tk4, tk5;
  PTree::Node* type;
  PTree::Encoding typeName;
  PTree::Node* arguments;

  my_lexer.get_token(tk1);
  if (tk1 != CRAlgoToken::choose) return false;
  if (my_lexer.get_token(tk2) != '<') return false;
  if (!type_id(type, typeName)) return false;
  if (my_lexer.get_token(tk3) != '>') return false;
  if (my_lexer.get_token(tk4) != '(') return false;
  if (!function_arguments(arguments)) return false;
  if (my_lexer.get_token(tk5) != ')') return false;

  st = new CRAlgo::ChooseExpression(new PTree::UserKeyword(tk1),
         new PTree::Atom(tk2), new PTree::Name(type, typeName), new PTree::Atom(tk3),
         new PTree::Atom(tk4), arguments, new PTree::Atom(tk5));
  return true;
}

// chooseFunction [returnType] [identifier] ( [arguments-list] ) 
//    [body]
bool CRAlgoParser::chooseFunction(PTree::Node*& st)
{
  Trace trace("CRAlgoParser::chooseFunction", Trace::PARSING);
  Token tk1, tk2, tk3, tk4;
  PTree::Node* returnType;
  PTree::Node* arguments;
  PTree::Encoding returnTypeEncoding, argumentsEncoding;
  
  // parse declaration
  if(my_lexer.get_token(tk1) != CRAlgoToken::STATE_FUNCTION) return false;
  if(!type_specifier(returnType, false, returnTypeEncoding)) return false;
  if(my_lexer.get_token(tk2) != Token::Identifier) return false;
  if(my_lexer.get_token(tk3) != '(') return false;
  {
    ScopeGuard guard(*this, new PTree::Atom("dummy", 5));
    if(!parameter_declaration_list(arguments, argumentsEncoding)) return false;
  }
  if(my_lexer.get_token(tk4) != ')') return false;

  CRAlgo::StateFundefStatement* def = new CRAlgo::StateFundefStatement(new PTree::UserKeyword(tk1), NULL);

  // parse body
  SymbolLookup::PrototypeScope* prototype = my_symbols.pop_current_prototype();
  ScopeGuard guard(*this, def);
  SymbolLookup::UserScope* userScope = dynamic_cast<SymbolLookup::UserScope* >(my_symbols.current_scope());
  assert(userScope);
  userScope->setPrototype(prototype);
  prototype->unref();    
  PTree::Node* body;
  if(!statement(body)) return false;

  PTree::Declarator* declarator = new PTree::Declarator(
    PTree::list(new PTree::Identifier(tk2), new PTree::Atom(tk3), arguments, new PTree::Atom(tk4)));
  PTree::FunctionDefinition* fundef = new PTree::FunctionDefinition(NULL, PTree::list(returnType, declarator, body));
  st = PTree::snoc(def, fundef);
  return true;  
}


// featureCall [funcall-expr] or
// featureCall(identifier) [funcall-expr] which is syntaxic sugar for 
//      featureScope(identifier) featureCall [funcall-expr] 
bool CRAlgoParser::featureCall(PTree::Node*& st)
{
  Trace trace("CRAlgoParser::featureCall", Trace::PARSING);
  Token tk1, tk2, tk3, tk4;
  PTree::Node* identifier = NULL;
  PTree::Node* n;

  if (my_lexer.get_token(tk1) != CRAlgoToken::FEATURE_GENERATOR_CALL) return false;
  
  my_lexer.look_ahead(0, tk2);
  if (tk2 == '(')
  {
    if (my_lexer.get_token(tk2) != '(') return false;
    if (!expression(identifier)) return false;
    if (my_lexer.get_token(tk3) != ')') return false;
    
    bool isInline = false; 
    Token inlineToken;
    if (my_lexer.look_ahead(0) == Token::INLINE)
      isInline = true, my_lexer.get_token(inlineToken);
    
    if (!postfix_expr(n)) return false;
    if (my_lexer.get_token(tk4) != ';') return false;
    PTree::Node* body = new CRAlgo::FeatureGeneratorCallStatement(new PTree::UserKeyword(tk1), n, new PTree::Atom(tk4),
      isInline ? new PTree::KeywordT<Token::INLINE>(inlineToken) : NULL);
      
    static Token featureScopeToken(CRAlgoToken::featureScope, strlen(CRAlgoToken::featureScope), CRAlgoToken::FEATURE_SCOPE);
    st = new CRAlgo::FeatureScopeStatement(new PTree::UserKeyword(featureScopeToken), new PTree::Atom(tk2), identifier, new PTree::Atom(tk3), body);
  }
  else
  {
    bool isInline = false; 
    Token inlineToken;
    if (my_lexer.look_ahead(0) == Token::INLINE)
      isInline = true, my_lexer.get_token(inlineToken);

    if (!postfix_expr(n)) return false;
    if (my_lexer.get_token(tk2) != ';') return false;
    st = new CRAlgo::FeatureGeneratorCallStatement(new PTree::UserKeyword(tk1), n, new PTree::Atom(tk2),
            isInline ? new PTree::KeywordT<Token::INLINE>(inlineToken) : NULL);
  }
  return true;
}


// featureScope(identifier) [block]
bool CRAlgoParser::featureScope(PTree::Node*& st)
{
  Trace trace("CRAlgoParser::featureScope", Trace::PARSING);
  Token tk1, tk2, tk3;
  PTree::Node* exp;
  PTree::Node* body;

  if (my_lexer.get_token(tk1) != CRAlgoToken::FEATURE_SCOPE) return false;
  if (my_lexer.get_token(tk2) != '(') return false;
  if (!expression(exp)) return false;
  if (my_lexer.get_token(tk3) != ')') return false;
  if (!statement(body)) return false;

  st = new CRAlgo::FeatureScopeStatement(new PTree::UserKeyword(tk1), new PTree::Atom(tk2), exp, new PTree::Atom(tk3), body);
  return true;
}  
