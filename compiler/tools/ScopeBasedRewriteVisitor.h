/*-----------------------------------------.---------------------------------.
| Filename: ScopeBasedRewriteVisitor.h     | Rewrite Visitor with current    |
| Author  : Francis Maes                   |  scope                          |
| Started : 18/02/2009 13:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_TOOLS_SCOPE_BASED_REWRITE_VISITOR_H_
# define CRALGO_TOOLS_SCOPE_BASED_REWRITE_VISITOR_H_

# include "RewriteVisitor.h"

class ScopeBasedRewriteVisitor : public RewriteVisitor
{
public:
  ScopeBasedRewriteVisitor(SymbolLookup::Scope* initialScope)
    {pushScope(initialScope);}
  ScopeBasedRewriteVisitor() {}
    
  virtual void visit(PTree::NamespaceSpec* namespaceSpec)
  {
    pushScope(namespaceSpec);
    RewriteVisitor::visit(namespaceSpec);
    popScope();
  }

  virtual void visit(PTree::ClassSpec* classSpec)
  {
    pushScope(classSpec);
    RewriteVisitor::visit(classSpec);
    popScope();
  }

  virtual void visit(PTree::FunctionDefinition* node)
  {
    pushScope(node);
    RewriteVisitor::visit(node);
    popScope();
  }
  
  virtual void visit(PTree::UserStatement* node)
  {
    pushScope(node);
    RewriteVisitor::visit(node);
    popScope();
  }
  
  virtual void visit(PTree::TemplateDecl* node)
  {
    pushScope(node);
    RewriteVisitor::visit(node);
    popScope();
  }

  virtual void visit(PTree::Block* node)
  {
    pushScope(node);
    RewriteVisitor::visit(node);
    popScope();
  }
  
protected:
  void pushScope(PTree::Node* scopeNode)
  {
    assert(scopeNode);
    pushScope(getCurrentScope()->find_scope(scopeNode));
  }

  void pushScope(SymbolLookup::Scope* newScope)
    {scopes.push_back(newScope);}
    
  void popScope()
    {scopes.pop_back();}
  
  SymbolLookup::Scope* getCurrentScope() const
  {
    for (int i = scopes.size() - 1; i >= 0; --i)
      if (scopes[i])
        return scopes[i];
    assert(false);
    return NULL;
  }  

private:
  std::vector<SymbolLookup::Scope* > scopes;
};

#endif // !CRALGO_TOOLS_SCOPE_BASED_REWRITE_VISITOR_H_
