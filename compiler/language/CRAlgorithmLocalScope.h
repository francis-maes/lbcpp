/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmLocalScope.h        | CR-algorithm local scope        |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 22:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_LANGUAGE_LOCAL_SCOPE_H_
# define CRALGORITHM_LANGUAGE_LOCAL_SCOPE_H_

class CRAlgorithmLocalScope
{
public:
  CRAlgorithmLocalScope(PTree::Node* node, SymbolLookup::LocalScope* scope)
    : node(node), scope(scope), number(0) {}
    
  void setNumber(size_t number)
    {this->number = number;}
    
  void setName(const std::string& name)
    {this->name = name;}

  size_t getNumber() const
    {return number;}
    
  std::string getName() const
    {return name;}

  SymbolLookup::LocalScope* getScope() const
    {return scope;}
    
  PTree::Node* getPTree() const
    {return node;}

  std::string getClassName() const
    {return "__LocalScope" + name + "__";}

  std::string getVariableName() const
    {return "__localScope" + name + "__";}
  
  std::string getLabelName() const
    {return "__localScope" + name + "_label__";}

  bool operator <(const CRAlgorithmLocalScope& otherScope) const
  {
    const PTree::Node* n1 = node;
    const PTree::Node* n2 = otherScope.node;
    if (n1->begin() != n2->begin())
      return n1->begin() < n2->begin();
    return n1->end() < n2->end();
  }

private:
  PTree::Node* node;
  SymbolLookup::LocalScope* scope;
  size_t number;
  std::string name;
};

#endif // !CRALGORITHM_LANGUAGE_LOCAL_SCOPE_H_
