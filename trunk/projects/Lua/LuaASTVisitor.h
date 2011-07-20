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
  virtual void acceptRawCode(const String& rawCode)
    {}

  virtual void acceptList(const std::vector<LuaASTNodePtr>& childNodes)
  {
    for (size_t i = 0; i < childNodes.size(); ++i)
      accept(childNodes[i]);
  }

  virtual void acceptBlock(const std::vector<LuaASTNodePtr>& childNodes)
  {
    for (size_t i = 0; i < childNodes.size(); ++i)
      accept(childNodes[i]);
  }


  // stat
  virtual void acceptSetStatement(const std::vector<LuaASTNodePtr>& lhs, const std::vector<LuaASTNodePtr>& expressions)
  {
    acceptList(lhs);
    acceptList(expressions);
  }

  virtual void acceptReturn(const std::vector<LuaASTNodePtr>& returnValues)
    {acceptList(returnValues);}

  // expr
  virtual void acceptNumber(double number)
    {}

  virtual void acceptFunction(const std::vector<LuaASTNodePtr>& signature, const std::vector<LuaASTNodePtr>& body)
  {
    acceptList(signature);
    acceptBlock(body);
  }

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
    jassert(node);
    const String& tag = node->getTag();
    if (tag == String::empty && node->getNumArguments() == 0)
      acceptList(node->getChildNodes());
    else if (tag == T("Raw"))
    {
      Variable v = node->getArgument(0);
      acceptRawCode(v.getString());
    }


    /*
    ** stat
    */
    else if (tag == T("Set"))
    {
      jassert(node->getNumChildNodes() == 2);
      acceptSetStatement(node->getChildNode(0)->getChildNodes(), node->getChildNode(1)->getChildNodes());
    }
    else if (tag == T("Return"))
      acceptReturn(node->getChildNodes());

    /*
    ** expr
    */
    else if (tag == T("Number"))
      acceptNumber(node->getArgument(0).getDouble());
    else if (tag == T("Function"))
    {
      jassert(node->getNumChildNodes() == 2);
      acceptFunction(node->getChildNode(0)->getChildNodes(), node->getChildNode(1)->getChildNodes());
    }
    else if (tag == T("Op"))
    {
      String opid = node->getArgument(0).getString();
      if (node->getNumChildNodes() == 1)
        acceptUnaryOperation(opid, node->getChildNode(0));
      else if (node->getNumChildNodes() == 2)
        acceptBinaryOperation(opid, node->getChildNode(0), node->getChildNode(1));
      else
        jassert(false);
    }
    else if (tag == T("Paren"))
      acceptParenthesis(node->getChildNode(0));

    /*
    ** Ident
    */
    else if (tag == T("Id"))
      acceptIdentifier(node->getArgument(0).getString());

    else
      jassert(false); // not implemented
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_VISITOR_H_

