/*-----------------------------------------.---------------------------------.
| Filename: Node.cpp                       | Lua AST                         |
| Author  : Francis Maes                   |                                 |
| Started : 21/07/2011 15:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Node.h"
#include "Visitor.h"
#include "PrettyPrinterVisitor.h"
using namespace lbcpp::lua;

String Node::print() const
  {return PrettyPrinterVisitor::print(*this);}

#define NODE_ACCEPT_FUNCTION(Class) \
  void Class ::accept(Visitor& visitor) \
    {return visitor.visit(*this);}

NODE_ACCEPT_FUNCTION(List)
NODE_ACCEPT_FUNCTION(Block)

// statements
NODE_ACCEPT_FUNCTION(Do)
NODE_ACCEPT_FUNCTION(Set)
NODE_ACCEPT_FUNCTION(While)
NODE_ACCEPT_FUNCTION(Return)
NODE_ACCEPT_FUNCTION(CallStatement)

// expressions
NODE_ACCEPT_FUNCTION(Nil)
NODE_ACCEPT_FUNCTION(Dots)
NODE_ACCEPT_FUNCTION(True)
NODE_ACCEPT_FUNCTION(False)
NODE_ACCEPT_FUNCTION(LiteralNumber)
NODE_ACCEPT_FUNCTION(LiteralString)
NODE_ACCEPT_FUNCTION(Function)
NODE_ACCEPT_FUNCTION(UnaryOperation)
NODE_ACCEPT_FUNCTION(BinaryOperation)
NODE_ACCEPT_FUNCTION(Parenthesis)
NODE_ACCEPT_FUNCTION(Call)
NODE_ACCEPT_FUNCTION(Identifier)
NODE_ACCEPT_FUNCTION(Index)
