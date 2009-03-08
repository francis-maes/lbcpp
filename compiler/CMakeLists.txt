INCLUDE_DIRECTORIES(${Synopsis_SOURCE_DIR})

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
  common.h
  ${CRALGO_COMPILER_TOOLS_SOURCES}
  ${CRALGO_COMPILER_LANGUAGE_SOURCES}
  ${CRALGO_COMPILER_GENERATOR_SOURCES}
  ${CRALGO_COMPILER_COMPILER_SOURCES}
  CRAlgoCompiler.cpp
)

SOURCE_GROUP(Tools FILES ${CRALGO_COMPILER_TOOLS_SOURCES})
SOURCE_GROUP(Language FILES ${CRALGO_COMPILER_LANGUAGE_SOURCES})
SOURCE_GROUP(Generator FILES ${CRALGO_COMPILER_GENERATOR_SOURCES})
SOURCE_GROUP(Compiler FILES ${CRALGO_COMPILER_COMPILER_SOURCES})

ADD_EXECUTABLE(CRAlgoCompiler ${CRALGO_COMPILER_SOURCES})

TARGET_LINK_LIBRARIES(CRAlgoCompiler Synopsis)
