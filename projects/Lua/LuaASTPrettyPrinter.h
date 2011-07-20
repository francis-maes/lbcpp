/*-----------------------------------------.---------------------------------.
| Filename: LuaASTPrettyPrinter.h          | Lua AST Pretty Printer          |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_AST_PRETTY_PRINTER_H_
# define LBCPP_LUA_AST_PRETTY_PRINTER_H_

# include "LuaASTVisitor.h"

namespace lbcpp
{

class LuaASTPrettyPrinter : public LuaASTVisitor
{
public:
  LuaASTPrettyPrinter(OutputStream& ostr, int indentation = 0)
    : ostr(ostr), indentation(indentation) {}
  
  static String toString(const LuaASTNodePtr& node)
  {
    juce::MemoryOutputStream ostr;
    LuaASTPrettyPrinter(ostr).accept(node);
    return String(ostr.getData());
  }
  
  // expr
  virtual void acceptUnaryOperation(const String& opid, const LuaASTNodePtr& childNode)
  {
    write(opid);
    accept(childNode);
  }

  virtual void acceptBinaryOperation(const String& opid, const LuaASTNodePtr& leftNode, const LuaASTNodePtr& rightNode)
  {
    const char* binaryOpIds[] = {
      "add", "sub", "mul", "div",
      "mod", "pow", "concat", "eq",
      "lt", "le", "and", "or"
    };

    const char* luaOperators[] = {
      "+", "-", "*", "/",
      "%", "^", "..", "==",
      "<", "<=", "and", "or"
    };

    accept(leftNode);

    for (size_t i = 0; i < sizeof (luaOperators) / sizeof (const char* ); ++i)
      if (opid == String(binaryOpIds[i]))
      {
        write(" ");
        write(luaOperators[i]);
        write(" ");
        break;
      }

    accept(rightNode);
  }

  virtual void acceptParenthesis(const LuaASTNodePtr& childNode)
  {
    write("(");
    accept(childNode);
    write(")");
  }

  // ident
  virtual void acceptIdentifier(const String& identifier)
    {write(identifier);}

private:
  OutputStream& ostr;
  int indentation;

  void write(const String& str)
    {ostr << str;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_PRETTY_PRINTER_H_
