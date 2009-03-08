/*-----------------------------------------.---------------------------------.
| Filename: PTreeGenerator.h               | Base class for code generators  |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2009 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_TOOLS_GENERATOR_PTREE_H_
# define CRALGORITHM_TOOLS_GENERATOR_PTREE_H_

# include "../common.h"

class PTreeGenerator
{
public:
  typedef std::vector<PTree::Node* > NodeVector;
  typedef std::vector<std::pair<PTree::Node*, PTree::Node*> > NodePairVector;
  
  const char* registerUserString(const std::string& str);
  
  /*
  ** Constant Keywords
  */
  PTree::Keyword* ifKeyword();
  PTree::Keyword* switchKeyword();
  PTree::Keyword* caseKeyword();
  PTree::Keyword* defaultKeyword();
  PTree::Keyword* gotoKeyword();
  PTree::Keyword* breakKeyword();
  PTree::Keyword* returnKeyword();
  PTree::Keyword* thisKeyword();
  PTree::Keyword* classKeyword();
  PTree::Keyword* structKeyword();
  PTree::Keyword* publicKeyword();
  PTree::Keyword* protectedKeyword();
  PTree::Keyword* privateKeyword();
  PTree::Keyword* virtualKeyword();
  PTree::Keyword* staticKeyword();
  PTree::Keyword* constKeyword();
  PTree::Keyword* voidKeyword();
  PTree::Keyword* intKeyword();
  PTree::Keyword* doubleKeyword();
  PTree::Keyword* charKeyword();
  PTree::Keyword* typedefKeyword();
  PTree::Keyword* newKeyword();
  PTree::Keyword* deleteKeyword();
  PTree::Keyword* elseKeyword();
  PTree::Keyword* continueKeyword();
  PTree::Keyword* inlineKeyword();
  PTree::Keyword* enumKeyword();
  
  /*
  ** Constant Atoms
  */
  PTree::Atom* newlineAtom();
  PTree::Atom* commaAtom();
  PTree::Atom* oParentAtom();
  PTree::Atom* cParentAtom();
  PTree::Atom* colonAtom();
  PTree::Atom* semiColonAtom();
  PTree::Atom* rightArrowAtom();
  PTree::Atom* dotAtom();
  PTree::Atom* oBraceAtom();
  PTree::Atom* cBraceAtom();
  PTree::Atom* assignAtom();
  PTree::Atom* equalAtom();
  PTree::Atom* notEqualAtom();
  PTree::Atom* doubleSlashAtom();
  PTree::Atom* questionMarkAtom();

  /*
  ** User constant atoms
  */
  inline PTree::Atom* atom(const char* text)
    {return new PTree::Atom(text, strlen(text));}

  inline PTree::Node* comment(const char* text)
    {return atom(std::string("// ") + text + "\n");}

  inline PTree::Identifier* identifier(const char* text)
    {return new PTree::Identifier(text, strlen(text));}

  /*
  ** User string atoms
  */
  PTree::Atom* atom(const std::string& text);
  PTree::Identifier* identifier(const std::string& text);
  PTree::Literal* literal(const std::string& text);
  PTree::Literal* literal(size_t number)
    {return literal(size2str(number));}

  /*
  ** List creators
  */
  PTree::List* list(PTree::Node* p1)
    {return PTree::list(p1);}
  PTree::List* list(PTree::Node* p1, PTree::Node* p2)
    {return PTree::list(p1, p2);}
  PTree::List* list(PTree::Node* p1, PTree::Node* p2, PTree::Node* p3)
    {return PTree::list(p1, p2, p3);}
  PTree::List* list(PTree::Node* p1, PTree::Node* p2, PTree::Node* p3, PTree::Node* p4)
    {return PTree::list(p1, p2, p3, p4);}
  PTree::List* list(PTree::Node* p1, PTree::Node* p2, PTree::Node* p3, PTree::Node* p4, PTree::Node* p5)
    {return PTree::list(p1, p2, p3, p4, p5);}

  PTree::List* list(const NodeVector& nodes, size_t firstIndex = 0)
    {return listT<PTree::List>(nodes, firstIndex);}
  
  template<class T>
  inline T* listT(const NodeVector& nodes, size_t firstIndex = 0)
    {return firstIndex < nodes.size() ? new T(nodes[firstIndex], list(nodes, firstIndex + 1)) : NULL;}
  
  /*
  ** Statements
  */
  PTree::ExprStatement* exprStatement(PTree::Node* expr, bool includeSemiColon = true)
    {return new PTree::ExprStatement(expr, includeSemiColon ? list(semiColonAtom()) : NULL);}

  PTree::ReturnStatement* returnStatement(PTree::Node* returnValue = NULL)
    {return new PTree::ReturnStatement(returnKeyword(),
        returnValue ? list(returnValue, semiColonAtom()) : list(semiColonAtom()));}

  PTree::IfStatement* ifStatement(PTree::Node* condition, PTree::Node* thenBlock, PTree::Node* elseBlock = NULL);

  // goto [label];
  PTree::GotoStatement* gotoStatement(PTree::Node* label)
    {return new PTree::GotoStatement(gotoKeyword(), list(label, semiColonAtom()));}

  // [label]:
  PTree::LabelStatement* labelStatement(PTree::Node* label)
    {return new PTree::LabelStatement(label, list(colonAtom()));}
  
  // break;
  PTree::BreakStatement* breakStatement()
    {return new PTree::BreakStatement(breakKeyword(), list(semiColonAtom()));}

  // continue;
  PTree::ContinueStatement* continueStatement()
    {return new PTree::ContinueStatement(continueKeyword(), list(semiColonAtom()));}
  
  /*
  ** Declarations
  */
  PTree::ParameterDeclaration* parameterDeclaration(PTree::Node* type, PTree::Node* identifier, PTree::Node* initialValue = NULL);
  
  /*
  ** Expressions
  */
  PTree::FuncallExpr* funcallExpr(PTree::Node* identifier, PTree::Node* arguments = NULL);
  PTree::ArrowMemberExpr* arrowMemberExpr(PTree::Node* left, PTree::Node* right);
  PTree::DotMemberExpr* dotMemberExpr(PTree::Node* left, PTree::Node* right);
  PTree::AssignExpr* assignExpr(PTree::Node* left, PTree::Node* right);
  PTree::CondExpr* condExpr(PTree::Node* condition, PTree::Node* truePart, PTree::Node* falsePart);
  PTree::InfixExpr* infixExpr(PTree::Node* left, PTree::Node* operation, PTree::Node* right);
  
  /*
  ** Misc
  */
  NodeVector removeTokenFromNodeList(PTree::Node* list, const char* tokenToRemove);

  static inline std::string size2str(size_t i)
    {std::ostringstream ostr; ostr << i; return ostr.str();}
  static inline std::string quote(const std::string& str)
    {return "\"" + str + "\""; /* todo: ameliorer */ }
};

class IdentifiedPTreeGenerator : public PTreeGenerator
{
public:
  IdentifiedPTreeGenerator()
    : ptreeIdentifier(NULL) {}

  void setName(const std::string& className)
    {ptreeIdentifier = identifier(className);}
  
  void setIdentifier(PTree::Node* identifier)
    {ptreeIdentifier = identifier;}

  PTree::Identifier* getIdentifier() const
    {return dynamic_cast<PTree::Identifier* >(ptreeIdentifier);}

protected:
  PTree::Node* ptreeIdentifier;
};

class BlockPTreeGenerator : public PTreeGenerator
{
public:
  void add(const NodeVector& nodes);
  void add(PTree::Node* node);
  void addComment(const char* commentLine);
  void addNewLine();
  void addAccessSpecifier(PTree::Keyword* keyword);
  void addVariableDeclaration(PTree::Node* type, PTree::Node* identifier, PTree::Node* initialValue = NULL);
  void addExpressionStatement(PTree::Node* expression);
  
  PTree::Node* createContent();
  PTree::Block* createBlock();
  
  size_t size() const
    {return nodes.size();}

  PTree::Node*& operator [](size_t index)
    {assert(index < nodes.size()); return nodes[index];}

  bool isEmpty() const
    {return nodes.size() == 0;}

protected:
  NodeVector nodes;
};

class ParamDeclarationListPTreeGenerator : public PTreeGenerator
{
public:
  ParamDeclarationListPTreeGenerator() : type(NULL) {}

  void addModifier(PTree::Node* modifier);
  void addModifiers(PTree::Node* modifiers);
  void addModifiers(const NodeVector& modifiers);

  void setType(PTree::Node* type)
    {this->type = type;}
    
  void addDeclarator(PTree::Declarator* node);  

  PTree::Declaration* createDeclaration();
  
private:
  NodeVector modifiers;
  PTree::Node* type;
  NodeVector declarators;
};

class ClassPTreeGenerator : public IdentifiedPTreeGenerator
{
public:
  ClassPTreeGenerator();
  
  void setKeyword(PTree::Keyword* structOrClass);
  void addBaseClass(PTree::Keyword* keyword, PTree::Identifier* identifier);

  PTree::ClassSpec* createSpecification();
  PTree::Declaration* createDeclaration();

  BlockPTreeGenerator body;
  
private:
  PTree::Keyword* keyword; // class or struct
  NodeVector baseClasses;
};

class FunctionPTreeGenerator : public IdentifiedPTreeGenerator
{
public:
  FunctionPTreeGenerator();
  virtual ~FunctionPTreeGenerator() {}
  
  PTree::FunctionDefinition* createDeclaration();
  
  void setReturnType(PTree::Node* returnType);
  void setConst(bool isConst);
  
  void addModifier(PTree::Node* modifier);
  void addModifiers(PTree::Node* modifiers);
  void addModifiers(const NodeVector& modifiers)
    {for (size_t i = 0; i < modifiers.size(); ++i) addModifier(modifiers[i]);}
  
  void addParameter(PTree::Node* type, PTree::Node* identifier, PTree::Node* initialValue = NULL);
  void addParameter(PTree::Node* parameterDeclaration);
  void addParameters(PTree::Node* parametersList);
  void addParameters(const NodeVector& parametersList);

  BlockPTreeGenerator body;

protected:
  bool isConst;
  PTree::Node* returnType;
  NodeVector modifiers;
  NodeVector parameters;
  
  virtual void customizeDeclarator(NodeVector& declarator) {}
};

class ConstructorPTreeGenerator : public FunctionPTreeGenerator
{
public:
  void addInitializer(PTree::Node* identifier, PTree::Node* value, bool onANewLine = false);

protected:
  std::vector<std::pair< std::pair<PTree::Node*, PTree::Node* >, bool > > initializationList;
  
  virtual void customizeDeclarator(NodeVector& declarator);
};

class SwitchPTreeGenerator : public PTreeGenerator
{
public:
  SwitchPTreeGenerator();
  
  void setCondition(PTree::Node* condition);
  void addCase(PTree::Node* caseValue, PTree::Node* caseImpl);
  void setDefault(PTree::Node* defaultImpl);
  
  PTree::Node* createStatement();
  
  size_t getNumCases() const
    {return cases.size();}
  
private:
  PTree::Node* condition;
  BlockPTreeGenerator cases;
};

class FuncallPTreeGenerator : public IdentifiedPTreeGenerator
{
public:
  void addArgument(PTree::Node* identifier);
  void addArguments(PTree::Node* arguments);
  
  PTree::FuncallExpr* createExpression();
  
private:
  NodeVector arguments;
};

#endif // !CRALGORITHM_TOOLS_GENERATOR_PTREE_H_
