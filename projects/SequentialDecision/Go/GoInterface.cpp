/*-----------------------------------------.---------------------------------.
| Filename: GoInterface.cpp                | Go Interface                    |
| Author  : Francis Maes                   |                                 |
| Started : 05/04/2011 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "GoInterface.h"
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

static CompositeFunctionPtr goDecisionMaker;

void lbcppgo::francisInit()
{
  std::cout << "Initializing lbcpp..." << std::endl;
  lbcpp::initialize("toto");
  std::cout << "Loading model..." << std::endl;
  goDecisionMaker = CompositeFunction::createFromFile(defaultExecutionContext(), File::getCurrentWorkingDirectory().getChildFile(T("lbcppgo.model")));
  std::cout << "ok for now" << std::endl;
}

void lbcppgo::francisCompute(int board[361], double **patternValues, double values[361], int blackEatenStones, int whiteEatenStones, int color,int tour, int lastMove, int lastLastMove)
{
  std::cout << "Francis compute" << std::endl;
  
  for (size_t i = 0; i < 361; ++i)
    values[i] = (double)i;
  
  std::cout << "BlackES: " << blackEatenStones << " WhiteES: " << whiteEatenStones << " Color: " << color << " Tour: " << tour << " LastMove: " << lastMove << " LLastMove: " << lastLastMove << std::endl;
}
