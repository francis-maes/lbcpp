/*-----------------------------------------.---------------------------------.
| Filename: CRAlgoLexer.h                  | CR-Algorithm Lexer              |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 17:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_LEXER_H_
# define CRALGO_LEXER_H_

# include "../common.h"

class CRAlgoToken : public Token
{
public:
  static const char* crAlgorithm;
  static const char* crAlgorithmCall;
  static const char* choose;
  static const char* reward;
  static const char* stateFunction;
  static const char* featureGenerator;
  static const char* featureGeneratorCall;
  static const char* featureScope;
  static const char* featureSense;

  enum
  {
    FEATURE_SCOPE = firstUserKeyword,
    //CHOOSE,
    //REWARD,
    
    STATE_FUNCTION,
    
    CR_ALGORITHM_CALL,
    FEATURE_GENERATOR_CALL,
    
    //FEATURE_GENERATOR,
    // CR_ALGORITHM = UserKeyword5,
  };
};

class CRAlgoLexer : public Lexer
{
public:
  CRAlgoLexer(Buffer* buffer, int tokenset = CXX|GCC);
};

#endif // !CRALGO_LEXER_H_
