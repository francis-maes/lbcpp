/*-----------------------------------------.---------------------------------.
| Filename: LuaSandBox.h                   | Lua Sand Box                    |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 13:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_SANDBOX_H_
# define LBCPP_LUA_SANDBOX_H_

# include "LuaAST.h"
# include "LuaASTPrettyPrinter.h"

extern "C" {
# include "../../src/Lua/lua/lua.h"
# include "../../src/Lua/lua/lauxlib.h"
# include "../../src/Lua/lua/lualib.h"
}; /* extern "C" */

namespace lbcpp
{

enum LuaChunkType
{
  luaExpression = 0,
  luaStatement,
  luaStatementBlock,
};

class LuaChunk : public NameableObject
{
public:
  LuaChunk(LuaState& lua, LuaChunkType type, const String& code)
    : lua(lua), type(type), code(code) {}

  LuaASTNodePtr getTree() const
    {ensureTreeIsUpToDate(); return tree;}

  void setTree(const LuaASTNodePtr& tree)
    {code = String::empty; this->tree = tree;}

  const String& getCode() const
    {ensureCodeIsUpToDate(); return code;}

private:
  friend class LuaChunkClass;

  LuaState& lua;
  LuaChunkType type;
  String code;
  LuaASTNodePtr tree;

  LuaASTNodePtr parse(LuaState& lua, const String& code) const
  {
    int dbg1 = lua.getTop();
    // call lua function LuaChunk.parseFromString with (codeType, code, codeName)
    lua_getfield(lua, LUA_GLOBALSINDEX, "LuaChunk");
    lua_getfield(lua, -1, "parseFromString");
    lua_remove(lua, -2); // remove "LuaChunk" from stack
    lua.pushInteger((int)type);
    lua.pushString(code);
    lua.pushString(name);
    lua_call(lua, 3, 1);
    LuaASTNodePtr res = lua.checkObject(-1, luaASTNodeClass).staticCast<LuaASTNode>();
    lua.pop();
    int dbg2 = lua.getTop();
    jassert(dbg1 == dbg2);
    return res;
  }

  String prettyPrint(LuaState& lua, const LuaASTNodePtr& tree) const
    {return tree->print();}

  void ensureTreeIsUpToDate() const
  {
    if (!tree)
      const_cast<LuaChunk* >(this)->tree = parse(lua, code);
  }

  void ensureCodeIsUpToDate() const
  {
    if (tree && code.isEmpty())
      const_cast<LuaChunk* >(this)->code = prettyPrint(lua, tree);
  }
};

// todo: move this
String LuaASTNode::print() const
  {return LuaASTPrettyPrinter::toString(refCountedPointerFromThis(this));}
// -

/*

function times(x, y)
  return 2 * x * y
end

==>

times = {}

function times.f(x, y)
  return 2 * x * y
end
function times.dx(x, y)
  return 2 * y
end
function times.dy(x, y)
  return 2 * x
end

*/

class LuaSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    LuaState luaState(context);

    static const char* initializeCode = 
      "package.path = 'C:/Projets/lbcpp/projects/Lua/lib/?.lua;' .. package.path\n"
      "require 'LuaChunk'\n";
    luaState.execute(initializeCode, "initializeCode");


    static const char* inputCode = 
      "function times(x, y) \n"
      "  return 2 * x * y\n"
      "end";

    LuaChunk chunk(luaState, luaStatement, inputCode);

    context.resultCallback(T("code-before"), chunk.getCode());
    context.resultCallback(T("tree-before"), chunk.getTree());

    chunk.setTree(rewriteTree(chunk.getTree()));

    context.resultCallback(T("tree-after"), chunk.getTree());
    context.resultCallback(T("code-after"), chunk.getCode());

    return true;
  }


  static LuaASTNodePtr multiply(const LuaASTNodePtr& left, const LuaASTNodePtr& right)
  {
    bool isLeftNumber = (left->getTag() == T("Number"));
    bool isRightNumber = (right->getTag() == T("Number"));

    if (isLeftNumber && isRightNumber)
      return LuaASTNode::newNumber(left->getArgument(0).getDouble() * right->getArgument(0).getDouble());

    if ((isLeftNumber && !isRightNumber) || (isRightNumber && !isLeftNumber))
    {
      double number = (isLeftNumber ? left : right)->getArgument(0).getDouble();
      LuaASTNodePtr expr = (isLeftNumber ? right : left);
      if (number == 0.0)
        return LuaASTNode::newNumber(0.0);
      else if (number == 1.0)
        return expr;
      else if (number == -1.0)
        return LuaASTNode::newOp("unm", expr);
    }

    return LuaASTNode::newOp("mul", left, right);
  }

  static LuaASTNodePtr add(const LuaASTNodePtr& left, const LuaASTNodePtr& right)
  {
    bool isLeftNumber = (left->getTag() == T("Number"));
    bool isRightNumber = (right->getTag() == T("Number"));

    if (isLeftNumber && isRightNumber)
      return LuaASTNode::newNumber(left->getArgument(0).getDouble() + right->getArgument(0).getDouble());

    // x + 0 = x, 0 + x = x
    if ((isLeftNumber && !isRightNumber) || (isRightNumber && !isLeftNumber))
    {
      double number = (isLeftNumber ? left : right)->getArgument(0).getDouble();
      LuaASTNodePtr expr = (isLeftNumber ? right : left);
      if (number == 0.0)
        return expr;
    }

    return LuaASTNode::newParen(LuaASTNode::newOp("add", left, right));
  }

  LuaASTNodePtr computeExpressionDerivative(const LuaASTNodePtr& expression, const LuaASTNodePtr& variable) const
  {
    if (expression->getTag() == T("Id"))
    {
      if (expression->getArgument(0).getString() == variable->getArgument(0).getString())
        return LuaASTNode::newNumber(1.0); // d(x)/dx = 1
      else
        return LuaASTNode::newNumber(0.0); // d(y)/dx = 0
    }

    if (expression->getTag() == T("Op"))
    {
      String opid = expression->getArgument(0).getString();

      if (expression->getNumChildNodes() == 2)
      {
        LuaASTNodePtr u = expression->getChildNode(0);
        LuaASTNodePtr v = expression->getChildNode(1);
        LuaASTNodePtr uprime = computeExpressionDerivative(u, variable);
        LuaASTNodePtr vprime = computeExpressionDerivative(v, variable);

        // (uv)' = (u'v + uv')
        if (opid == T("mul"))
          return add(multiply(uprime, v), multiply(u, vprime));

        // (u + v)' = (u' + v')
        else if (opid == T("add"))
          return add(uprime, vprime);


      }
    }

    if (expression->getTag() == T("Number"))
    {
      // d(cst)/dx = 0
      return LuaASTNode::newNumber(0.0);
    }

    return expression;
  }

  LuaASTNodePtr computeStatementDerivative(const LuaASTNodePtr& statement, const LuaASTNodePtr& variable) const
  {
    if (statement->getTag() == T("Return"))
    {
      std::vector<LuaASTNodePtr> childNodes(statement->getNumChildNodes());
      for (size_t i = 0; i < childNodes.size(); ++i)
        childNodes[i] = computeExpressionDerivative(statement->getChildNode(i), variable);
      return new LuaASTNode(T("Return"), childNodes);
    }
    return statement;
  }

  LuaASTNodePtr computeBlockDerivative(const LuaASTNodePtr& block, const LuaASTNodePtr& variable) const
  {
    std::vector<LuaASTNodePtr> res;
    for (size_t i = 0; i < block->getNumChildNodes(); ++i)
      res.push_back(computeStatementDerivative(block->getChildNode(i), variable));
    return new LuaASTNode(res);
  }


  // 'Set Id = Function' statement => statement block 
  LuaASTNodePtr rewriteTree(const LuaASTNodePtr& tree) const
  {
    jassert(tree->getTag() == T("Set"));
    LuaASTNodePtr identifier = tree->getChildNode(0)->getChildNode(0);
    LuaASTNodePtr function = tree->getChildNode(1)->getChildNode(0);
    jassert(identifier->getTag() == T("Id") && function->getTag() == T("Function"));
    LuaASTNodePtr arguments = function->getChildNode(0);
    LuaASTNodePtr body = function->getChildNode(1);

    std::vector<LuaASTNodePtr> block;
    String id = identifier->print();
    block.push_back(new LuaASTNode(T("set ") + id + T(" = {}\n")));
    block.push_back(new LuaASTNode(T("function ") + id + T(".f(") + arguments->print() + T(") ") + body->print() + T("end")));

    for (size_t i = 0; i < arguments->getNumChildNodes(); ++i)
    {
      LuaASTNodePtr argumentIdentifier = arguments->getChildNode(i);
      String arg = argumentIdentifier->print();
      LuaASTNodePtr derivativeBody = computeBlockDerivative(body, argumentIdentifier);
      block.push_back(new LuaASTNode(T("function ") + id + T(".d") + arg + T("(") + arguments->print() + T(") ") + derivativeBody->print() + T("end")));
    }
    return new LuaASTNode(block);
  }

protected:
  friend class LuaSandBoxClass;

  File luaFile;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_SANDBOX_H_

