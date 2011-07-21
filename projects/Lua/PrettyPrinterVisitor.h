/*-----------------------------------------.---------------------------------.
| Filename: PrettyPrinterVisitor.h         | Lua AST Pretty Printer          |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_VISITOR_PRETTY_PRINTER_H_
# define LBCPP_LUA_VISITOR_PRETTY_PRINTER_H_

# include "Visitor.h"

namespace lbcpp {
namespace lua {

class PrettyPrinterVisitor : public Visitor
{
public:
  PrettyPrinterVisitor(OutputStream& ostr, int indentation = 0)
    : ostr(ostr), indentation(indentation), isLineStart(true) {}

  static String print(const Node& node)
  {
    juce::MemoryOutputStream ostr;
    PrettyPrinterVisitor visitor(ostr);
    const_cast<Node& >(node).accept(visitor);
    return String(ostr.getData());
  }
  
  virtual void visit(List& list)
  {
    size_t n = list.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      list.getSubNode(i)->accept(*this);
      if (i < n - 1)
        write(", ");
    }
  }

  virtual void visit(Block& block)
  {
    size_t n = block.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      block.getSubNode(i)->accept(*this);
      endLine();
    }
  }

  // statements
  virtual void visit(Do& statement)
    {jassert(false);} // not yet implemented

  virtual void visit(Set& statement)
  {
    statement.getSubNode(0)->accept(*this);
    write(" = ");
    statement.getSubNode(1)->accept(*this);
  }

  virtual void visit(While& statement)
    {jassert(false);} // not yet implemented

  virtual void visit(Return& statement)
  {
    write("return ");
    size_t n = statement.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      statement.getSubNode(i)->accept(*this);
      if (i < n - 1)
        write(", ");
    }
  }
      
  virtual void visit(CallStatement& statement)
    {jassert(false);} // not yet implemented

  // expressions
  virtual void visit(Nil& expression)
    {write("nil");}

  virtual void visit(Dots& expression)
    {write("...");}

  virtual void visit(LiteralBoolean& expression)
    {write(expression.getValue() ? "true" : "false");}

  virtual void visit(LiteralNumber& expression)
    {write(String(expression.getValue()));}

  virtual void visit(LiteralString& expression)
    {write(expression.getValue().quoted());}

  virtual void visit(Function& function)
  {
    write("function (");
    function.getPrototype()->accept(*this);
    write(")");
    endLine();

    ++indentation;
    function.getBlock()->accept(*this);
    --indentation;

    write("end");
    endLine();
  }

  void writeIdentifierOrGenericAccess(const ExpressionPtr& expr, bool addDotBeforeIdentifier = false)
  {
    LiteralStringPtr str = expr.dynamicCast<LiteralString>();
    if (str)
    {
      String value = str->getValue();
      if (value.length() > 0 && !value.containsAnyOf(T(" \t\r\n")) && 
          (juce::CharacterFunctions::isLetterOrDigit(value[0]) || value[0] == '_'))
      {
        if (addDotBeforeIdentifier)
          write(".");
        write(value); // identifier
        return;
      }
    }

    // string access
    write("[");
    expr->accept(*this);
    write("]");
  }

  virtual void visit(Pair& pair)
  {
    writeIdentifierOrGenericAccess(pair.getSubNode(0));
    write(" = ");
    pair.getSubNode(1)->accept(*this);
  }

  virtual void visit(Table& table)
  {
    write("{");
    size_t n = table.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      table.getSubNode(i)->accept(*this);
      if (i < n - 1)
        write(", ");
    }
    write("}");
  }

  virtual void visit(UnaryOperation& operation)
  {
    static const char* luaOperators[] = {"not", "#", "-"};
    write(luaOperators[operation.getOp()]);
    if (operation.getOp() == notOp)
      write(" ");
    operation.getExpr()->accept(*this);
  }

  virtual void visit(BinaryOperation& operation)
  {
    static const char* luaOperators[] = {
      "+", "-", "*", "/",
      "%", "^", "..", "==",
      "<", "<=", "and", "or"
    };
    operation.getLeft()->accept(*this);
    write(" ");
    write(luaOperators[operation.getOp()]);
    write(" ");
    operation.getRight()->accept(*this);
  }

  virtual void visit(Parenthesis& parenthesis)
    {write("("); parenthesis.getExpr()->accept(*this); write(")");}

  virtual void visit(Call& call)
  {
    call.getFunction()->accept(*this);
    write("(");
    size_t n = call.getNumArguments();
    for (size_t i = 0; i < n; ++i)
    {
      call.getArgument(i)->accept(*this);
      if (i < n - 1)
        write(", ");
    }
    write(")");
  }

  virtual void visit(Identifier& identifier)
  {
    if (identifier.hasDerivableFlag())
      write("derivable ");
    write(identifier.getIdentifier());
  }

  virtual void visit(Index& index)
  {
    index.getLeft()->accept(*this);
    writeIdentifierOrGenericAccess(index.getRight(), true);
  }

private:
  OutputStream& ostr;
  int indentation;
  bool isLineStart;

  void write(const String& str)
  {
    if (isLineStart)
    {
      for (int i = 0; i < indentation; ++i)
        ostr << "  ";
      isLineStart = false;
    }
    ostr << str;
  }

  void endLine()
  {
    ostr << "\n";
    isLineStart = true;
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_PRETTY_PRINTER_H_
