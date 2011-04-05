/*-----------------------------------------.---------------------------------.
| Filename: GoInterface.h                  | Go Interface                    |
| Author  : Francis Maes                   |                                 |
| Started : 05/04/2011 16:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_HEADER_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_HEADER_H_

namespace lbcppgo
{
  extern void francisInit();

  extern void francisCompute(int board[361], double **patternValues, double values[361], int blackEatenStones, int whiteEatenStones, int color,int tour, int lastMove, int LastLastMove);
};

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_HEADER_H_
