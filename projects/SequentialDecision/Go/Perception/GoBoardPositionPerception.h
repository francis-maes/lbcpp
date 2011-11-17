/*-----------------------------------------.---------------------------------.
| Filename: GoBoardPositionPerception.h    | Go Board Position Perception    |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2011 12:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GO_PERCEPTION_BOARD_POSITION_H_
# define LBCPP_GO_PERCEPTION_BOARD_POSITION_H_

# include "../GoProblem.h"

namespace lbcpp
{

// predeclarations
class GoBoardPositionPerception;
typedef ReferenceCountedObjectPtr<GoBoardPositionPerception> GoBoardPositionPerceptionPtr;
class GoBoardPerception;
typedef ReferenceCountedObjectPtr<GoBoardPerception> GoBoardPerceptionPtr;
class GoRegionPerception;
typedef ReferenceCountedObjectPtr<GoRegionPerception> GoRegionPerceptionPtr;
class GoStatePerception;
typedef ReferenceCountedObjectPtr<GoStatePerception> GoStatePerceptionPtr;

extern ClassPtr goBoardPositionPerceptionClass;

enum GoBoardPositionState
{
  goEmpty = 0,
  goMyStone,
  goOpponentStone,
  goOutside
};

class GoBoardPositionPerception : public Object
{
public:
  GoBoardPositionPerception(GoBoardPerceptionPtr boardPerception, const GoBoardPtr& board, const GoBoard::Position& position)
    : board(boardPerception), capturedAtPreviousTurn(false), position(position)
  {
    if (position.first == board->getSize() && position.second == board->getSize())
      c11 = c12 = c13 = c21 = c22 = c23 = c31 = c32 = c33 = goOutside;
    else
    {
      c11 = getPositionState(board, position, -1, -1);
      c12 = getPositionState(board, position, -1, 0);
      c13 = getPositionState(board, position, -1, 1);
      c21 = getPositionState(board, position, 0, -1);
      c22 = getPositionState(board, position, 0, 0);
      c23 = getPositionState(board, position, 0, 1);
      c31 = getPositionState(board, position, 1, -1);
      c32 = getPositionState(board, position, 1, 0);
      c33 = getPositionState(board, position, 1, 1);
    }
  }

  GoBoardPositionPerception() : capturedAtPreviousTurn(false), position(0, 0) {}

  void setPrevious(GoBoardPositionPerceptionPtr previous)
    {this->previous = previous;}

  void setCapturedAtPreviousTurn(bool captured = true)
    {capturedAtPreviousTurn = captured;}

  virtual String toShortString() const
    {return "position";}

protected:
  friend class GoBoardPositionPerceptionClass;

  GoBoardPerceptionPtr board;
  //GoBoardPositionState positionState;
  GoBoardPositionPerceptionPtr previous;
  bool capturedAtPreviousTurn;

  GoBoardPositionState c11, c12, c13;
  GoBoardPositionState c21, c22, c23;
  GoBoardPositionState c31, c32, c33;

  static GoBoardPositionState getPositionState(const GoBoardPtr& board, const GoBoard::Position& position, int deltaRow, int deltaCol)
  {
    int s = (int)board->getSize();
    int row = (int)position.first + deltaRow;
    int col = (int)position.second + deltaCol;
    if (row >= 0 && row < s && col >= 0 && col < s)
      return (GoBoardPositionState)board->get(GoBoard::Position(row, col));
    else
      return goOutside;
  }

private:
  GoBoard::Position position;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GO_PERCEPTION_BOARD_POSITION_H_
