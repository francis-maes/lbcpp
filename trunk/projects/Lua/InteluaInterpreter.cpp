/*-----------------------------------------.---------------------------------.
| Filename: InteluaInterpreter.cpp         | Intelua File Interpreter        |
| Author  : Francis Maes                   |                                 |
| Started : 03/08/2011 19:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "InteluaInterpreter.h"
#include "DerivableRewriter.h"
#include "SubspecifiedRewriter.h"
#include "InteluaPreprocessRewriter.h"
#include "PrettyPrinterVisitor.h"
using namespace lbcpp;

bool InteluaInterpreter::checkInteluaDirectory(ExecutionContext& context, File& inteluaDirectory)
{
  if (!inteluaDirectory.getChildFile("AST.lua").existsAsFile() ||
      !inteluaDirectory.getChildFile("Language").isDirectory())
  {
    context.errorCallback("Intelua initialization", T("Incorrect intelua directory: ") + inteluaDirectory.getFullPathName());
    return false;
  }
  return true;
}

InteluaInterpreter::InteluaInterpreter(ExecutionContext& context, const File& inteluaDirectory, bool verbose)
  : translatorState(context, true, true), lua(context, true, true, verbose), verbose(verbose)
{
  String baseDirectory = inteluaDirectory.getFullPathName().replaceCharacter('\\', '/');
  {
    String initializeCode =
      "package.path = '" + baseDirectory + "/?.lua;' .. package.path\n"
      "require 'Language.LuaChunk'\n";
    translatorState.execute(initializeCode, "initializeCode");
  }

  {
    String initializeCode = 
      "package.path = '" + baseDirectory + "/?.lua;' .. package.path\n"
      "require 'InteluaCore'\n"
      "package.inteluaPath = " + inteluaDirectory.getFullPathName().quoted() + "\n";
    lua.execute(initializeCode, "initializeCode");

    lua.pushObject(ObjectPtr(this));
    lua.setGlobal("interpreter");
  }
}

InteluaInterpreter::~InteluaInterpreter()
{
  lua.clear();
}


bool InteluaInterpreter::executeBuffer(const char* code, const char* chunkName)
  {return loadBuffer(code, chunkName) && executeLoadedChunk(chunkName);}

bool InteluaInterpreter::executeFile(const File& file)
  {return loadFile(file) && executeLoadedChunk(file.getFileName());}

bool InteluaInterpreter::executeLoadedChunk(const char* chunkName)
{
  ExecutionContext& context = lua.getContext();
  if (verbose) context.enterScope(String("Lua interpreting ") + chunkName);
  bool ok = lua.call(0, 0);
  if (verbose) context.leaveScope(ok);
  return ok;
}

bool InteluaInterpreter::loadFile(const File& file)
{
  ExecutionContext& context = lua.getContext();

  if (verbose) context.enterScope(String("Parsing ") + file.getFileName());
  lua::BlockPtr block = parse(file, luaStatementBlock).staticCast<lua::Block>();
  if (verbose)
  {
    if (block)
      context.resultCallback("input ast", block);
    context.leaveScope(block != lua::BlockPtr());
  }

  if (block)
    return loadBlock(block, file.getFileName());
  else
  {
    context.errorCallback(file.getFileName(), "Parse error");
    return false;
  }
}

bool InteluaInterpreter::loadBuffer(const char* buffer, const char* chunkName)
{
  ExecutionContext& context = lua.getContext();

  if (verbose) context.enterScope(String("Parsing ") + chunkName);
  lua::BlockPtr block = parse(buffer, chunkName, luaStatementBlock).staticCast<lua::Block>();
  if (verbose)
  {
    if (block)
      context.resultCallback("input ast", block);
    context.leaveScope(block != lua::BlockPtr());
  }

  if (block)
    return loadBlock(block, chunkName);
  else
  {
    context.errorCallback(chunkName, "Parse error");
    return false;
  }
}

bool InteluaInterpreter::loadBlock(lua::BlockPtr block, const char* chunkName)
{
  ExecutionContext& context = lua.getContext();
  jassert(block);

  if (verbose) context.enterScope(String("Rewriting ") + chunkName);
  rewrite(block);
  if (verbose) context.leaveScope(block != lua::BlockPtr());
  if (!block)
  {
    context.errorCallback(chunkName, "Translation error");
    return false;
  }

  if (verbose) context.enterScope(String("Pretty printing ") + chunkName);
  String generatedCode = prettyPrint(block);
  if (verbose)
  {
    context.leaveScope(generatedCode.isNotEmpty());
    //context.informationCallback(T("Generated code"), generatedCode);
    std::cout << "Generated Code:" << std::endl << generatedCode << std::endl;
  }

  if (verbose) context.enterScope(String("Executing ") + chunkName);
  bool res = lua.loadBuffer(generatedCode, chunkName);
  if (verbose) context.leaveScope(res);
  return res;
}

int InteluaInterpreter::loadBuffer(LuaState& state)
{
  InteluaInterpreterPtr interpreter = state.checkObject(1, inteluaInterpreterClass).staticCast<InteluaInterpreter>();
  const char* buffer = state.checkString(2);
  const char* chunkName = state.getTop() >= 3 ? state.checkString(3) : buffer;
  bool ok = interpreter->loadBuffer(buffer, chunkName);
  if (ok)
    return 1;
  else
  {
    state.pushNil();
    state.pushString("could not load buffer");
    return 2;
  }
}

int InteluaInterpreter::loadFile(LuaState& state)
{
  InteluaInterpreterPtr interpreter = state.checkObject(1, inteluaInterpreterClass).staticCast<InteluaInterpreter>();
  const char* filename = state.checkString(2);
  bool ok = interpreter->loadFile(File::getCurrentWorkingDirectory().getChildFile(filename));
  if (ok)
    return 1;
  else
  {
    state.pushNil();
    state.pushString("could not load file");
    return 2;
  }
}

lua::NodePtr InteluaInterpreter::parse(const char* code, const char* name, LuaChunkType type)
{
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

lua::NodePtr InteluaInterpreter::parse(const File& file, LuaChunkType type)
{
  // call lua function LuaChunk.parseFromFile with (codeType, filepath)
  translatorState.getGlobal("LuaChunk", "parseFromFile");
  translatorState.pushInteger((int)type);
  translatorState.pushString(file.getFullPathName());
  if (!translatorState.call(2, 1))
    return lua::NodePtr();
  lua::NodePtr res = translatorState.checkObject(-1, lua::nodeClass).staticCast<lua::Node>();
  translatorState.pop();
  jassert(translatorState.getTop() == 0);    
  return res;
}

void InteluaInterpreter::rewrite(lua::BlockPtr& block)
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

String InteluaInterpreter::prettyPrint(const lua::NodePtr& tree) const
{
  juce::MemoryOutputStream ostr;
  lua::PrettyPrinterVisitor visitor(ostr);
  tree->accept(visitor);

  String code(ostr.getData());

  visitor.fillMissingLinesInLinesMap();
  const std::vector<size_t>& linesMap = visitor.getLinesMap();
  
  String linesMapStr = "__linesMaps[debug.getinfo(1).source] = {";
  for (size_t i = 0; i < linesMap.size(); ++i)
  {
    linesMapStr += String((int)linesMap[i]); // + 1 because we add the __linesMap line
    if (i < linesMap.size() - 1)
      linesMapStr +=  ","; 
  }

  linesMapStr += "}\n";
  return linesMapStr + code;
}
