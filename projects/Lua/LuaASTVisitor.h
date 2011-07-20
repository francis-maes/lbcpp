/*-----------------------------------------.---------------------------------.
| Filename: LuaASTVisitor.h                | Base class for writing visitors |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 16:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_AST_VISITOR_H_
# define LBCPP_LUA_AST_VISITOR_H_

# include "LuaAST.h"

namespace lbcpp
{

/*
block: { stat* }

stat:
| `Do{ block }
| `Set{ {lhs+} {expr+} }
| `While{ expr block }
| `Repeat{ block expr }
| `If{ (expr block)+ block? }
| `Fornum{ ident expr expr expr? block }
| `Forin{ {ident+} {expr+} block }
| `Local{ {ident+} {expr+}? }
| `Localrec{ {ident+} {expr+}? }
| `Goto{string}
| `Label{string}
| `Return
| `Break
| apply

expr:
| `Nil | `Dots | `True | `False
| `Number{ number }
| `String{ string }
| `Function{ { ident* `Dots? } block } 
| `Table{ ( `Pair{ expr expr } | expr )* }
| `Op{ binopid expr expr } | `Op{ unopid expr }
| `Paren{ expr }
| `Stat{ block expr }
| apply
| lhs

apply:
| `Call{ expr expr* }
| `Invoke{ expr `String{ string } expr* }

lhs: ident | `Index{ expr expr }

ident: `Id{ string }

binopid: "add" | "sub" | "mul"    | "div"
       | "mod" | "pow" | "concat" | "eq"
       | "lt"  | "le"  | "and"    | "or"

unopid:  "not" | "len" | "unm"
*/

class LuaASTVisitor
{
public:
  virtual ~LuaASTVisitor() {}

  // general
  virtual void acceptList(const std::vector<LuaASTNodePtr>& childNodes)
  {
    for (size_t i = 0; i < childNodes.size(); ++i)
      accept(childNodes[i]);
  }

  // expr
  virtual void acceptUnaryOperation(const String& opid, const LuaASTNodePtr& childNode)
    {accept(childNode);}

  virtual void acceptBinaryOperation(const String& opid, const LuaASTNodePtr& leftNode, const LuaASTNodePtr& rightNode)
    {accept(leftNode); accept(rightNode);}

  virtual void acceptParenthesis(const LuaASTNodePtr& childNode)
    {accept(childNode);}


  // ident
  virtual void acceptIdentifier(const String& identifier)
    {}


  // -
  virtual void accept(const LuaASTNodePtr& node)
  {
    const String& tag = node->getTag();
    if (tag == String::empty && node->getNumVariables() == 0)
    {
      acceptList(node->getChildNodes());
      return;
    }
    
    /*
    ** expr
    */
    if (tag == T("Op"))
    {
      String opid = node->getNodeVariable(0).getString();
      if (node->getNumChildNodes() == 1)
        acceptUnaryOperation(opid, node->getChildNode(0));
      else if (node->getNumChildNodes() == 2)
        acceptBinaryOperation(opid, node->getChildNode(0), node->getChildNode(1));
      else
        jassert(false);
      return;
    }

    if (tag == T("Paren"))
    {
      acceptParenthesis(node->getChildNode(0));
      return;
    }

    /*
    ** Ident
    */
    if (tag == T("Id"))
    {
      acceptIdentifier(node->getNodeVariable(0).getString());
      return;
    }

    jassert(false); // not implemented
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_VISITOR_H_

