/*-----------------------------------------.---------------------------------.
| Filename: GoProblem.h                    | Go Problem                      |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2011 17:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_

# include "../Core/DecisionProblem.h"
# include <lbcpp/Data/Matrix.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <list>

namespace lbcpp
{

extern EnumerationPtr playerEnumeration;

enum Player
{
  noPlayers = 0,
  blackPlayer,
  whitePlayer,
  bothPlayers
};

class GoBoard : public ShortEnumerationMatrix
{
public:
  GoBoard(size_t size)
    : ShortEnumerationMatrix(playerEnumeration, size, size, 0) {}
  GoBoard() {}

  typedef std::pair<size_t, size_t> Position;
  typedef std::set<Position> PositionSet;

  void set(const Position& position, Player player)
    {elements[makeIndex(position)] = (char)player;}

  void set(const PositionSet& positions, Player player)
  {
    for (PositionSet::const_iterator it = positions.begin(); it != positions.end(); ++it)
      set(*it, player);
  }

  Player get(const Position& position) const
    {return (Player)elements[makeIndex(position)];}

  void getAdjacentPositions(const Position& position, std::vector<Position>& res)
  {
    size_t x = position.first;
    size_t y = position.second;
    res.clear();
    res.reserve(4);
    if (x > 0)
      res.push_back(Position(x - 1, y));
    if (y > 0)
      res.push_back(Position(x, y - 1));
    if (x < numColumns - 1)
      res.push_back(Position(x + 1, y));
    if (y < numRows - 1)
      res.push_back(Position(x, y + 1));
  }

  void getGroup(const Position& position, PositionSet& res, PositionSet& liberties)
  {
    std::list<Position> toExplore;
    toExplore.push_back(position);
    Player player = get(position);

    while (toExplore.size())
    {
      Position explored = toExplore.front();
      toExplore.pop_front();
      res.insert(explored);

      std::vector<Position> adj;
      getAdjacentPositions(explored, adj);
      for (size_t i = 0; i < adj.size(); ++i)
      {
        Position candidate = adj[i];
        Player p = get(candidate);
        if (p == player && res.find(candidate) == res.end())
          toExplore.push_back(candidate);
        else if (p == noPlayers)
          liberties.insert(candidate);
      }
    }
  }

protected:
  size_t makeIndex(const Position& position) const
    {return ShortEnumerationMatrix::makeIndex(position.second, position.first);}
};

typedef ReferenceCountedObjectPtr<GoBoard> GoBoardPtr;

extern ClassPtr goStateClass;
class GoState;
typedef ReferenceCountedObjectPtr<GoState> GoStatePtr;

class GoState : public NameableObject
{
public:
  GoState(size_t time, GoBoardPtr board, size_t whitePrisonerCount, size_t blackPrisonerCount)
    : time(time), board(board), whitePrisonerCount(whitePrisonerCount), blackPrisonerCount(blackPrisonerCount) {}

  GoState(const String& name, size_t size)
    : NameableObject(name), time(0), board(new GoBoard(size)), whitePrisonerCount(0), blackPrisonerCount(0) {}

  GoState() : time(0), whitePrisonerCount(0), blackPrisonerCount(0) {}

  size_t getTime() const
    {return time;}

  const GoBoardPtr& getBoard() const
    {return board;}

  typedef GoBoard::Position Position;
  typedef std::set<Position> PositionSet;

  void addStone(Player player, size_t x, size_t y)
  {
    Position position(x, y);
    board->set(position, player);
    checkForCapture(position, player);
    checkForSuicide(position, player);

    previousPositions.push_front(position);
    if (previousPositions.size() > numPreviousPositions)
      previousPositions.pop_back();

    ++time;
  }

  Player getCurrentPlayer() const
    {return (time % 2) == 0 ? blackPlayer : whitePlayer;}

  void getStonesThatWouldBeCapturedIfPlaying(Player player, const Position& stonePosition, PositionSet& res)
  {
    std::vector<Position> adjacentPositions;
    board->getAdjacentPositions(stonePosition, adjacentPositions);
    Player opponent = (player == blackPlayer ? whitePlayer : blackPlayer);

    res.clear();
    for (size_t i = 0; i < adjacentPositions.size(); ++i)
    {
      Position position = adjacentPositions[i];
      if (board->get(position) == opponent)
      {
        PositionSet group;
        PositionSet liberties;
        board->getGroup(position, group, liberties);
        if (liberties.size() == 1 && *liberties.begin() == stonePosition)
          for (PositionSet::const_iterator it = group.begin(); it != group.end(); ++it)
            res.insert(*it);
      }
    }
  }

  void checkForCapture(const Position& position, Player player)
  {
    std::vector<Position> adjacentPositions;
    board->getAdjacentPositions(position, adjacentPositions);
    Player opponent = (player == blackPlayer ? whitePlayer : blackPlayer);

    capturedAtPreviousTurn.clear();
    for (size_t i = 0; i < adjacentPositions.size(); ++i)
      checkLiberties(adjacentPositions[i], opponent, &capturedAtPreviousTurn);
  }

  void checkForSuicide(const Position& position, Player player)
    {checkLiberties(position, player);}

  void checkLiberties(const Position& position, Player player, PositionSet* captured = NULL)
  {
    if (board->get(position) == player)
    {
      PositionSet group;
      PositionSet liberties;
      board->getGroup(position, group, liberties);
      if (liberties.empty())
      {
        board->set(group, noPlayers);
        if (player == whitePlayer)
          whitePrisonerCount += group.size();
        else
          blackPrisonerCount += group.size();
       
        if (captured)
          for (PositionSet::const_iterator it = group.begin(); it != group.end(); ++it)
            captured->insert(*it);
      }
    }
  }

  const Position& getLastPosition() const
    {jassert(previousPositions.size()); return previousPositions.front();}

  const std::list<Position>& getPreviousPositions() const
    {return previousPositions;}

  const PositionSet& getCapturedAtPreviousTurn() const
    {return capturedAtPreviousTurn;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t)
  {
    Object::clone(context, t);
    const GoStatePtr& target = t.staticCast<GoState>();
    target->board = board->cloneAndCast<GoBoard>(context);
    target->previousPositions = previousPositions;
    target->capturedAtPreviousTurn = capturedAtPreviousTurn;
  }

  virtual String toString() const
    {return getName() + T(" ") + String((int)time);}

  virtual String toShortString() const
    {return getName() + T(" ") + String((int)time);}

protected:
  friend class GoStateClass;

  enum 
  {
    numPreviousPositions = 10,
  };

  size_t time;
  GoBoardPtr board;
  size_t whitePrisonerCount;
  size_t blackPrisonerCount;
  std::list<Position> previousPositions;
  PositionSet capturedAtPreviousTurn;
};

class GoStateSampler : public SimpleUnaryFunction
{
public:
  GoStateSampler(size_t size = 19)
    : SimpleUnaryFunction(randomGeneratorClass, goStateClass), minSize(size), maxSize(size + 1) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    return new GoState(0, new GoBoard(random->sampleSize(minSize, maxSize)), 0, 0);
  }

protected:
  friend class GoStateSamplerClass;

  size_t minSize;
  size_t maxSize;
};


// state: GoState, action: Pair<PositiveInteger, PositiveInteger>
class GoTransitionFunction : public SimpleBinaryFunction
{
public:
  GoTransitionFunction()
    : SimpleBinaryFunction(goStateClass, pairClass(positiveIntegerType, positiveIntegerType), goStateClass) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& state = inputs[0].getObjectAndCast<GoState>();
    const PairPtr& action = inputs[1].getObjectAndCast<Pair>();
    size_t x = (size_t)action->getFirst().getInteger();
    size_t y = (size_t)action->getSecond().getInteger();

    GoStatePtr newState = state->cloneAndCast<GoState>();
    newState->addStone(state->getCurrentPlayer(), x, y);
    return newState;
  }
};

class GoRewardFunction : public SimpleBinaryFunction
{
public:
  GoRewardFunction()
    : SimpleBinaryFunction(goStateClass, pairClass(positiveIntegerType, positiveIntegerType), doubleType) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& state = inputs[0].getObjectAndCast<GoState>();
    const PairPtr& action = inputs[1].getObjectAndCast<Pair>();
    // FIXME
    return 0.0;
  }
};

class GoProblem : public DecisionProblem
{
public:
  GoProblem(size_t size = 19)
    : DecisionProblem(new GoStateSampler(size), new GoTransitionFunction(), new GoRewardFunction(), 1.0) {}

  virtual void getAvailableActions(const Variable& s, std::vector<Variable>& actions) const
  {
    const GoStatePtr& state = s.getObjectAndCast<GoState>();
    const GoBoardPtr& board = state->getBoard();
    actions.clear();

    bool testKo = (state->getCapturedAtPreviousTurn().size() == 1);

    for (size_t i = 0; i < board->getNumColumns(); ++i)
      for (size_t j = 0; j < board->getNumRows(); ++j)
      {
        GoBoard::Position position(i, j);
        if (board->get(position) == noPlayers)
        {
          if (testKo)
          {
            GoState::PositionSet captured;
            state->getStonesThatWouldBeCapturedIfPlaying(state->getCurrentPlayer(), position, captured);
            if (captured.find(state->getLastPosition()) != captured.end())
              continue; // KO
          }
          actions.push_back(new Pair(actionType, i, j));
        }
      }
  }
};

typedef ReferenceCountedObjectPtr<GoProblem> GoProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_
