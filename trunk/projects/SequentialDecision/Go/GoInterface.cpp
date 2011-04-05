/*-----------------------------------------.---------------------------------.
| Filename: GoInterface.cpp                | Go Interface                    |
| Author  : Francis Maes                   |                                 |
| Started : 05/04/2011 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "GoInterface.h"
#include <lbcpp/lbcpp.h>

void lbcppgo::francisInit()
{
  std::cout << "Super initi !!!" << std::endl;
}

void lbcppgo::francisCompute(int board[361], double **patternValues, double values[361], int blackEatenStones, int whiteEatenStones, int color,int tour, int lastMove, int lastLastMove)
{
  std::cout << "Francis compute" << std::endl;
  
  for (size_t i = 0; i < 361; ++i)
    values[i] = (double)i;
  
  std::cout << "BlackES: " << blackEatenStones << " WhiteES: " << whiteEatenStones << " Color: " << color << " Tour: " << tour << " LastMove: " << lastMove << " LLastMove: " << lastLastMove << std::endl;
}
