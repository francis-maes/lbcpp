INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR})

SET(SYNOPSIS_PTREE_SOURCES
PTree/Atoms.cc
PTree/Atoms.hh
PTree/Display.cc
PTree/Display.hh
PTree/Encoding.cc
PTree/Encoding.hh
PTree/GC.hh
PTree/Lists.cc
PTree/Lists.hh
PTree/Node.cc
PTree/Node.hh
PTree/NodesFwd.hh
PTree/operations.cc
PTree/operations.hh
PTree/TypeVisitor.hh
PTree/Visitor.cc
PTree/Visitor.hh
PTree/Writer.cc
PTree/Writer.hh
)

SET(SYNOPSIS_SYMBOL_LOOKUP_SOURCES
SymbolLookup/Display.cc
SymbolLookup/Display.hh
SymbolLookup/Scope.cc
SymbolLookup/Scope.hh
SymbolLookup/Scopes.cc
SymbolLookup/Scopes.hh
SymbolLookup/Symbol.cc
SymbolLookup/Symbol.hh
SymbolLookup/Walker.cc
SymbolLookup/Walker.hh
)

SET(SYNOPSIS_TYPE_ANALYSIS_SOURCES
TypeAnalysis/ConstEvaluator.cc
TypeAnalysis/ConstEvaluator.hh
TypeAnalysis/Kit.cc
TypeAnalysis/Kit.hh
TypeAnalysis/OverloadResolver.cc
TypeAnalysis/OverloadResolver.hh
TypeAnalysis/Type.cc
TypeAnalysis/Type.hh
TypeAnalysis/TypeEvaluator.cc
TypeAnalysis/TypeEvaluator.hh
TypeAnalysis/Visitor.hh
)


SET(SYNOPSIS_ROOT_SOURCES
config.hh
Buffer.cc
Buffer.hh
Lexer.cc
Lexer.hh
PTree.hh
Parser.cc
Parser.hh
SymbolFactory.cc
SymbolFactory.hh
SymbolLookup.hh
Timer.hh
Token.hh
Trace.cc
Trace.hh
TypeAnalysis.hh
)

SET(SYNOPSIS_SOURCES
  ${SYNOPSIS_PTREE_SOURCES}
  ${SYNOPSIS_SYMBOL_LOOKUP_SOURCES}
  ${SYNOPSIS_TYPE_ANALYSIS_SOURCES}
  ${SYNOPSIS_ROOT_SOURCES}
)

SOURCE_GROUP(PTree FILES ${SYNOPSIS_PTREE_SOURCES})
SOURCE_GROUP(SymbolLookup FILES ${SYNOPSIS_SYMBOL_LOOKUP_SOURCES})
SOURCE_GROUP(TypeAnalysis FILES ${SYNOPSIS_TYPE_ANALYSIS_SOURCES})
SOURCE_GROUP(Synopsis FILES ${SYNOPSIS_ROOT_SOURCES})

ADD_LIBRARY(Synopsis ${SYNOPSIS_SOURCES})
