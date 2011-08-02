/*-----------------------------------------.---------------------------------.
| Filename: LuaInterpreter.h               | Lua File Interpreter            |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 12:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_INTERPRETER_H_
# define LBCPP_LUA_INTERPRETER_H_

# include "Node.h"
# include "DerivableRewriter.h"
# include "Rewriter.h"
# include "Scope.h"

namespace lbcpp
{

class SubLuaInterpreter
{
public:
  SubLuaInterpreter(ExecutionContext& context)
    : lua(context)
  {
    static const char* initializeCode = 
      "package.path = 'C:/Projets/lbcpp/projects/Lua/lib/?.lua;' .. package.path\n"
      "require 'Language.LuaChunk'\n";
    lua.execute(initializeCode, "initializeCode");
  }
  
  bool interpretFile(const File& subLuaFile, LuaChunkType type = luaStatementBlock)
  {
    ExecutionContext& context = lua.getContext();

    // open file
    if (!subLuaFile.exists())
    {
      context.errorCallback(T("File ") + subLuaFile.getFullPathName() + T(" does not exists"));
      return false;
    }
    if (subLuaFile.isDirectory())
    {
      context.errorCallback(subLuaFile.getFullPathName() + T(" is a directory"));
      return false;
    }
    InputStream* istr = subLuaFile.createInputStream();
    if (!istr)
    {
      context.errorCallback(T("Could not open file ") + subLuaFile.getFullPathName());
      return false;
    }

    // read file
    String code;
    while (!istr->isExhausted())
      code += istr->readNextLine() + T("\n");
    delete istr;

    // interpret code
    String fileName = subLuaFile.getFileName();
    return interpretCode(code, fileName);
  }

  bool interpretCode(const char* code, const char* name)
  {
    ExecutionContext& context = lua.getContext();

    lua::BlockPtr block = parse(code, name, luaStatementBlock).staticCast<lua::Block>();
    if (!block)
    {
      context.errorCallback("Parse error", name);
      return false;
    }
    rewrite(block);
    if (!block)
    {
      context.errorCallback("Translation error", name);
      return false;
    }

    String generatedCode = prettyPrint(block);
    context.informationCallback(T("Generated code"), generatedCode);
    return lua.execute(generatedCode, name);
  }

protected:
  LuaState lua;

  lua::NodePtr parse(const char* code, const char* name, LuaChunkType type = luaStatementBlock)
  {
    // call lua function LuaChunk.parseFromString with (codeType, code, codeName)
    lua.getGlobal("LuaChunk", "parseFromString");
    lua.pushInteger((int)type);
    lua.pushString(code);
    lua.pushString(name);
    lua.call(3, 1);
    lua::NodePtr res = lua.checkObject(-1, lua::nodeClass).staticCast<lua::Node>();
    lua.pop();
    return res;
  }

  void rewrite(lua::BlockPtr& block)
  {
    // preprocessing
    block = lua::RemoveParenthesisRewriter().rewrite(block);
    block = lua::RemoveUnmLiteralRewriter().rewrite(block);
    block = lua::TransformInvokeIntoCallRewriter().rewrite(block);

    // scope analysis
    lua::ScopePtr scopes = lua::Scope::get(block);
    //context.resultCallback(T("scopes"), scopes);

    // derivable extension
    lua::DerivableRewriter::applyExtension(block);
  }

  String prettyPrint(const lua::NodePtr& tree) const
    {return tree->print();}

};

class ExecuteLuaString : public WorkUnit
{
public:
  String code;
 
  virtual String toShortString() const
  {
    int n = code.indexOfChar('\n');
    if (n < 0)
      return code;
    else
      return code.substring(0, n) + T("...");
  }

  virtual Variable run(ExecutionContext& context)
  {
    SubLuaInterpreter interpreter(context);
    return interpreter.interpretCode(code, "ExecuteLuaString");
  }
};

class ExecuteLuaFile : public WorkUnit
{
public:
  File file;

  virtual Variable run(ExecutionContext& context)
  {
    SubLuaInterpreter interpreter(context);
    return interpreter.interpretFile(file);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_INTERPRETER_H_
