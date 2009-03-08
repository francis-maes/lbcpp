/*-----------------------------------------.---------------------------------.
| Filename: PrettyPrintVisitor.h           | Pretty Print Visitor            |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 20:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_TOOLS_PRETTY_PRINT_VISITOR_H_
# define CRALGO_TOOLS_PRETTY_PRINT_VISITOR_H_

# include "RecursiveVisitor.h"
# include "../language/CRAlgoPTree.h"
# include "../language/CRAlgoLexer.h"
# include "CodePrinter.h"
# include <typeinfo>

extern std::string typeIdName(char const *mangled);

class PrettyPrintVisitor : public PTree::Visitor
{
public:
  PrettyPrintVisitor(CodePrinter& printer)
    : printer(printer), insideChoose(false) {}

  void prettyPrint(PTree::Node* node)
  {
    node->accept(this);
    printer.flush();
  }

  virtual void visit(PTree::Atom* a)
    {addToken(a);}
  
  virtual void visit(PTree::List* l) 
  {
    currentNodes.push_back(l);
    for (PTree::Iterator it(l); !it.empty(); ++it)
    {
      PTree::Node* node = *it;
      if (node)
        node->accept(this);
    }    
    currentNodes.pop_back();
  }

  virtual void visit(PTree::Brace* l)
  {
    currentNodes.push_back(l);
    addToken(PTree::first(l));
    bool doIndentation = !insideChoose && PTree::first(l);
    if (doIndentation)
      printer.indent();
    PTree::Node* content = PTree::second(l);
    if (content)
      content->accept(this);
    if (doIndentation)
      printer.dedent();
    addToken(PTree::third(l));
    currentNodes.pop_back();
  }
  
  void visitIndentableStatement(PTree::Node* node)
  {
    currentNodes.push_back(node);
    
    // normal for the l-1 first childrens
    int l = PTree::length(node);
    assert(l > 0);
    PTree::Iterator it(node);
    for (int i = 0; i < l - 1; ++i, ++it)
    {
      assert(!it.empty());
      PTree::Node* children = *it;
      if (children)
        children->accept(this);
    }
    
    // special treatment for the last children
    assert(!it.empty());
    PTree::Node* last = *it;
    if (dynamic_cast<PTree::Brace* >(last))
      last->accept(this);
    else
    {
      printer.indent();
      printer.newLine();
      last->accept(this);
      printer.dedent();
      printer.newLine();
    }
    currentNodes.pop_back();
  }
  
  virtual void visit(PTree::ForStatement* node)
    {visitIndentableStatement(node);}
  virtual void visit(PTree::IfStatement* node)
    {visitIndentableStatement(node);}
  virtual void visit(PTree::WhileStatement* node)
    {visitIndentableStatement(node);}
  virtual void visit(PTree::CaseStatement* node)
    {visitIndentableStatement(node);}
  
  virtual void visit(PTree::UserStatementExpr* node)
  {
    if (dynamic_cast<CRAlgo::ChooseExpression* >(node))
    {
      bool oInsideChoose = insideChoose;
      insideChoose = true;
      PTree::Visitor::visit(node);
      insideChoose = oInsideChoose;
    }
    else
      PTree::Visitor::visit(node);
  }

  virtual void visit(PTree::FunctionDefinition* node)
  {
    visit(static_cast<PTree::List* >(node));
//    if (dynamic_cast<PTree::FunctionDefinition* >(currentNodes.back()))
      printer.newLine();
  }

protected:
  CodePrinter& printer;
  
  bool insideChoose;
  std::vector<PTree::Node* > previousNodes;
  std::vector<PTree::Node* > currentNodes;
  
  static void displayNodeStack(const std::vector<PTree::Node* >& stack, std::ostream& ostr)
  {
    for (size_t i = 0; i < stack.size(); ++i)
    {
      ostr << typeIdName(typeid(*stack[i]).name());
      if (i < stack.size() - 1)
        ostr << " -> ";
    }
  }
  
  void addToken(PTree::Node* atom)
  {
    if (atom)
    {
      assert(atom->is_atom());
      
      currentNodes.push_back(atom);
      addSeparator(previousNodes, currentNodes);
      previousNodes = currentNodes;
      currentNodes.pop_back();
      
      printer.write(atom->position(), atom->length());
    }
  }
  
  void addSeparator(const std::vector<PTree::Node* >& before, const std::vector<PTree::Node* >& after)
  {
    if (!before.size())
      return;
    
    bool doSpace = true;
    bool doNewline = false;
    
#define isInstanceOf(Var, ClassName) \
    (dynamic_cast<ClassName* >(Var))
  
#define beforeIs(ClassName) \
    isInstanceOf(before.back(), ClassName)
#define beforeIsAtom(AtomString) \
    (before.back() && before.back()->is_atom() && (*before.back()) == (AtomString))
#define beforeStartsWith(Str) \
    (PTree::reify(before.back()).find(Str) == 0)
#define beforeAncestorIs(ClassName, AncestorNum) \
    (before.size() > (AncestorNum) && isInstanceOf(before[before.size() - (AncestorNum) - 1], ClassName))
#define beforeParentIs(ClassName) \
   beforeAncestorIs(ClassName, 1)     

#define afterIs(ClassName) \
    isInstanceOf(after.back(), ClassName)
#define afterIsAtom(AtomString) \
    (after.back() && after.back()->is_atom() && (*after.back()) == (AtomString))
#define afterAncestorIs(ClassName, AncestorNum) \
    (after.size() > ((AncestorNum) - 1) && isInstanceOf(after[after.size() - (AncestorNum) - 1], ClassName))
#define afterParentIs(ClassName) \
   afterAncestorIs(ClassName, 1)     
    
    bool afterIsOParent = afterIsAtom("(") || afterIsAtom("[");
    bool afterIsCParent = afterIsAtom(")") || afterIsAtom("]");
    bool beforeIsOParent = beforeIsAtom("(") || beforeIsAtom("[");
//    bool beforeIsCParent = beforeIsAtom(")") || beforeIsAtom("]");
    
    /*
    ** After
    */
    if (afterIsOParent)
    {
      doSpace = beforeIs(PTree::Keyword) || beforeIsAtom(CRAlgoToken::featureSense);
//      if (!beforeIs(PTree::Keyword)) //beforeIs(PTree::Identifier) && beforeParentIs(PTree::Declarator) && afterParentIs(PTree::Declarator))
//        doSpace = false;
      
    }
    else if (afterIsCParent)
    {
      doSpace = false;
    }
    else if (afterIsAtom("{") && !insideChoose)
    {
      doNewline = true;
    }
    else if (afterIsAtom("}") && !insideChoose)
    {
      doNewline = true;
    }
    else if (afterIsAtom("::") || afterIsAtom(".") || afterIsAtom("->") || afterIsAtom(";") || afterIsAtom(","))
      doSpace = false;
    else if (afterIsAtom(":"))
    {
      doSpace = afterParentIs(PTree::ClassSpec) || afterParentIs(PTree::CondExpr);
    }
    
    if (!beforeParentIs(PTree::AccessSpec) && afterParentIs(PTree::AccessSpec))
      printer.newLine();
      
    /*
    ** Before
    */
    if (beforeIsAtom("{") && !insideChoose)
    {
      if (!afterIsAtom("{"))
        doNewline = true;
    }
    else if (beforeIsAtom("}") && !insideChoose)
    {
      if (!afterIsAtom("{") && !afterIsAtom(";"))
        doNewline = true;      
    }
    else if (beforeIsAtom("->") || beforeIsAtom(".") || beforeIsAtom("::") || beforeIsAtom("++") || beforeIsOParent)
      doSpace = false;
    else if (beforeIsAtom(";"))
    {
      doNewline = (!beforeAncestorIs(PTree::ForStatement, 2) && !beforeParentIs(PTree::ForStatement)) || afterIsAtom("}");
    }
    else if (beforeIsAtom(":"))
    {
      doNewline = !beforeParentIs(PTree::CondExpr) && !beforeAncestorIs(PTree::ClassSpec, 2);
    }
    else if (beforeStartsWith("//") || beforeIsAtom("\n"))
      doSpace = false;
    
    static std::ofstream ostr("/Users/francis/Projets/Nieme/trunk/src/problems/cralgorithms/CRAlgorithms/separators.txt");
    ostr << "ADD-SEPARATOR" << std::endl;
    ostr << "  Before = "; displayNodeStack(before, ostr); ostr << " (" << PTree::reify(before.back()) << ")" << std::endl;
    ostr << "  After = "; displayNodeStack(after, ostr); ostr << " (" << PTree::reify(after.back()) << ")" << std::endl;
    ostr << "  => space: " << doSpace << " newLine: " << doNewline << std::endl;
    ostr << std::endl;
    
    if (doNewline)
      printer.newLine();
    else if (doSpace)
      printer.write(" ");
  }
};

inline void prettyPrint(PTree::Node* node, CodePrinter& printer)
  {PrettyPrintVisitor visitor(printer); visitor.prettyPrint(node);}
  
inline std::string prettyPrint(PTree::Node* node)
{
  std::ostringstream ostr;
  CodePrinter printer(ostr);
  prettyPrint(node, printer);
  return ostr.str();
}

#endif // !CRALGO_TOOLS_PRETTY_PRINT_VISITOR_H_
