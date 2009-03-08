/*-----------------------------------------.---------------------------------.
| Filename: CRAlgoParser.h                 | CR-Algorithm Parser             |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 17:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_LANGUAGE_PARSER_H_
# define CRALGO_LANGUAGE_PARSER_H_

# include "../common.h"

class CRAlgoParser : public Parser
{
public:
  CRAlgoParser(Lexer& lexer, SymbolFactory& symbols, int ruleset = CXX|GCC);

  virtual bool parse_user_statement(PTree::Node*& st);
  virtual bool parse_user_postfix_expr(PTree::Node*& st);
  virtual bool is_user_opt_member_spec(const Token& token);

protected:
  bool insideCRAlgorithm;

  bool crAlgorithmCall(PTree::Node*& st);
  bool featureCall(PTree::Node*& st);
  bool choose(PTree::Node*& st);
  bool stateFunction(PTree::Node*& st);
  bool featureScope(PTree::Node*& st);
};

#endif // !CRALGO_LANGUAGE_PARSER_H_
