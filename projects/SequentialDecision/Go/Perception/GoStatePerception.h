/*-----------------------------------------.---------------------------------.
| Filename: GoStatePerception.h            | Go State Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2011 12:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GO_PERCEPTION_STATE_H_
# define LBCPP_GO_PERCEPTION_STATE_H_

# include "GoBoardPerception.h"
# include "GoRegionPerception.h"

namespace lbcpp
{

class GoStatePerception : public Object
{
public:
  GoStatePerception(const GoStatePtr& state)
  {
    // general state
    isCurrentPlayerBlack = (state->getCurrentPlayer() == blackPlayer);
    time = state->getTime();
    myPrisonerCount = isCurrentPlayerBlack ? state->getBlackPrisonerCount() : state->getWhitePrisonerCount();
    opponentPrisonerCount = isCurrentPlayerBlack ? state->getWhitePrisonerCount() : state->getBlackPrisonerCount();

    // board
    board = new GoBoardPerception(this, state->getBoardWithCurrentPlayerAsBlack());

    // previous actions
    PositiveIntegerPairVectorPtr previousActions = state->getPreviousActions();
    lastAction = GoBoardPositionPerceptionPtr();
    for (size_t i = 0; i < previousActions->size(); ++i)
    {
      GoBoardPositionPerceptionPtr positionPerception = board->getPosition(previousActions->get(i));
      positionPerception->setPrevious(lastAction);
      lastAction = positionPerception;
    }

    // captured at previous turn
    GoState::PositionSet captured = state->getCapturedAtPreviousTurn();
    for (GoState::PositionSet::const_iterator it = captured.begin(); it != captured.end(); ++it)
      board->getPosition(*it)->setCapturedAtPreviousTurn();


    // FIXME: create GoRegionPerception
    //fourConnexityGraph = segmentMatrixFunction(false)->compute(context, board).getObjectAndCast<SegmentedMatrix>(); 
    //eightConnexityGraph = segmentMatrixFunction(true)->compute(context, board).getObjectAndCast<SegmentedMatrix>();
  }
  GoStatePerception() {}

  virtual String toShortString() const
    {return "state";}

  const GoBoardPerceptionPtr& getBoard() const
    {return board;}

  const GoBoardPositionPerceptionPtr& getLastAction() const
    {return lastAction;}

protected:
  friend class GoStatePerceptionClass;

  bool isCurrentPlayerBlack;
  size_t time;
  size_t myPrisonerCount;
  size_t opponentPrisonerCount;

  GoBoardPerceptionPtr board;
  GoBoardPositionPerceptionPtr lastAction;

  //SegmentedMatrixPtr fourConnexityGraph;
  //SegmentedMatrixPtr eightConnexityGraph;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GO_PERCEPTION_STATE_H_
