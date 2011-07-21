/*-----------------------------------------.---------------------------------.
| Filename: LuaSandBox.h                   | Lua Sand Box                    |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 13:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_SANDBOX_H_
# define LBCPP_LUA_SANDBOX_H_

# include "Node.h"
# include "DerivativeVisitor.h"

extern "C" {
# include "../../src/Lua/lua/lua.h"
# include "../../src/Lua/lua/lauxlib.h"
# include "../../src/Lua/lua/lualib.h"
}; /* extern "C" */

namespace lbcpp
{

class LuaChunk : public NameableObject
{
public:
  LuaChunk(LuaState& lua, LuaChunkType type, const String& code)
    : lua(lua), type(type), code(code) {}
  LuaChunk(LuaState& lua, LuaChunkType type, const File& file)
    : lua(lua), type(type)
  {
    InputStream* istr = file.createInputStream();
    if (istr)
    {
      while (!istr->isExhausted())
        code += istr->readNextLine();
      delete istr;
    }
  }

  lua::NodePtr getTree() const
    {ensureTreeIsUpToDate(); return tree;}

  void setTree(const lua::NodePtr& tree)
    {code = String::empty; this->tree = tree;}

  const String& getCode() const
    {ensureCodeIsUpToDate(); return code;}

private:
  friend class LuaChunkClass;

  LuaState& lua;
  LuaChunkType type;
  String code;
  lua::NodePtr tree;

  lua::NodePtr parse(LuaState& lua, const String& code) const
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
    lua::NodePtr res = lua.checkObject(-1, lua::nodeClass).staticCast<lua::Node>();
    lua.pop();
    int dbg2 = lua.getTop();
    jassert(dbg1 == dbg2);
    return res;
  }

  String prettyPrint(LuaState& lua, const lua::NodePtr& tree) const
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


    /*static const char* inputCode = "2 * x * y";
      "function times(x, y) \n"
      "  return 2 * x * y, x\n"
      "end";*/

    //LuaChunk chunk(luaState, luaExpression, inputCode);
    LuaChunk chunk(luaState, luaExpression, luaFile);

    context.resultCallback(T("code-before"), chunk.getCode());
    context.resultCallback(T("tree-before"), chunk.getTree());

    chunk.setTree(lua::ExpressionDerivativeRewriter::computeWrtVariable(chunk.getTree(), new lua::Identifier("x")));

    //chunk.setTree(/*rewriteTree*/(chunk.getTree()));
    context.resultCallback(T("tree-after"), chunk.getTree());
    context.resultCallback(T("code-after"), chunk.getCode());

    std::cout << "-- generated code:" << std::endl;
    std::cout << chunk.getCode() << std::endl;

    // write to output file
    if (outputFile.exists())
      outputFile.deleteFile();
    OutputStream* ostr = outputFile.createOutputStream();
    if (ostr)
    {
      *ostr << chunk.getCode();
      delete ostr;
    }
    return true;
  }

#if 0

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
#endif // 0

protected:
  friend class LuaSandBoxClass;

  File luaFile;
  File outputFile;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_SANDBOX_H_

