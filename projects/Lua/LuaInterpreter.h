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
# include "Scope.h"

# include "DerivableRewriter.h"
# include "SubspecifiedRewriter.h"
# include "InteluaPreprocessRewriter.h"

namespace lbcpp
{

class InteluaInterpreter
{
public:
  InteluaInterpreter(ExecutionContext& context, bool verbose = false)
    : lua(context, true, true, verbose)
  {
    static const char* initializeCode = 
      "package.path = 'C:/Projets/lbcpp/projects/Lua/lib/?.lua;' .. package.path\n"
      "require 'InteluaCore'\n";
    lua.execute(initializeCode, "initializeCode");
  }

  ~InteluaInterpreter()
  {
    lua.clear();
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

  bool interpretCode(const char* code, const char* name, bool verbose = false)
  {
    ExecutionContext& context = lua.getContext();

    if (verbose) context.enterScope("Parsing");
    lua::BlockPtr block = parse(code, name, luaStatementBlock).staticCast<lua::Block>();
    if (verbose)
    {
      if (block)
        context.resultCallback("input ast", block);
      context.leaveScope(block != lua::BlockPtr());
    }
    if (!block)
    {
      context.errorCallback(name, "Parse error");
      return false;
    }

    if (verbose) context.enterScope("Rewriting");
    rewrite(block);
    if (verbose) context.leaveScope(block != lua::BlockPtr());
    if (!block)
    {
      context.errorCallback(name, "Translation error");
      return false;
    }

    if (verbose) context.enterScope("Pretty printing");
    String generatedCode = prettyPrint(block);
    if (verbose)
    {
      context.leaveScope(generatedCode.isNotEmpty());
      //context.informationCallback(T("Generated code"), generatedCode);
      std::cout << "Generated Code:" << std::endl << generatedCode << std::endl;
    }

    return lua.execute(generatedCode, name, verbose);
  }

protected:
  LuaState lua;

  lua::NodePtr parse(const char* code, const char* name, LuaChunkType type = luaStatementBlock)
  {
    LuaState translatorState(lua.getContext(), true, true); // needs lbcpp to create ast nodes

    static const char* initializeCode = 
      "package.path = 'C:/Projets/lbcpp/projects/Lua/lib/?.lua;' .. package.path\n"
      "require 'Language.LuaChunk'\n";
    translatorState.execute(initializeCode, "initializeCode");

    // call lua function LuaChunk.parseFromString with (codeType, code, codeName)
    translatorState.getGlobal("LuaChunk", "parseFromString");
    translatorState.pushInteger((int)type);
    translatorState.pushString(code);
    translatorState.pushString(name);
    if (!translatorState.call(3, 1))
      return lua::NodePtr();
    
    lua::NodePtr res = translatorState.checkObject(-1, lua::nodeClass).staticCast<lua::Node>();
    translatorState.pop();
    jassert(translatorState.getTop() == 0);    
    return res;
  }

  void rewrite(lua::BlockPtr& block)
  {
    ExecutionContext& context = lua.getContext();

    // preprocessing
    block = lua::InteluaPreprocessRewriter().rewrite(block);

    // scope analysis
    lua::ScopePtr scopes = lua::Scope::get(block);
    //context.resultCallback(T("scopes"), scopes);

    // derivable extension
    lua::SubspecifiedRewriter::applyExtension(context, block); // FIXME: are scopes still good ?
    lua::DerivableRewriter::applyExtension(context, block);
  }

  String prettyPrint(const lua::NodePtr& tree) const
    {return tree->print();}
};

class ExecuteLuaString : public WorkUnit
{
public:
  String code;
  String description;
 
  virtual String toShortString() const
  {
    if (description.isNotEmpty())
      return description;
    int n = code.indexOfChar('\n');
    if (n < 0)
      return code;
    else
      return code.substring(0, n) + T("...");
  }

  virtual Variable run(ExecutionContext& context)
  {
    static bool verbose = true;
    InteluaInterpreter interpreter(context, verbose);
    return interpreter.interpretCode(code, toShortString(), verbose);
  }
};

class ExecuteLuaFile : public WorkUnit
{
public:
  File file;

  virtual Variable run(ExecutionContext& context)
  {
    InteluaInterpreter interpreter(context);
    return interpreter.interpretFile(file);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_INTERPRETER_H_
