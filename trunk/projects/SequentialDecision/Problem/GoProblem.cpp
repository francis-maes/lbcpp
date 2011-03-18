/*-----------------------------------------.---------------------------------.
| Filename: GoProblem.cpp                  | Go Problem                      |
| Author  : Francis Maes                   |                                 |
| Started : 15/03/2011 15:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "GoProblem.h"
using namespace lbcpp;

/*
** GoBoard
*/
GoBoard::GoBoard(size_t size)
  : ShortEnumerationMatrix(playerEnumeration, size, size, 0)
{
  thisClass = goBoardClass;
}

void GoBoard::getAdjacentPositions(const Position& position, Position res[4], size_t& numAdjacentPositions)
{
  size_t x = position.first;
  size_t y = position.second;
  numAdjacentPositions = 0;
  if (x > 0)
    res[numAdjacentPositions++] = Position(x - 1, y);
  if (y > 0)
    res[numAdjacentPositions++] = Position(x, y - 1);
  if (x < numColumns - 1)
    res[numAdjacentPositions++] = Position(x + 1, y);
  if (y < numRows - 1)
    res[numAdjacentPositions++] = Position(x, y + 1);
}

void GoBoard::getGroup(const Position& position, PositionSet& res, PositionSet& liberties)
{
  std::list<Position> toExplore;
  toExplore.push_back(position);
  Player player = get(position);

  Position adjacentPositions[4];
  size_t numAdjacentPositions;

  size_t size = getSize();
  std::vector<bool> exploredFlags(size * size, false);

  while (toExplore.size())
  {
    Position explored = toExplore.front();
    toExplore.pop_front();

    if (!exploredFlags[explored.first * size + explored.second])
    {
      exploredFlags[explored.first * size + explored.second] = true;
      res.insert(explored);
    
      getAdjacentPositions(explored, adjacentPositions, numAdjacentPositions);
      for (size_t i = 0; i < numAdjacentPositions; ++i)
      {
        Position candidate = adjacentPositions[i];
        Player p = get(candidate);
        if (p == player && !exploredFlags[candidate.first * size + candidate.second])
          toExplore.push_back(candidate);
        else if (p == noPlayers)
          liberties.insert(candidate);
      }
    }
  }
}

/*
** GoState
*/
GoState::GoState(const String& name, size_t size)
  : DecisionProblemState(name), time(0), board(new GoBoard(size)), whitePrisonerCount(0), blackPrisonerCount(0), previousActions(new GoActionVector())
{
  for (size_t i = 0; i < size; ++i)
    for (size_t j = 0; j < size; ++j)
      freePositions.insert(Position(i, j));
  availableActions = computeAvailableActions();
}

GoState::GoState() : time(0), whitePrisonerCount(0), blackPrisonerCount(0) {}

Player GoState::getCurrentPlayer() const
  {return (time % 2) == 0 ? blackPlayer : whitePlayer;}

GoBoardPtr GoState::getBoardWithCurrentPlayerAsBlack() const
{
  if (getCurrentPlayer() == blackPlayer)
    return board;
  else
  {
    size_t n = board->getSize();
    GoBoardPtr res(new GoBoard(n));
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
      {
        Position position(i, j);
        Player pl = board->get(position);
        if (pl != noPlayers)
          pl = (pl == blackPlayer ? whitePlayer : blackPlayer);
        res->set(position, pl);
      }
    return res;
  }
}

GoState::Position GoState::getLastPosition() const
  {jassert(previousActions && previousActions->getNumElements()); return previousActions->get(0);}

void GoState::addStone(Player player, size_t x, size_t y)
{
  Position position(x, y);
  board->set(position, player);
  freePositions.erase(position);

  checkForCapture(position, player);
  checkForSuicide(position, player);

  previousActions->prepend(position);
  if (previousActions->getNumElements() > numPreviousPositions)
    previousActions->remove(previousActions->getNumElements() - 1);

  availableActions = computeAvailableActions();
  ++time;
}

void GoState::getStonesThatWouldBeCapturedIfPlaying(Player player, const Position& stonePosition, PositionSet& res) const
{
  Position adjacentPositions[4];
  size_t numAdjacentPositions;
  board->getAdjacentPositions(stonePosition, adjacentPositions, numAdjacentPositions);
  Player opponent = (player == blackPlayer ? whitePlayer : blackPlayer);

  res.clear();
  for (size_t i = 0; i < numAdjacentPositions; ++i)
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

void GoState::checkForCapture(const Position& position, Player player)
{
  Position adjacentPositions[4];
  size_t numAdjacentPositions;
  board->getAdjacentPositions(position, adjacentPositions, numAdjacentPositions);
  Player opponent = (player == blackPlayer ? whitePlayer : blackPlayer);

  capturedAtPreviousTurn.clear();
  for (size_t i = 0; i < numAdjacentPositions; ++i)
    checkLiberties(adjacentPositions[i], opponent, &capturedAtPreviousTurn);
}

void GoState::checkForSuicide(const Position& position, Player player)
  {checkLiberties(position, player);}

void GoState::checkLiberties(const Position& position, Player player, PositionSet* captured)
{
  if (board->get(position) == player)
  {
    PositionSet group;
    PositionSet liberties;
    board->getGroup(position, group, liberties);
    if (liberties.empty())
    {
      board->set(group, noPlayers);
      addPositionsToPositionSet(freePositions, group);

      if (player == whitePlayer)
        whitePrisonerCount += group.size();
      else
        blackPrisonerCount += group.size();
     
      if (captured)
        addPositionsToPositionSet(*captured, group);
    }
  }
}

void GoState::addPositionsToPositionSet(PositionSet& res, const PositionSet& newPositions)
{
  std::set<Position> temp;
  std::set_union(res.begin(), res.end(), newPositions.begin(), newPositions.end(), std::inserter(temp, temp.begin()));
  res.swap(temp);
}

void GoState::removePositionsFromPositionSet(PositionSet& res, const PositionSet& positionsToRemove)
{
  std::set<Position> temp;
  std::set_intersection(res.begin(), res.end(), positionsToRemove.begin(), positionsToRemove.end(), std::inserter(temp, temp.begin()));
  res.swap(temp);
}

TypePtr GoState::getActionType() const
  {return goActionClass;}

ContainerPtr GoState::computeAvailableActions() const
{
  bool testKo = (getCapturedAtPreviousTurn().size() == 1);

  TypePtr actionType = getActionType();
  GoActionVectorPtr res = new GoActionVector();
  res->reserve(freePositions.size());
  for (PositionSet::const_iterator it = freePositions.begin(); it != freePositions.end(); ++it)
  {
    Position position = *it;
    if (board->get(position) == noPlayers)
    {
      if (testKo)
      {
        GoState::PositionSet captured;
        getStonesThatWouldBeCapturedIfPlaying(getCurrentPlayer(), position, captured);
        if (captured.size() == 1 && *captured.begin() == getLastPosition())
          continue; // KO
      }
      res->append(position);
    }
  }
  return res;
}

void GoState::performTransition(const Variable& a, double& reward)
{
  const GoActionPtr& action = a.getObjectAndCast<GoAction>();
  addStone(getCurrentPlayer(), action->getX(), action->getY());
  reward = 0.0; // FIXME: compute reward
}

void GoState::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  Object::clone(context, t);
  const GoStatePtr& target = t.staticCast<GoState>();
  target->board = board->cloneAndCast<GoBoard>(context);
  target->previousActions = previousActions->cloneAndCast<GoActionVector>();
  target->capturedAtPreviousTurn = capturedAtPreviousTurn;
  target->freePositions = freePositions;
  target->availableActions = availableActions->cloneAndCast<Container>(context);
}

String GoState::toString() const
  {return getName() + T(" ") + String((int)time);}

String GoState::toShortString() const
  {return getName() + T(" ") + String((int)time);}
