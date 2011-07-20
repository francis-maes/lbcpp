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
  
   // general
  virtual void acceptRawCode(const String& rawCode)
    {write(rawCode);}

  virtual void acceptList(const std::vector<LuaASTNodePtr>& childNodes)
  {
    size_t n = childNodes.size();
    for (size_t i = 0; i < n; ++i)
    {
      accept(childNodes[i]);
      if (i < n - 1)
        write(", ");
    }
  }

  virtual void acceptBlock(const std::vector<LuaASTNodePtr>& childNodes)
  {
    for (size_t i = 0; i < childNodes.size(); ++i)
    {
      accept(childNodes[i]);
      endline();
    }
  }

  // stat
  virtual void acceptSetStatement(const std::vector<LuaASTNodePtr>& lhs, const std::vector<LuaASTNodePtr>& expressions)
  {
    acceptList(lhs);
    write(" = ");
    acceptList(expressions);
    endline();
  }

  virtual void acceptReturn(const std::vector<LuaASTNodePtr>& returnValues)
  {
    write("return ");
    acceptList(returnValues);
    endline();
  }

  // expr
  virtual void acceptNumber(double number)
    {write(String(number));}

  virtual void acceptFunction(const std::vector<LuaASTNodePtr>& signature, const std::vector<LuaASTNodePtr>& body)
  {
    write("function (");
    acceptList(signature);
    write(")");
    endline();

    ++indentation;
    acceptBlock(body);
    --indentation;

    write("end");
    endline();
  }

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

  void endline()
  {
    ostr << "\n";
    for (int i = 0; i < indentation; ++i)
      ostr << "  ";
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_PRETTY_PRINTER_H_
