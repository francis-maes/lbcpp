/*-----------------------------------------.---------------------------------.
| Filename: PTreeAnalyser.h                | PTree analysers                 |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2009 18:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_TOOLS_PTREE_ANALYSER_H_
# define CRALGORITHM_TOOLS_PTREE_ANALYSER_H_

# include "../common.h"

extern const SymbolLookup::Symbol* simpleSymbolLookup(SymbolLookup::Scope* currentScope, PTree::Identifier* id);

class PTreeAnalyser
{
public:
  PTreeAnalyser(PTree::Node* node) : node(node) {}

  PTree::Node* getPTree() const
    {return node;}
    
  std::string getString() const
    {return PTree::reify(node);}
    
  operator PTree::Node* () const
    {return node;}
  
  bool operator <(const PTreeAnalyser& other) const
  {
    const PTree::Node* n1 = getPTree();
    const PTree::Node* n2 = other.getPTree();
    if (n1->begin() != n2->begin())
      return n1->begin() < n2->begin();
    return n1->end() < n2->end();
  }

protected:
  PTree::Node* node;
  
  void assertFalse(PTree::Node* node);
  void assertWithNode(bool condition, PTree::Node* node);
};

class ParameterPTreeAnalyser : public PTreeAnalyser
{
public:
  ParameterPTreeAnalyser(PTree::ParameterDeclaration* node);
  ParameterPTreeAnalyser(PTree::Declaration* declaration, PTree::Declarator* declarator);
  
  PTree::Node* getModifiers() const
    {return PTree::first(declaration);}
  
  PTree::Node* getType() const
    {return type;}
  std::string getTypeString() const
    {return PTree::reify(type);}
    
  PTree::Node* getRecomposedType(bool includeReferences = true) const; // compose the type of declaration and the type modifiers of declarator  

  PTree::Identifier* getIdentifier() const
    {return identifier;}
  std::string getIdentifierString() const
    {return PTree::reify(identifier);}
  
  PTree::Declarator* getDeclarator() const
    {return declarator;}
  
  PTree::Declarator* getDeclaratorWithoutInitialValue() const;
  
  // Depending on includeTypeName:
  // int i = 51; => 51 or 51
  // std::vector<int> v(10, 0) => 10, 0 or std::vector<int>(10,0) 
  PTree::Node* getInitializationArguments(bool includeTypeName) const;
  
  bool isReference() const // returns true if the declarator starts with '&'
  {
    PTree::Node* f = declarator ? PTree::first(declarator) : NULL;
    return f && f->is_atom() && (*f == "&");
  }
  
protected:
  PTree::Node* declaration;
  PTree::Node* modifiers;
  PTree::Node* type;
  PTree::Declarator* declarator;
    int identifierNumber;
    PTree::Identifier* identifier;

  void setDeclaration(PTree::Node* declaration);
  void setDeclarator(PTree::Node* declarator);
};

class ParameterListPTreeAnalyser : public PTreeAnalyser
{
public:
  ParameterListPTreeAnalyser(PTree::Node* node);
  ParameterListPTreeAnalyser() : PTreeAnalyser(NULL) {}
  
  size_t size() const
    {return v.size();}
  
  void pushFront(PTree::ParameterDeclaration* param)
    {v.insert(v.begin(), param);}
    
  void popFront()
    {assert(v.size()); v.erase(v.begin());}
  
  ParameterPTreeAnalyser operator [](size_t i) const
    {assert(i < v.size()); return ParameterPTreeAnalyser(v[i]);}

  const std::vector<PTree::Node* >& getPTreeVector() const
    {return *(const std::vector<PTree::Node* >* )&v;}
  
private:
  std::vector<PTree::ParameterDeclaration* > v;
};

class BlockPTreeAnalyser : public PTreeAnalyser
{
public:
  BlockPTreeAnalyser(PTree::Node* block);
  
  PTree::Node* getContent() const
    {return content;}
  
private:
  PTree::Node* content;
};

class TypePTreeAnalyser : public PTreeAnalyser
{
public:
  TypePTreeAnalyser(PTree::Node* node) : PTreeAnalyser(node) {}

  bool isVoid() const
    {return getString() == "void";}
};

class KeywordsPTreeAnalyser : public PTreeAnalyser
{
public:
  KeywordsPTreeAnalyser(PTree::Node* node);

  bool contains(const char* keyword) const;
};


class FunctionPTreeAnalyser : public PTreeAnalyser
{
public:
  FunctionPTreeAnalyser(PTree::FunctionDefinition* node);
    
  KeywordsPTreeAnalyser getModifiers() const
    {return KeywordsPTreeAnalyser(modifiers);}

  TypePTreeAnalyser getReturnType() const
    {return TypePTreeAnalyser(returnType);}
  std::string getReturnTypeString() const
    {return PTree::reify(returnType);}

  PTree::Node* getIdentifier() const
    {return identifier;}

  std::string getIdentifierString() const
    {return PTree::reify(identifier);}

  bool isPartOfClassImplementation(std::string& classIdentifier, std::string& functionIdentifier) const
  {
    bool res = PTree::length(identifier) == 3
      && dynamic_cast<PTree::Identifier* >(PTree::first(identifier))
      && dynamic_cast<PTree::Atom* >(PTree::second(identifier)) && (*PTree::second(identifier) == "::")
      && dynamic_cast<PTree::Identifier* >(PTree::third(identifier));
    if (!res)
      return false;
    classIdentifier = PTree::reify(PTree::first(identifier));
    functionIdentifier = PTree::reify(PTree::third(identifier));
    return true;
  }

  ParameterListPTreeAnalyser getParameters() const
    {return parameters ? ParameterListPTreeAnalyser(parameters) : ParameterListPTreeAnalyser();}

  bool hasBody() const
    {return body != NULL;}

  BlockPTreeAnalyser getBody() const
    {return BlockPTreeAnalyser(body);}

  bool isConst() const
    {return constKeyword && *constKeyword == "const";}
  
private:
  PTree::Node* modifiers;
  PTree::Node* returnType;
  PTree::Node* declarator;
  PTree::Node*  identifier;
  PTree::Node*  parameters;
  PTree::Node*  constKeyword;
  PTree::Node* body;
};

// variable declaration, e.g. static const int *p1, *p2, *pnull = NULL;
class DeclarationPTreeAnalyser : public PTreeAnalyser
{
public:
  DeclarationPTreeAnalyser(PTree::Declaration* node);
  
  PTree::Node* getType() const
    {return type;}
        
  PTree::Node* getModifiers() const
    {return modifiers;}
  
  std::vector<ParameterPTreeAnalyser> getVariables() const;
  
private:
  PTree::Node* modifiers;
  PTree::Node* type;
  PTree::Node* declarators;
};

#endif // !CRALGORITHM_TOOLS_PTREE_ANALYSER_H_
