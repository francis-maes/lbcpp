/*-----------------------------------------.---------------------------------.
| Filename: ReferenceToPointerRewriteVi...h| A visitor that (tries to)       |
| Author  : Francis Maes                   | transform references into       |
| Started : 03/03/2009 20:50               | pointers.                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_TOOLS_REFERENCE_TO_POINTER_REWRITE_VISITOR_H_
# define CRALGORITHM_TOOLS_REFERENCE_TO_POINTER_REWRITE_VISITOR_H_

# include "PTreeAnalyser.h"
# include "ScopeBasedRewriteVisitor.h"

class ReferenceToPointerRewriteVisitor : public ScopeBasedRewriteVisitor
{
public:
  PTree::Node* transformReferencesIntoPointers(PTree::Node* node, SymbolLookup::Scope* scope)
  {
    pushScope(scope);
    std::cout << "====================" << std::endl << std::endl;
    PTree::display(node, std::cout, false, true);
    std::cout << std::endl << " ===> " << std::endl;
    PTree::Node* res = rewrite(node);
    PTree::display(res, std::cout, false, true);
    std::cout << std::endl << std::endl;
    popScope();
    return res;
  }

  virtual void visit(PTree::Declaration* node)
  {
    DeclarationPTreeAnalyser input(node);
    ParamDeclarationListPTreeGenerator output;
    
    output.addModifiers(input.getModifiers());
    output.setType(input.getType());
    
    std::vector<ParameterPTreeAnalyser> variables = input.getVariables();
    for (size_t i = 0; i < variables.size(); ++i)
    {
      ParameterPTreeAnalyser& var = variables[i];
      output.addDeclarator(var.isReference()
        ? rewriteReferenceDeclarator(var.getDeclarator())
        : var.getDeclarator());
    }
    
    setResult(output.createDeclaration());
  }
  
  virtual void visit(PTree::DotMemberExpr* node)
  {
    setResult(new PTree::DotMemberExpr(rewrite(PTree::first(node)),
      PTree::list(PTree::second(node), PTree::third(node))));
  }
  
  virtual void visit(PTree::ArrowMemberExpr* node)
  {
    setResult(new PTree::ArrowMemberExpr(rewrite(PTree::first(node)),
      PTree::list(PTree::second(node), PTree::third(node))));
  }
  
  virtual void visit(PTree::Identifier* node)
  {
    const SymbolLookup::VariableName* variableName = 
      dynamic_cast<const SymbolLookup::VariableName* >(simpleSymbolLookup(getCurrentScope(), node));
    if (variableName)
    {
      ParameterPTreeAnalyser input(NULL, (PTree::Declarator* )variableName->ptree());
      if (input.isReference())
      {
        // id => (&id)
        setResult(new PTree::ParenExpr(new PTree::Atom("(", 1), 
            PTree::list(new PTree::UnaryExpr(new PTree::Atom("&", 1), PTree::list(node)), new PTree::Atom(")", 1))));
        return;
      }
    }
    
    setResult(node);
  }
  
private:
  PTree::Declarator* rewriteReferenceDeclarator(PTree::Declarator* declarator)
  {
    std::vector<PTree::Node* > input, output;
    PTree::Identifier* identifier = NULL;

    for (PTree::Iterator it(declarator); !it.empty(); ++it)
      input.push_back(*it);
    for (size_t i = 0; i < input.size(); ++i)
    {
      std::cout << "INPUT: '" << PTree::reify(input[i]) << "' ==> OUTPUT '";
      if (identifier)
        output.push_back(rewrite(input[i]));
      else if (input[i]->is_atom() && *input[i] == "&")
        output.push_back(new PTree::Atom("*", 1));
      else
      {
        PTree::Identifier* id = dynamic_cast<PTree::Identifier* >(input[i]);
        if (id)
          identifier = id;
        output.push_back(input[i]);
      }
      std::cout << PTree::reify(output.back()) << " (" << output.size() << ")" << std::endl;
    }
    return PTreeGenerator().listT<PTree::Declarator>(output);
  }
};

#endif // !CRALGORITHM_TOOLS_REFERENCE_TO_POINTER_REWRITE_VISITOR_H_
