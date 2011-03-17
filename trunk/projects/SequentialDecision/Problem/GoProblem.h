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
# include <lbcpp/UserInterface/MatrixComponent.h>

namespace lbcpp
{

extern EnumerationPtr playerEnumeration;

enum Player
{
  noPlayers = 0,
  blackPlayer,
  whitePlayer
};

class GoBoard : public ShortEnumerationMatrix
{
public:
  GoBoard(size_t size);
  GoBoard() {}

  typedef std::pair<size_t, size_t> Position;
  typedef std::set<Position> PositionSet;

  size_t getSize() const
    {jassert(getNumColumns() == getNumRows()); return getNumRows();}

  void set(const Position& position, Player player)
    {elements[makeIndex(position)] = (char)player;}

  void set(const PositionSet& positions, Player player)
  {
    for (PositionSet::const_iterator it = positions.begin(); it != positions.end(); ++it)
      set(*it, player);
  }

  Player get(const Position& position) const
    {return (Player)elements[makeIndex(position)];}

  void getAdjacentPositions(const Position& position, Position res[4], size_t& numAdjacentPositions);
  void getGroup(const Position& position, PositionSet& res, PositionSet& liberties);

protected:
  size_t makeIndex(const Position& position) const
    {return ShortEnumerationMatrix::makeIndex(position.second, position.first);}
};

typedef ReferenceCountedObjectPtr<GoBoard> GoBoardPtr;

extern ClassPtr goBoardClass;

extern ClassPtr goStateClass;
class GoState;
typedef ReferenceCountedObjectPtr<GoState> GoStatePtr;

class GoState : public DecisionProblemState
{
public:
  GoState(const String& name, size_t size);
  GoState();

  size_t getTime() const
    {return time;}

  const GoBoardPtr& getBoard() const
    {return board;}

  Player getCurrentPlayer() const;
  GoBoardPtr getBoardWithCurrentPlayerAsBlack() const;

  typedef GoBoard::Position Position;
  typedef std::set<Position> PositionSet;

  void addStone(Player player, size_t x, size_t y);

  void getStonesThatWouldBeCapturedIfPlaying(Player player, const Position& stonePosition, PositionSet& res) const;

  const Position& getLastPosition() const
    {jassert(previousPositions.size()); return previousPositions.front();}

  const std::list<Position>& getPreviousPositions() const
    {return previousPositions;}

  const PositionSet& getCapturedAtPreviousTurn() const
    {return capturedAtPreviousTurn;}

  /*
  ** DecisionProblemState
  */
  virtual TypePtr getActionType() const;
  virtual ContainerPtr getAvailableActions() const
    {return availableActions;}
  virtual void performTransition(const Variable& action, double& reward);

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;
  virtual String toString() const;
  virtual String toShortString() const;

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
  PositionSet freePositions;
  ContainerPtr availableActions;

  ContainerPtr computeAvailableActions() const;

  void checkForCapture(const Position& position, Player player);
  void checkForSuicide(const Position& position, Player player);
  void checkLiberties(const Position& position, Player player, PositionSet* captured = NULL);

  void addPositionsToPositionSet(PositionSet& res, const PositionSet& newPositions);
  void removePositionsFromPositionSet(PositionSet& res, const PositionSet& positionsToRemove);
};

class GoAction;
typedef ReferenceCountedObjectPtr<GoAction> GoActionPtr;

class GoAction : public Object
{
public:
  GoAction(size_t x, size_t y)
    : x((unsigned char)x), y((unsigned char)y) {}
  GoAction() : x((unsigned char)-1), y((unsigned char)-1) {}

  size_t getX() const
    {return (size_t)x;} 

  size_t getY() const
    {return (size_t)y;}
 
  virtual int compare(const ObjectPtr& otherObject) const
  {
    const GoActionPtr& other = otherObject.staticCast<GoAction>();
    if (x != other->x)
      return (int)x - (int)other->x;
    else
      return (int)y - (int)other->y;
  }
 
protected:
  friend class GoActionClass;

  unsigned char x, y;
};

extern ClassPtr goActionClass;

class GoStateSampler : public SimpleUnaryFunction
{
public:
  GoStateSampler(size_t size = 19)
    : SimpleUnaryFunction(randomGeneratorClass, goStateClass), minSize(size), maxSize(size + 1) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    return new GoState(T("Sampled"), random->sampleSize(minSize, maxSize));
  }

protected:
  friend class GoStateSamplerClass;

  size_t minSize;
  size_t maxSize;
};

class GoProblem : public DecisionProblem
{
public:
  GoProblem(size_t size = 19)
    : DecisionProblem(new GoStateSampler(size), 1.0) {}
};

typedef ReferenceCountedObjectPtr<GoProblem> GoProblemPtr;

class GoStateComponent : public MatrixComponent
{
public:
  GoStateComponent(GoStatePtr state, const String& name)
    : MatrixComponent(state->getBoard()), state(state) {}
 
  virtual juce::Colour selectColour(const Variable& element) const
  {
    if (!element.exists())
      return Colours::lightgrey;
    const juce::Colour colours[] = {juce::Colours::beige, juce::Colours::black, juce::Colours::white, juce::Colours::grey};
    return colours[element.getInteger() % (sizeof (colours) / sizeof (juce::Colour))];
  }

  virtual juce::Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name);

protected:
  GoStatePtr state;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_
