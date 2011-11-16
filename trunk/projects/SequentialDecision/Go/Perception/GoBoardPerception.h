/*-----------------------------------------.---------------------------------.
| Filename: GoBoardPerception.h            | Go Board Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2011 12:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GO_PERCEPTION_BOARD_H_
# define LBCPP_GO_PERCEPTION_BOARD_H_

# include "GoBoardPositionPerception.h"

namespace lbcpp
{

class GoBoardPerception : public Object
{
public:
  GoBoardPerception(GoStatePerceptionPtr state, GoBoardPtr board)
    : state(state), board(board)
  {
    boardSize = board->getSize();
    positions = new ObjectVector(goBoardPositionPerceptionClass, boardSize * boardSize);
    size_t index = 0;
    for (size_t i = 0; i < boardSize; ++i)
      for (size_t j = 0; j < boardSize; ++j)
      {
        GoBoard::Position position(i, j);
        positions->set(index++, new GoBoardPositionPerception(this, position, (GoBoardPositionState)board->get(position)));
      }
    passPosition = new GoBoardPositionPerception(this, GoBoard::Position(boardSize, boardSize), goOutside);
  }
  GoBoardPerception() : boardSize(0) {}

  GoBoardPositionPerceptionPtr getPosition(const GoBoard::Position& position) const
  {
    if (position.first == boardSize && position.second == boardSize)
      return passPosition;
    return positions->getAndCast<GoBoardPositionPerception>(position.first * boardSize + position.second);
  }

  GoBoardPositionPerceptionPtr getPassPosition() const
    {return passPosition;}

  virtual String toShortString() const
    {return "board";}

protected:
  friend class GoBoardPerceptionClass;

  GoStatePerceptionPtr state;
  size_t boardSize;
  ObjectVectorPtr positions;
  GoBoardPositionPerceptionPtr passPosition;

private:
  GoBoardPtr board; // with current player as black
};

}; /* namespace lbcpp */

#endif // !LBCPP_GO_PERCEPTION_BOARD_H_
