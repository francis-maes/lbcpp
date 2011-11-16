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
  GoBoardPositionPerception(GoBoardPerceptionPtr board, const GoBoard::Position& position, GoBoardPositionState positionState)
    : board(board), positionState(positionState), capturedAtPreviousTurn(false), position(position) {}
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
  GoBoardPositionState positionState;
  GoBoardPositionPerceptionPtr previous;
  bool capturedAtPreviousTurn;

private:
  GoBoard::Position position;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GO_PERCEPTION_BOARD_POSITION_H_
