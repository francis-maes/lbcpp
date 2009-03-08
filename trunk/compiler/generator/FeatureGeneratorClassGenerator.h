/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorGenerator.h    | Feature generator generator     |
| Author  : Francis Maes                   |                                 |
| Started : 05/02/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_FEATURE_GENERATOR_GENERATOR_H_
# define CRALGORITHM_GENERATOR_FEATURE_GENERATOR_GENERATOR_H_

# include "../tools/RewriteVisitor.h"
# include "../tools/PTreeAnalyser.h"
# include "../tools/PTreeGenerator.h"
# include "../language/CRAlgoLexer.h"

class FeatureGeneratorClassGenerator : public ClassPTreeGenerator
{
public:
  FeatureGeneratorClassGenerator(PTree::FunctionDefinition* node, SymbolLookup::Scope* scope, bool isInCRAlgorithm);
  
  PTree::Node* createCode(PTree::FunctionDefinition** staticToDynamicFunctionDefinition = NULL);
  
private:
  FunctionPTreeAnalyser input;
  ParameterListPTreeAnalyser parameters;
  SymbolLookup::Class* classScope;
  bool isInCRAlgorithm;
  bool isStatic;
};

#endif // !CRALGORITHM_GENERATOR_FEATURE_GENERATOR_GENERATOR_H_
