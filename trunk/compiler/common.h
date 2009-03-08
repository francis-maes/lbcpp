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

# include <Synopsis/Trace.hh>
# include <Synopsis/Buffer.hh>
# include <Synopsis/Lexer.hh>
# include <Synopsis/SymbolFactory.hh>
# include <Synopsis/Parser.hh>
# include <Synopsis/PTree.hh>
# include <Synopsis/PTree/Writer.hh>
# include <Synopsis/PTree/Display.hh>
# include <Synopsis/SymbolLookup/Display.hh>
# include <Synopsis/TypeAnalysis/TypeEvaluator.hh>
# include <iostream>
# include <iomanip>
# include <fstream>

using namespace Synopsis;

#endif // !CRALGO_COMMON_H_
