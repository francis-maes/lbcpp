/*-----------------------------------------.---------------------------------.
| Filename: ChooseClassGenerator.h         | Choose class generator          |
| Author  : Francis Maes                   |                                 |
| Started : 19/02/2009 21:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_CHOOSE_CLASS_H_
# define CRALGORITHM_GENERATOR_CHOOSE_CLASS_H_

# include "../tools/PTreeGenerator.h"
# include "../language/CRAlgorithmChoose.h"

class ChooseClassGenerator : public ClassPTreeGenerator
{
public:
  ChooseClassGenerator(CRAlgorithmChoose& choose, SymbolLookup::Scope* scope, const std::string& crAlgorithmClassName);
  
  struct StateFunctionInfo
  {
    StateFunctionInfo(const std::string& identifier, CRAlgo::StateFundefStatement* stateFunctionDefinition)
      : identifier(identifier), className("__" + identifier + "Function__"), kind(stateFunctionDefinition->getKind()) {}
    
    std::string identifier;
    std::string className;
    std::string kind;
    
    std::string getDynamicClassName() const
      {return ChooseClassGenerator::getDynamicClassName(kind);}
  };
  
private:
  size_t getNumStateFunctionsOfKind(const std::string& kind) const;
  std::vector<StateFunctionInfo> getStateFunctionsOfKind(const std::string& kind) const;  
  

  static std::string getDynamicClassName(const std::string& kind)
    {return "lcpp::" + kind + "Function";}

  std::string className;
  std::string crAlgorithmClassName;
  std::vector<StateFunctionInfo> stateFunctions;
  
  void addMemberVariables(BlockPTreeGenerator& classBody);

  PTree::Node* createNumStateValuesEnum();
  PTree::Node* createStateFunctionGetter(const std::string& stateFunctionKind);
  PTree::Node* createCompositeStateFunctionGetter(const std::string& stateFunctionKind);
  PTree::Node* createConstructor();
  PTree::Node* createDestructor();
};

#endif // !CRALGORITHM_GENERATOR_CHOOSE_CLASS_H_
