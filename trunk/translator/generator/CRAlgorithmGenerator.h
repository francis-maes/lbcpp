/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmGenerator.h         | CR-algorithm top-level generator|
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_H_
# define CRALGORITHM_GENERATOR_H_

# include "CRAlgorithmScopeGenerator.h"
# include "../tools/PTreeAnalyser.h"

class CRAlgorithmGenerator : public CRAlgorithmScopeGenerator
{
public:
  CRAlgorithmGenerator(const FunctionPTreeAnalyser& input, SymbolLookup::FunctionScope* scope);
  
  PTree::Node* createCode();
  
  virtual void customizeCopyConstructor(ConstructorPTreeGenerator& generator);
  virtual void customizeDefaultConstructor(ConstructorPTreeGenerator& generator);
  virtual void customizeDestructor(FunctionPTreeGenerator& generator);

  bool hasReturnValue() const
    {return !input.getReturnType().isVoid();}

private:
  FunctionPTreeAnalyser input;
//  std::string name;
//  PTree::FunctionDefinition* node;
//  PTree::Node* returnType;
};


#endif // !CRALGORITHM_GENERATOR_H_
