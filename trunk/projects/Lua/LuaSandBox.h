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
    {return LuaASTPrettyPrinter::toString(tree);}

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


    LuaChunk chunk(luaState, luaExpression, T("a + (b * c)"));

    context.resultCallback(T("code-before"), chunk.getCode());
    context.resultCallback(T("tree-before"), chunk.getTree());

    chunk.setTree(rewriteTree(chunk.getTree()));

    context.resultCallback(T("tree-after"), chunk.getTree());
    context.resultCallback(T("code-after"), chunk.getCode());

    return true;
  }

  LuaASTNodePtr rewriteTree(const LuaASTNodePtr& tree) const
    {return tree;}

protected:
  friend class LuaSandBoxClass;

  File luaFile;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_SANDBOX_H_

