/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmChoose.h            | CR-algorithm choose             |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 22:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_LANGUAGE_CHOOSE_H_
# define CRALGORITHM_LANGUAGE_CHOOSE_H_

# include "../tools/PTreeGenerator.h"
# include "CRAlgoPTree.h"

class RewriteVisitor;
class CRAlgorithmChoose
{
public:
  CRAlgorithmChoose(CRAlgo::ChooseExpression* node, SymbolLookup::Scope* scope, PTree::Node* parentStatementOrDeclaration, size_t number);
  
  CRAlgo::ChooseExpression* getPTree() const
    {return node;}
    
  PTree::Node* getParentStatementOrDeclaration() const
    {return parentStatementOrDeclaration;}
    
  void setParentStatementOrDeclaration(PTree::Node* node)
    {parentStatementOrDeclaration = node;}

  PTree::Node* getChooseType() const
    {return PTree::third(node);}
    
  size_t getChooseNumber() const
    {return number;}

  std::string getContainerType() const;

  /*
  ** Names
  */
  std::string getName() const
    {return PTreeGenerator::size2str(number);}

  std::string getParametersClassName() const
    {return "__Choose" + getName() + "Parameters__";}
    
  std::string getLabelName() const
    {return "__choose" + getName() + "_label__";}

  std::string getResultVariableName() const
    {return "__choose" + getName() + "_result__";}
  
  /*
  ** Arguments
  */
  std::vector<PTree::Node* > getArguments() const;
  
  PTree::Node* getChoiceNode() const
    {return choiceNode;}

  size_t getNumStateFunctions() const
    {return stateFunctions.size();}
    
  PTree::Node* getStateFunctionArgument(size_t index) const
    {assert(index < stateFunctions.size()); return stateFunctions[index].first;}
    
  CRAlgo::StateFundefStatement* getStateFunctionDefinition(size_t index) const
    {assert(index < stateFunctions.size()); return stateFunctions[index].second;}
  
private:
  CRAlgo::ChooseExpression* node;
  SymbolLookup::Scope* scope;
  PTree::Node* parentStatementOrDeclaration;
  size_t number;
  
  std::vector<std::pair<PTree::Node*, CRAlgo::StateFundefStatement* > > stateFunctions; // (argument, stateFunction definition) pairs
  PTree::Node* choiceNode;
  
  static CRAlgo::StateFundefStatement*
    findStateFunctionRecursively(const std::string& identifier, const SymbolLookup::Scope* scope);
};


#endif // !CRALGORITHM_LANGUAGE_CHOOSE_H_
