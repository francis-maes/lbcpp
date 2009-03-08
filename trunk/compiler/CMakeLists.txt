INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(SYNOPSIS_ROOT_SOURCES
  synopsis/config.hh
  synopsis/Buffer.cc
  synopsis/Buffer.hh
  synopsis/Lexer.cc
  synopsis/Lexer.hh
  synopsis/PTree.hh
  synopsis/Parser.cc
  synopsis/Parser.hh
  synopsis/SymbolFactory.cc
  synopsis/SymbolFactory.hh
  synopsis/SymbolLookup.hh
  synopsis/Timer.hh
  synopsis/Token.hh
  synopsis/Trace.cc
  synopsis/Trace.hh
  synopsis/TypeAnalysis.hh
)

SET(SYNOPSIS_PTREE_SOURCES
  synopsis/PTree/Atoms.cc
  synopsis/PTree/Atoms.hh
  synopsis/PTree/Display.cc
  synopsis/PTree/Display.hh
  synopsis/PTree/Encoding.cc
  synopsis/PTree/Encoding.hh
  synopsis/PTree/GC.hh
  synopsis/PTree/Lists.cc
  synopsis/PTree/Lists.hh
  synopsis/PTree/Node.cc
  synopsis/PTree/Node.hh
  synopsis/PTree/NodesFwd.hh
  synopsis/PTree/operations.cc
  synopsis/PTree/operations.hh
  synopsis/PTree/TypeVisitor.hh
  synopsis/PTree/Visitor.cc
  synopsis/PTree/Visitor.hh
  synopsis/PTree/Writer.cc
  synopsis/PTree/Writer.hh
)

SET(SYNOPSIS_SYMBOL_LOOKUP_SOURCES
  synopsis/SymbolLookup/Display.cc
  synopsis/SymbolLookup/Display.hh
  synopsis/SymbolLookup/Scope.cc
  synopsis/SymbolLookup/Scope.hh
  synopsis/SymbolLookup/Scopes.cc
  synopsis/SymbolLookup/Scopes.hh
  synopsis/SymbolLookup/Symbol.cc
  synopsis/SymbolLookup/Symbol.hh
  synopsis/SymbolLookup/Walker.cc
  synopsis/SymbolLookup/Walker.hh
)

SET(SYNOPSIS_TYPE_ANALYSIS_SOURCES
  synopsis/TypeAnalysis/ConstEvaluator.cc
  synopsis/TypeAnalysis/ConstEvaluator.hh
  synopsis/TypeAnalysis/Kit.cc
  synopsis/TypeAnalysis/Kit.hh
  synopsis/TypeAnalysis/OverloadResolver.cc
  synopsis/TypeAnalysis/OverloadResolver.hh
  synopsis/TypeAnalysis/Type.cc
  synopsis/TypeAnalysis/Type.hh
  synopsis/TypeAnalysis/TypeEvaluator.cc
  synopsis/TypeAnalysis/TypeEvaluator.hh
  synopsis/TypeAnalysis/Visitor.hh
)

SET(CRALGO_COMPILER_TOOLS_SOURCES
  tools/PlatformUtilities.h
  tools/PlatformUtilities.cpp
  tools/RecursiveVisitor.h
  tools/RewriteVisitor.h
  tools/ScopeBasedRewriteVisitor.h
  tools/ReferenceToPointerRewriteVisitor.h
  tools/CodePrinter.h
  tools/PrettyPrintVisitor.h
  tools/PreprocessorAndCommentsChunker.h
  tools/ErrorManager.h
  tools/ErrorManager.cpp
  tools/PTreeAnalyser.h
  tools/PTreeAnalyser.cpp
  tools/PTreeGenerator.h
  tools/PTreeGenerator.cpp
)

SET(CRALGO_COMPILER_LANGUAGE_SOURCES
  language/CRAlgoPTree.h
  language/CRAlgoLexer.h
  language/CRAlgoParser.h
  language/CRAlgoParser.cpp

  language/CRAlgorithmVariableTranslator.h
  language/CRAlgorithmVariableTranslator.cpp
  language/CRAlgorithmLocalScope.h
  language/CRAlgorithmChoose.h
  language/CRAlgorithmChoose.cpp
)

SET(CRALGO_COMPILER_GENERATOR_SOURCES
  generator/FeatureGeneratorClassGenerator.h
  generator/FeatureGeneratorClassGenerator.cpp

  generator/ChooseClassGenerator.h
  generator/ChooseClassGenerator.cpp

  generator/CRAlgorithmScopeGenerator.h
  generator/CRAlgorithmScopeCopyConstructor.hpp
  generator/CRAlgorithmScopeDefaultConstructor.hpp
  generator/CRAlgorithmScopeDestructor.hpp
  generator/CRAlgorithmScopeIntrospectionFunction.hpp
  generator/CRAlgorithmScopeStepFunction.hpp
  generator/CRAlgorithmScopeRunFunction.hpp
  generator/CRAlgorithmStateFunctionClass.hpp
  generator/CRAlgorithmScopeEnumerator.hpp
  generator/CRAlgorithmScopeGenerator.cpp

  generator/CRAlgorithmGenerator.h
  generator/CRAlgorithmGenerator.cpp
)

SET(CRALGO_COMPILER_COMPILER_SOURCES
  compiler/CheckCRAlgoSyntaxVisitor.h
  compiler/CRAlgoCompilerRewriteVisitor.h
  compiler/CRInputFile.h
  compiler/CRInputFile.cpp
  compiler/CppOutputFile.h
  compiler/CppOutputFile.cpp
)

SET(CRALGO_COMPILER_SOURCES
  ${SYNOPSIS_ROOT_SOURCES}
  ${SYNOPSIS_PTREE_SOURCES}
  ${SYNOPSIS_SYMBOL_LOOKUP_SOURCES}
  ${SYNOPSIS_TYPE_ANALYSIS_SOURCES}
  common.h
  ${CRALGO_COMPILER_TOOLS_SOURCES}
  ${CRALGO_COMPILER_LANGUAGE_SOURCES}
  ${CRALGO_COMPILER_GENERATOR_SOURCES}
  ${CRALGO_COMPILER_COMPILER_SOURCES}
  CRAlgoCompiler.cpp
)

SOURCE_GROUP(Synopsis FILES ${SYNOPSIS_ROOT_SOURCES})
SOURCE_GROUP(Synopsis/PTree FILES ${SYNOPSIS_PTREE_SOURCES})
SOURCE_GROUP(Synopsis/SymbolLookup FILES ${SYNOPSIS_SYMBOL_LOOKUP_SOURCES})
SOURCE_GROUP(Synopsis/TypeAnalysis FILES ${SYNOPSIS_TYPE_ANALYSIS_SOURCES})
SOURCE_GROUP(Tools FILES ${CRALGO_COMPILER_TOOLS_SOURCES})
SOURCE_GROUP(Language FILES ${CRALGO_COMPILER_LANGUAGE_SOURCES})
SOURCE_GROUP(Generator FILES ${CRALGO_COMPILER_GENERATOR_SOURCES})
SOURCE_GROUP(Compiler FILES ${CRALGO_COMPILER_COMPILER_SOURCES})

ADD_EXECUTABLE(CRAlgoCompiler ${CRALGO_COMPILER_SOURCES})

TARGET_LINK_LIBRARIES(CRAlgoCompiler)
