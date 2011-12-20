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
# include <lbcpp/Luape/LuapeFunction.h>

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
    bTrue = true, bFalse = false;
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

  GoBoardPositionPerception() : capturedAtPreviousTurn(false), bTrue(true), bFalse(false), position(0, 0) {}

  void setPrevious(GoBoardPositionPerceptionPtr previous)
    {this->previous = previous;}

  void setCapturedAtPreviousTurn(bool captured = true)
    {capturedAtPreviousTurn = captured;}

  virtual String toShortString() const
    {return "position";}

  const GoBoard::Position& getPosition() const
    {return position;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target)
  {
    Object::clone(context, target);
    target.staticCast<GoBoardPositionPerception>()->position = position;
  }

  virtual size_t getSizeInBytes() const
    {return sizeof (*this);}

protected:
  friend class GoBoardPositionPerceptionClass;

  GoBoardPerceptionPtr board;
  GoBoardPositionState positionState;
  GoBoardPositionPerceptionPtr previous;
  bool capturedAtPreviousTurn;

  bool bTrue, bFalse;

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
    return goOutside;
  }

private:
  GoBoard::Position position;
};

extern ClassPtr goBoardPositionPerceptionClass;

class GoBoardPositionRelationPerception : public Object
{
public:
  GoBoardPositionRelationPerception(const GoBoard::Position& a, const GoBoard::Position& b)
  {
    int delta1 = abs((int)a.first - (int)b.first);
    int delta2 = abs((int)a.second - (int)b.second);
    smallDelta = (size_t)juce::jmin(delta1, delta2);
    largeDelta = (size_t)juce::jmax(delta1, delta2);
  }
  GoBoardPositionRelationPerception() : smallDelta(0), largeDelta(0) {}

  size_t smallDelta;
  size_t largeDelta;

  virtual size_t getSizeInBytes() const
    {return sizeof (*this);}
};

extern ClassPtr goBoardPositionRelationPerceptionClass;

class GoBoardPositionRelationLuapeFunction : public LuapeFunction
{
public:
  virtual Flags getFlags() const
    {return commutativeFlag;}
    
  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(goBoardPositionPerceptionClass);}

  virtual TypePtr initialize(const std::vector<TypePtr>& inputTypes)
    {return goBoardPositionRelationPerceptionClass;}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const GoBoardPositionPerceptionPtr& first = inputs[0].getObjectAndCast<GoBoardPositionPerception>();
    const GoBoardPositionPerceptionPtr& second = inputs[1].getObjectAndCast<GoBoardPositionPerception>();
    if (!first || !second)
      return Variable::missingValue(goBoardPositionRelationPerceptionClass);
    return new GoBoardPositionRelationPerception(first->getPosition(), second->getPosition());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_GO_PERCEPTION_BOARD_POSITION_H_
