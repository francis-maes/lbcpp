/*-----------------------------------------.---------------------------------.
| Filename: common.h                       | Common Include File             |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_COMMON_H_
# define CRALGO_COMMON_H_

# ifdef WIN32
#  pragma warning(disable:4996)
# endif // WIN32

# include <synopsis/Trace.hh>
# include <synopsis/Buffer.hh>
# include <synopsis/Lexer.hh>
# include <synopsis/SymbolFactory.hh>
# include <synopsis/Parser.hh>
# include <synopsis/PTree.hh>
# include <synopsis/PTree/Writer.hh>
# include <synopsis/PTree/Display.hh>
# include <synopsis/SymbolLookup/Display.hh>
# include <synopsis/TypeAnalysis/TypeEvaluator.hh>
# include <iostream>
# include <iomanip>
# include <fstream>

using namespace Synopsis;

#endif // !CRALGO_COMMON_H_
