/*-----------------------------------------.---------------------------------.
| Filename: RecursiveVisitor.h             | Recursive Visitor BaseClass     |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 17:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_TOOLS_RECURSIVE_VISITOR_H_
# define CRALGO_TOOLS_RECURSIVE_VISITOR_H_

# include "../common.h"

class RecursiveVisitor : public PTree::Visitor
{
public:
  RecursiveVisitor() : visitorBreaked(false) {}
  
  virtual void visit(PTree::List* node)
  {
    if (node->car() && !visitorBreaked)
      node->car()->accept(this);
    if (node->cdr() && !visitorBreaked)
      node->cdr()->accept(this);
  }
  
  void visitRecursively(PTree::Node* node)
  {
    assert(node);
    visitorBreaked = false;
    node->accept(this);
  }
  
protected:
  void breakVisitor()
    {visitorBreaked = true;}
  
  bool isVisitorBreaked() const
    {return visitorBreaked;}
    
private:
  bool visitorBreaked;
};

#endif // !CRALGO_TOOLS_RECURSIVE_VISITOR_H_
