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
    : ostr(ostr), indentation(indentation), isLineStart(true), currentLineNumber(1) {}

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
      accept(list.getSubNode(i));
      if (i < n - 1)
        write(", ");
    }
  }

  virtual void visit(Block& block)
  {
    size_t n = block.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      size_t line = currentLineNumber;
      accept(block.getSubNode(i));
      if (currentLineNumber != line && indentation == 0)
        endLine();
      endLine();
    }
  }

  // statements
  virtual void visit(Do& statement)
    {jassert(false);} // not yet implemented

  virtual void visit(Set& statement)
  {
    accept(statement.getSubNode(0));
    write(" = ");
    accept(statement.getSubNode(1));
  }

  virtual void visit(While& statement)
  {
    write("while ");
    accept(statement.getSubNode(0)); // condition
    write(" do");
    endLine();
    ++indentation;
    accept(statement.getSubNode(1)); // block
    --indentation;
    write("end");
  }

  virtual void visit(Repeat& statement)
  {
    write("repeat");
    endLine();
    ++indentation;
    accept(statement.getSubNode(0)); // block
    --indentation;
    write("until ");
    accept(statement.getSubNode(1)); // condition
  }

  virtual void visit(If& statement)
  {
    for (size_t i = 0; i < statement.getNumConditions(); ++i)
    {
      write(i == 0 ? "if" : "elseif");
      write(" ");
      accept(statement.getSubNode(i * 2 + 1)); // condition i
      write(" then");
      printBlockWithIndentation(statement.getSubNode(i * 2)); // block i
    }
    if (statement.getNumBlocks() > statement.getNumConditions())
    {
      write("else");
      printBlockWithIndentation(statement.getSubNode(statement.getNumSubNodes() - 1)); // last block
    }
    write("end");
  }

  virtual void visit(ForNum& statement)
  {
    write("for ");
    accept(statement.getSubNode(0)); // identifier
    write(" = ");
    accept(statement.getSubNode(1)); // from
    write(",");
    accept(statement.getSubNode(2)); // to
    if (statement.getStep())
    {
      write(",");
      accept(statement.getSubNode(3)); // step
    }
    write(" do");
    printBlockWithIndentation(statement.getSubNode(statement.getNumSubNodes() - 1)); // block
    write("end");
  }

  virtual void visit(ForIn& statement)
  {
    write("for ");
    accept(statement.getSubNode(0)); // identifiers
    write(" in ");
    accept(statement.getSubNode(1)); // expressions
    write(" do");
    printBlockWithIndentation(statement.getSubNode(2)); // block
    write("end");
  }

  virtual void visit(Local& statement)
  {
    const ListPtr& expressions = statement.getExpressions();

    if (statement.isFunction() && expressions->getNumSubNodes() == 1 && expressions->getSubNode(0).isInstanceOf<Function>())
    {
      write("local function ");
      accept(statement.getSubNode(0)); // identifiers
      jassert(expressions->getNumSubNodes() == 1);
      FunctionPtr function = expressions->getSubNode(0).dynamicCast<Function>();
      jassert(function);

      write("(");
      accept(function->getSubNode(0)); // prototype
      write(")");
      endLine();
      ++indentation;
      accept(function->getSubNode(1)); // block
      --indentation;
      write("end");
      endLine();
    }
    else
    {
      write("local ");
      accept(statement.getSubNode(0)); // identifiers
      if (expressions && expressions->getNumSubNodes())
      {
        write(" = ");
        accept(statement.getSubNode(1)); // expressions
      }
    }
  }

  virtual void visit(Return& statement)
  {
    write("return ");
    size_t n = statement.getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      accept(statement.getSubNode(i));
      if (i < n - 1)
        write(", ");
    }
  }

  virtual void visit(Break& statement)
    {write("break");}
   
  virtual void visit(ExpressionStatement& statement)
    {accept(statement.getSubNode(0));}

  virtual void visit(Parameter& statement)
  {
    write("parameter ");
    accept(statement.getSubNode(0)); // identifier
    write(" = ");
    accept(statement.getSubNode(1)); // properties
  }

  // expressions
  virtual void visit(Nil& expression)
    {write("nil");}

  virtual void visit(Dots& expression)
    {write("...");}

  virtual void visit(LiteralBoolean& expression)
    {write(expression.getValue() ? "true" : "false");}

  virtual void visit(LiteralNumber& expression)
  {
    String str(expression.getValue());
    if (str == T("1.#INF") || str == T("inf"))
      write("math.huge");
    else if (str == T("-1.#INF"))
      write("-math.huge");
    else if (str == T("nan"))
      write("(0/0)");
    else
      write(str);
  }

  virtual void visit(LiteralString& expression)
  {
    write("\"");
    const String& str = expression.getValue();
    for (int i = 0; i < str.length(); ++i)
    {
      juce::tchar c = str[i];
      if (c == '\a')
        write("\\a");
      else if (c == '\b')
        write("\\b");
      else if (c == '\f')
        write("\\f");
      else if (c == '\n')
        write("\\n");
      else if (c == '\r')
        write("\\r");
      else if (c == '\t')
        write("\\t");
      else if (c == '\v')
        write("\\v");
      else if (c == '\\')
        write("\\\\");
      else if (c == '\'')
        write("'");
      else if (c == '"')
        write("\\\"");
      else
        write(c);
    }
    write("\"");
  }

  virtual void visit(Function& function)
  {
    write("function (");
    accept(function.getSubNode(0)); // prototype
    write(")");
    printBlockWithIndentation(function.getSubNode(1)); // block
    write("end");
    endLine();
  }

  virtual void visit(Pair& pair)
  {
    ExpressionPtr key = pair.getSubNode(0);
    String str;

    IdentifierPtr identifier = key.dynamicCast<Identifier>();
    if (identifier)
      accept(pair.getSubNode(0)); // identifier access
    else if (isLiteralStringConvertibleToIdentifier(key, str))
      write(str); // literal string becomes identifier
    else
    { 
      write("[");   // generic access
      accept(pair.getSubNode(0));
      write("]");
    }
    write(" = ");
    accept(pair.getSubNode(1));
  }

  virtual void visit(Table& table)
  {
    size_t n = table.getNumSubNodes();
    bool formatWithNewScope = false;
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr pn = table.getSubNode(i).dynamicCast<Pair>();
      if (pn && pn->getSubNode(1).isInstanceOf<Function>())
      {
        formatWithNewScope = true;
        break;
      }
    }

    write("{");
    if (formatWithNewScope)
    {
      endLine();
      ++indentation;
    }
    for (size_t i = 0; i < n; ++i)
    {
      accept(table.getSubNode(i));
      if (i < n - 1)
        write(", ");
    }
    if (formatWithNewScope)
      --indentation;
    write("}");
  }

  virtual void visit(UnaryOperation& operation)
  {
    static const char* luaOperators[] = {"not", "#", "-"};
    bool isLiteralOrIdentifier = operation.getExpr().dynamicCast<LiteralNumber>() || 
      operation.getExpr().dynamicCast<Identifier>();

    write(luaOperators[operation.getOp()]);
    if (isLiteralOrIdentifier)
      write(" ");
    else
      write("(");
     accept(operation.getSubNode(0));
     if (!isLiteralOrIdentifier)
       write(")");
  }

  virtual void visit(BinaryOperation& operation)
  {
    static const char* luaOperators[] = {
      "+", "-", "*", "/",
      "%", "^", "..", "==",
      "<", "<=", "and", "or"
    };
    printWithParenthesis(operation.getSubNode(0), areParenthesisRequired(operation, true));
    write(" ");
    write(luaOperators[operation.getOp()]);
    write(" ");
    printWithParenthesis(operation.getSubNode(1), areParenthesisRequired(operation, false));
  }

  virtual void visit(Parenthesis& parenthesis)
    {write("("); accept(parenthesis.getSubNode(0)); write(")");}

  virtual void visit(Call& call)
  {
    printWithParenthesis(call.getSubNode(0), call.getFunction().isInstanceOf<Operation>());
    write("(");
    size_t n = call.getNumArguments();
    for (size_t i = 0; i < n; ++i)
    {
      accept(call.getSubNode(i + 1)); // argument i
      if (i < n - 1)
        write(", ");
    }
    write(")");
  }
 
  virtual void visit(Invoke& call)
  {
    accept(call.getSubNode(0)); // object
    write(":");
    write(call.getFunction()->getValue());
    write("(");
    size_t n = call.getNumArguments();
    for (size_t i = 0; i < n; ++i)
    {
      accept(call.getSubNode(i + 2)); // argument i
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
    accept(index.getSubNode(0)); // left
    String str;
    if (isLiteralStringConvertibleToIdentifier(index.getRight(), str))
    {
      write(".");
      write(str); // string becomes identifier
    }
    else
    {
      // string access
      write("[");
      accept(index.getSubNode(1)); // right
      write("]");
    }
  }

  virtual void visit(Subspecified& subspecified)
    {write("subspecified "); accept(subspecified.getSubNode(0));}

  const std::vector<size_t>& getLinesMap() const
    {return linesMap;}

  void fillMissingLinesInLinesMap()
  {
    size_t line = 0;
    for (size_t i = 0; i < linesMap.size(); ++i)
    {
      ++line;
      if (linesMap[i] == 0)
        linesMap[i] = line;
      else
        line = linesMap[i];
    }
  }

private:
  OutputStream& ostr;
  int indentation;
  bool isLineStart;
  size_t currentLineNumber;

  std::vector<size_t> linesMap;

  virtual void accept(NodePtr& node)
  {
    if (node)
    {
      LineInfoPtr info = node->getFirstLineInfo();
      if (info)
      {
        if (linesMap.size() < currentLineNumber)
          linesMap.resize(currentLineNumber, 0);
        linesMap[currentLineNumber - 1] = info->getLine();
      }
      node->accept(*this);
    }
    else
      write("<null>");
  }

  void write(juce::tchar c)
  {
    String s;
    s += c;
    write(s);
  }

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
    ++currentLineNumber;
  }

  void printBlockWithIndentation(NodePtr& node)
  {
    endLine();
    ++indentation;
    accept(node);
    --indentation;
  }

  void printWithParenthesis(NodePtr& node, bool doParenthesis = true)
  {
    if (doParenthesis)
      write("(");
    accept(node);
    if (doParenthesis)
      write(")");
  }

  bool areParenthesisRequired(BinaryOperation& operation, bool isLeftOperand)
  {
    NodePtr operand = isLeftOperand ? operation.getLeft() : operation.getRight();
    int pre = operation.getPrecendenceRank();
    OperationPtr operandOp = operand.dynamicCast<Operation>();
    return operandOp && (operandOp->getPrecendenceRank() < pre ||
      (operandOp->getPrecendenceRank() == pre && !isLeftOperand));
  }

  bool isLiteralStringConvertibleToIdentifier(const ExpressionPtr& expr, String& value)
  {
    LiteralStringPtr str = expr.dynamicCast<LiteralString>();
    if (!str)
      return false;
    value = str->getValue();
    if (value.length() == 0 || value.containsAnyOf(T(" \t\r\n")))
      return false;
    if (!juce::CharacterFunctions::isLetterOrDigit(value[0]) && value[0] != '_')
      return false;

    static const char* keywords[] = {
      "and", "break", "do", "else", "elseif",
      "end", "false", "for", "function", "if",
      "in", "local", "nil", "not", "or", "repeat",
      "return", "then", "true", "until", "while"
    };
    static const size_t numKeywords = sizeof (keywords) / sizeof (const char* );

    const char* valueStr = (const char* )value;
    for (size_t i = 0; i < numKeywords; ++i)
      if (!strcmp(valueStr, keywords[i]))
        return false;

    return true;
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_PRETTY_PRINTER_H_
