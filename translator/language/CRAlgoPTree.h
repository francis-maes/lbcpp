/*-----------------------------------------.---------------------------------.
| Filename: CRAlgoPtree.h                  | CR-Algorithm PTree user nodes   |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 11:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LANGUAGE_PTREE_H_
# define LBCPP_LANGUAGE_PTREE_H_

# include "../common.h"

namespace CRAlgo
{

class ChooseExpression : public PTree::UserStatementExpr
{
public:
  ChooseExpression(PTree::Node* chooseKeyword, PTree::Node* oBracket, PTree::Node* typeName, PTree::Node* cBracket, PTree::Node* oParent, PTree::Node* arguments, PTree::Node* cParent)
    : PTree::UserStatementExpr(chooseKeyword, PTree::list(oBracket, typeName, cBracket, oParent, arguments, cParent)) {}
  ChooseExpression(Node *p, Node *q) : PTree::UserStatementExpr(p, q) {}

  PTree::Node* getArguments()
    {return PTree::nth(this, 5);}
};

class StateFundefStatement : public PTree::UserStatement
{
public:
  StateFundefStatement(Node *p, Node *q) : PTree::UserStatement(p, q) {}
  
  PTree::FunctionDefinition* getFunctionDefinition() const
    {return dynamic_cast<PTree::FunctionDefinition*>(const_cast<PTree::Node* >(PTree::second(this)));}
  
  PTree::Declarator* getDeclarator() const
    {return dynamic_cast<PTree::Declarator* >(PTree::third(getFunctionDefinition()));}
  
  std::string getIdentifier() const
    {return PTree::reify(PTree::first(getDeclarator()));}
    
  std::string getKind() const;
};

class FeatureScopeStatement : public PTree::UserStatement
{
public:
  FeatureScopeStatement(PTree::Node* featureScopeKeyword, PTree::Node* oParent, PTree::Node* arguments, PTree::Node* cParent, PTree::Node* body)
    : PTree::UserStatement(featureScopeKeyword, PTree::list(oParent, arguments, cParent, body)) {}
  FeatureScopeStatement(Node *p, Node *q) : PTree::UserStatement(p, q) {}
    
  PTree::Node* getArguments()
    {return PTree::third(this);}
    
  PTree::Node* getBody()
    {return PTree::nth(this, 4);}
};

class CRAlgorithmCallExpr : public PTree::UserStatementExpr
{
public:
  CRAlgorithmCallExpr(Node* p, Node* q) : PTree::UserStatementExpr(p, q) {}

  PTree::Node* getFuncallExpr()
    {return PTree::nth(this, PTree::length(this) - 1);}

  PTree::Node* getIdentifier()
    {return PTree::first(getFuncallExpr());}
    
  PTree::Node* getArguments()
    {return PTree::third(getFuncallExpr());}
};

class FeatureGeneratorCallStatement : public PTree::UserStatement
{
public:
  FeatureGeneratorCallStatement(PTree::Node* featureCallKeyword, PTree::Node* identifier, PTree::Node* expr, PTree::Node* semiColon, PTree::Node* inlineKeyword = NULL)
    : PTree::UserStatement(featureCallKeyword, PTree::list(identifier, expr, semiColon, inlineKeyword)) {}
  FeatureGeneratorCallStatement(Node* p, Node* q) : PTree::UserStatement(p, q) {}
  
  PTree::Node* getIdentifier()
    {return PTree::second(this);}

  PTree::Node* getExpression()
    {return PTree::third(this);}
  
  bool isFunctionCall()
    {return dynamic_cast<PTree::FuncallExpr* >(getExpression()) != NULL;}
    
  bool isInlineCall()
    {PTree::Node* inlineKeyword = PTree::nth(this, 4); return inlineKeyword && *inlineKeyword == "inline";}
};

extern bool isCRAlgorithm(PTree::FunctionDefinition* node);
extern bool isCRAlgorithmType(PTree::Node* type, std::string* crAlgorithmIdentifier = NULL); // crAlgorithm<id>
extern bool isFeatureGenerator(PTree::FunctionDefinition* node);

extern bool isReward(PTree::Node* identifier);
extern bool isFeatureSense(PTree::Node* identifier);


}; /* namespace CRAlgo */

#endif // !LBCPP_LANGUAGE_PTREE_H_
