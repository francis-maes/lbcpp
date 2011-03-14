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

  void set(const Position& position, Player player)
    {elements[makeIndex(position)] = (char)player;}

  void set(const std::set<Position>& positions, Player player)
  {
    for (std::set<Position>::const_iterator it = positions.begin(); it != positions.end(); ++it)
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

  void getGroup(const Position& position, std::set<Position>& res, size_t& numLiberties)
  {
    std::list<Position> toExplore;
    toExplore.push_back(position);
    Player player = get(position);
    numLiberties = 0;

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
          ++numLiberties;
      }
    }
  }

protected:
  size_t makeIndex(const Position& position) const
    {return ShortEnumerationMatrix::makeIndex(position.second, position.first);}
};

typedef ReferenceCountedObjectPtr<GoBoard> GoBoardPtr;

extern ClassPtr goStateClass;

class GoState : public Object
{
public:
  GoState(size_t time, GoBoardPtr board, size_t whitePrisonerCount, size_t blackPrisonerCount)
    : Object(goStateClass), time(time), board(board), whitePrisonerCount(whitePrisonerCount), blackPrisonerCount(blackPrisonerCount) {}
  GoState() : time(0) {}

  size_t getTime() const
    {return time;}

  const GoBoardPtr& getBoard() const
    {return board;}

  typedef GoBoard::Position Position;

  void addStone(size_t x, size_t y, Player player)
  {
    Position position(x, y);
    board->set(position, player);
    checkForCapture(position, player);
    checkForSuicide(position, player);
    ++time;
  }

  void checkForCapture(const Position& position, Player player)
  {
    std::vector<Position> adjacentPositions;
    board->getAdjacentPositions(position, adjacentPositions);
    Player opponent = (player == blackPlayer ? whitePlayer : blackPlayer);

    for (size_t i = 0; i < adjacentPositions.size(); ++i)
      checkLiberties(adjacentPositions[i], opponent);
  }

  void checkForSuicide(const Position& position, Player player)
    {checkLiberties(position, player);}

  void checkLiberties(const Position& position, Player player)
  {
    if (board->get(position) == player)
    {
      size_t numLiberties;
      std::set<Position> group;
      board->getGroup(position, group, numLiberties);
      if (!numLiberties)
      {
        board->set(group, noPlayers);
        if (player == whitePlayer)
          whitePrisonerCount += group.size();
        else
          blackPrisonerCount += group.size();
      }
    }
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target)
  {
    Object::clone(context, target);
    target.staticCast<GoState>()->board = board->cloneAndCast<GoBoard>(context);
  }

protected:
  friend class GoStateClass;

  size_t time;
  GoBoardPtr board;
  size_t whitePrisonerCount;
  size_t blackPrisonerCount;
};

typedef ReferenceCountedObjectPtr<GoState> GoStatePtr;

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

    size_t time = state->getTime();

    GoStatePtr newState = state->cloneAndCast<GoState>();
    newState->addStone(x, y, (time % 2) == 0 ? blackPlayer : whitePlayer);
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
};

typedef ReferenceCountedObjectPtr<GoProblem> GoProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_
