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
    : ostr(ostr), indentation(indentation), isLineStart(true), lineNumber(1) {}

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
      size_t line = lineNumber;
      block.getSubNode(i)->accept(*this);
      if (line != lineNumber && indentation == 0)
        endLine();
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
  {
    write("while ");
    statement.getCondition()->accept(*this);
    write(" do");
    endLine();
    ++indentation;
    statement.getBlock()->accept(*this);
    --indentation;
    write("end");
  }

  virtual void visit(Repeat& statement)
  {
    write("repeat");
    endLine();
    ++indentation;
    statement.getBlock()->accept(*this);
    --indentation;
    write("until ");
    statement.getCondition()->accept(*this);
  }

  virtual void visit(If& statement)
  {
    for (size_t i = 0; i < statement.getNumConditions(); ++i)
    {
      write(i == 0 ? "if" : "elseif");
      write(" ");
      statement.getCondition(i)->accept(*this);
      write(" then");
      printBlockWithIndentation(statement.getBlock(i));
    }
    if (statement.getNumBlocks() > statement.getNumConditions())
    {
      write("else");
      printBlockWithIndentation(statement.getBlock(statement.getNumBlocks() - 1));
    }
    write("end");
  }

  virtual void visit(ForNum& statement)
  {
    write("for ");
    statement.getIdentifier()->accept(*this);
    write(" = ");
    statement.getFrom()->accept(*this);
    write(",");
    statement.getTo()->accept(*this);
    if (statement.getStep())
    {
      write(",");
      statement.getStep()->accept(*this);
    }
    write(" do");
    printBlockWithIndentation(statement.getBlock());
    write("end");
  }

  virtual void visit(ForIn& statement)
  {
    write("for ");
    statement.getIdentifiers()->accept(*this);
    write(" in ");
    statement.getExpressions()->accept(*this);
    write(" do");
    printBlockWithIndentation(statement.getBlock());
    write("end");
  }

  virtual void visit(Local& statement)
  {
    write("local ");
    statement.getIdentifiers()->accept(*this);
    if (statement.getExpressions())
    {
      write(" = ");
      statement.getExpressions()->accept(*this);
    }
  }

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

  virtual void visit(Break& statement)
    {write("break");}
   
  virtual void visit(ExpressionStatement& statement)
    {statement.getExpression()->accept(*this);}

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

  virtual void visit(Pair& pair)
  {
    writeIdentifierOrGenericAccess(pair.getSubNode(0));
    write(" = ");
    pair.getSubNode(1)->accept(*this);
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
      table.getSubNode(i)->accept(*this);
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
    if (operation.getOp() == notOp ||
        (operation.getOp() == unmOp && !isLiteralOrIdentifier) ||
        (operation.getOp() == lenOp && !isLiteralOrIdentifier))
      write(" ");
    operation.getExpr()->accept(*this);
  }


  bool areParenthesisRequired(BinaryOperation& operation, bool isLeftOperand)
  {
    NodePtr operand = isLeftOperand ? operation.getLeft() : operation.getRight();
    int pre = operation.getPrecendenceRank();
    OperationPtr operandOp = operand.dynamicCast<Operation>();
    return operandOp && (operandOp->getPrecendenceRank() < pre ||
      (operandOp->getPrecendenceRank() == pre && !isLeftOperand));
  }

  virtual void visit(BinaryOperation& operation)
  {
    static const char* luaOperators[] = {
      "+", "-", "*", "/",
      "%", "^", "..", "==",
      "<", "<=", "and", "or"
    };
    printWithParenthesis(operation.getLeft(), areParenthesisRequired(operation, true));
    write(" ");
    write(luaOperators[operation.getOp()]);
    write(" ");
    printWithParenthesis(operation.getRight(), areParenthesisRequired(operation, false));
  }

  virtual void visit(Parenthesis& parenthesis)
    {write("("); parenthesis.getExpr()->accept(*this); write(")");}

  virtual void visit(Call& call)
  {
    ExpressionPtr function = call.getFunction();
    printWithParenthesis(function, function.dynamicCast<Operation>());
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
 
  virtual void visit(Invoke& call)
  {
    call.getObject()->accept(*this);
    write(":");
    write(call.getFunction()->getValue());
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
  size_t lineNumber;

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
    ++lineNumber;
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
        write(value); // string becomes identifier
        return;
      }
    }

    // string access
    write("[");
    expr->accept(*this);
    write("]");
  }


  void printBlockWithIndentation(const BlockPtr& block)
  {
    endLine();
    ++indentation;
    block->accept(*this);
    --indentation;
  }

  void printWithParenthesis(const NodePtr& node, bool doParenthesis = true)
  {
    if (doParenthesis)
      write("(");
    node->accept(*this);
    if (doParenthesis)
      write(")");
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_VISITOR_PRETTY_PRINTER_H_
