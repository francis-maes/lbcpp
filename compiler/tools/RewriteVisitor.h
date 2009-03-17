/*-----------------------------------------.---------------------------------.
| Filename: RewriteVisitor.h               | Rewrite Visitor BaseClass       |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 17:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_TOOLS_REWRITE_VISITOR_H_
# define CRALGO_TOOLS_REWRITE_VISITOR_H_

# include "../common.h"
# include "../language/CRAlgoPTree.h"

class RewriteVisitor : public PTree::Visitor
{
public:
  PTree::Node* rewrite(PTree::Node* node)
  {
    if (node)
    {
      node->accept(this);
      return result;
    }
    return NULL;
  }
  
  virtual void visit(PTree::Atom* node)
    {setResult(node);}
    
  virtual void visit(PTree::Declarator* node)
    {visitList((PTree::List* )node); setResult(new PTree::Declarator(getResult(), node->encoded_type(), node->encoded_name(), node->name()));}

  virtual void visit(PTree::Name* node)
    {visitList((PTree::List* )node); setResult(new PTree::Name(getResult(), node->encoded_name()));}

  virtual void visit(PTree::ClassSpec* node)
    {setResult(new PTree::ClassSpec(node->encoded_name(), rewrite(node->car()), rewrite(node->cdr()), node->get_comments()));}

#define defaultListDefinition(T) \
  virtual void visit(PTree:: T * node) {visitList(node);}
  
  // default
  defaultListDefinition(List);

  // braces
  defaultListDefinition(Brace);
  defaultListDefinition(Block);
  defaultListDefinition(ClassBody);
  
  // misc
  defaultListDefinition(TemplateDecl);
  defaultListDefinition(TemplateInstantiation);
  defaultListDefinition(ExternTemplate);
  defaultListDefinition(MetaclassDecl);
  defaultListDefinition(LinkageSpec);
  defaultListDefinition(NamespaceSpec);

  // declarations
  defaultListDefinition(Declaration);
  defaultListDefinition(Typedef);
  defaultListDefinition(UsingDirective);
  defaultListDefinition(UsingDeclaration);
  defaultListDefinition(NamespaceAlias);
  defaultListDefinition(FunctionDefinition);
  defaultListDefinition(ParameterDeclaration);  

  // misc
  //defaultListDefinition(FstyleCastExpr);
  defaultListDefinition(EnumSpec);
  defaultListDefinition(TypeParameter);
  defaultListDefinition(AccessSpec);
  defaultListDefinition(AccessDecl);
  defaultListDefinition(UserAccessSpec);
  defaultListDefinition(UserdefKeyword);

  // statements
  defaultListDefinition(IfStatement);
  defaultListDefinition(SwitchStatement);
  defaultListDefinition(WhileStatement);
  defaultListDefinition(DoStatement);
  defaultListDefinition(ForStatement);
  defaultListDefinition(TryStatement);
  defaultListDefinition(BreakStatement);
  defaultListDefinition(ContinueStatement);
  defaultListDefinition(ReturnStatement);
  defaultListDefinition(GotoStatement);
  defaultListDefinition(CaseStatement);
  defaultListDefinition(DefaultStatement);
  defaultListDefinition(LabelStatement);
  defaultListDefinition(ExprStatement);
  
  // expressions
  defaultListDefinition(Expression);
  defaultListDefinition(AssignExpr);
  defaultListDefinition(CondExpr);
  defaultListDefinition(InfixExpr);
  defaultListDefinition(PmExpr);
  defaultListDefinition(CastExpr);
  defaultListDefinition(UnaryExpr);
  defaultListDefinition(ThrowExpr);
  defaultListDefinition(SizeofExpr);
  defaultListDefinition(OffsetofExpr);
  defaultListDefinition(TypeidExpr);
  defaultListDefinition(TypeofExpr);
  defaultListDefinition(NewExpr);
  defaultListDefinition(DeleteExpr);
  defaultListDefinition(ArrayExpr);
  defaultListDefinition(PostfixExpr);
  defaultListDefinition(DotMemberExpr);
  defaultListDefinition(ArrowMemberExpr);
  defaultListDefinition(ParenExpr);
  defaultListDefinition(FuncallExpr);
  
    // chooseFunction, featureScope, featureCall
  virtual void visit(PTree::UserStatement* node)
  {
    CRAlgo::StateFundefStatement* chooseFunction = dynamic_cast<CRAlgo::StateFundefStatement* >(node);
    if (chooseFunction)
    {
      visitList(chooseFunction);
      return;
    }
    CRAlgo::FeatureScopeStatement* featureScope = dynamic_cast<CRAlgo::FeatureScopeStatement* >(node);
    if (featureScope)
    {
      visitList(featureScope);
      return;
    }
    CRAlgo::FeatureGeneratorCallStatement* featureGeneratorCall = dynamic_cast<CRAlgo::FeatureGeneratorCallStatement* >(node);
    if (featureGeneratorCall)
    {
      visitList(featureGeneratorCall);
      return;
    }
    visitList(node);
  }


  // crAlgorithmCall
  virtual void visit(PTree::UserStatementExpr* node)
  {
    CRAlgo::CRAlgorithmCallExpr* crAlgorithmCall = dynamic_cast<CRAlgo::CRAlgorithmCallExpr* >(node);
    if (crAlgorithmCall)
    {
      visitList(crAlgorithmCall);
      return;
    }
    CRAlgo::ChooseExpression* chooseExpr = dynamic_cast<CRAlgo::ChooseExpression* >(node);
    if (chooseExpr)
    {
      visitList(chooseExpr);
      return;
    }
    visitList(node);
  }

  // todo: continue to be exhaustive
  
#undef defaultListDefinition

protected:
  template<class T>
  void visitList(T* node)
  {
    PTree::Node* car = rewrite(node->car());
    PTree::Node* cdr = rewrite(node->cdr());
    setResult(new T(car, cdr));      
  }
  
  void setResult(PTree::Node* result)
    {this->result = result;}
  
  PTree::Node* getResult() const
    {return result;}
  
private:
  PTree::Node* result;
};

#endif // !CRALGO_TOOLS_REWRITE_VISITOR_H_
