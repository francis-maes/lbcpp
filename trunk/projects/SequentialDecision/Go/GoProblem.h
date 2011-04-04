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

class GoState;
typedef ReferenceCountedObjectPtr<GoState> GoStatePtr;
class PositiveIntegerPair;
typedef ReferenceCountedObjectPtr<PositiveIntegerPair> PositiveIntegerPairPtr;
class PositiveIntegerPairVector;
typedef ReferenceCountedObjectPtr<PositiveIntegerPairVector> PositiveIntegerPairVectorPtr;

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

  lbcpp_UseDebuggingNewOperator

protected:
  size_t makeIndex(const Position& position) const
    {return ShortEnumerationMatrix::makeIndex(position.second, position.first);}
};

typedef ReferenceCountedObjectPtr<GoBoard> GoBoardPtr;

extern ClassPtr goBoardClass;
extern ClassPtr goStateClass;

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

  Position getLastPosition() const;

  const PositiveIntegerPairVectorPtr& getPreviousActions() const
    {return previousActions;}

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

  lbcpp_UseDebuggingNewOperator

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
  PositiveIntegerPairVectorPtr previousActions;
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

extern ClassPtr positiveIntegerPairClass;

namespace impl
{
  typedef std::pair<size_t, size_t> PositiveIntegerPair;
};

class PositiveIntegerPair : public Object
{
public:
  PositiveIntegerPair(const impl::PositiveIntegerPair& p)
    : Object(positiveIntegerPairClass), first(p.first), second(p.second)
  {
  }

  PositiveIntegerPair(size_t first, size_t second)
    : Object(positiveIntegerPairClass), first(first), second(second) {}
  PositiveIntegerPair() : Object(positiveIntegerPairClass), first((size_t)-1), second((size_t)-1) {}

  size_t getFirst() const
    {return first;} 

  size_t getSecond() const
    {return second;}
 
  virtual int compare(const ObjectPtr& otherObject) const
  {
    const PositiveIntegerPairPtr& other = otherObject.staticCast<PositiveIntegerPair>();
    if (first != other->first)
      return (int)first - (int)other->first;
    else
      return (int)second - (int)other->second;
  }

  impl::PositiveIntegerPair getValue() const
    {return impl::PositiveIntegerPair(first, second);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PositiveIntegerPairClass;

  size_t first;
  size_t second;
};

extern ClassPtr positiveIntegerPairVectorClass;

class PositiveIntegerPairVector : public BuiltinVector<impl::PositiveIntegerPair, PositiveIntegerPair>
{
public:
  typedef BuiltinVector<impl::PositiveIntegerPair, PositiveIntegerPair> BaseClass;

  PositiveIntegerPairVector(size_t length = 0)
    : BaseClass(positiveIntegerPairVectorClass, length, impl::PositiveIntegerPair(0, 0)) {}

  virtual TypePtr getElementsType() const
    {return positiveIntegerPairClass;}

  lbcpp_UseDebuggingNewOperator
};

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

  lbcpp_UseDebuggingNewOperator

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

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<GoProblem> GoProblemPtr;

class GoBoardComponent : public MatrixComponent
{
public:
  GoBoardComponent(GoBoardPtr board, const String& name)
    : MatrixComponent(board) {}
 
  virtual juce::Colour selectColour(const Variable& element)
  {
    if (!element.exists())
      return Colours::lightgrey;
    const juce::Colour colours[] = {juce::Colours::beige, juce::Colours::black, juce::Colours::white, juce::Colours::grey};
    return colours[element.getInteger() % (sizeof (colours) / sizeof (juce::Colour))];
  }

  lbcpp_UseDebuggingNewOperator
};

class GoStateComponent : public MatrixComponent
{
public:
  GoStateComponent(GoStatePtr state, const String& name);
 
  virtual juce::Colour selectColour(const Variable& element)
  {
    if (!element.exists())
      return Colours::lightgrey;
    const juce::Colour colours[] = {juce::Colours::beige, juce::Colours::black, juce::Colours::white, juce::Colours::grey};
    return colours[element.getInteger() % (sizeof (colours) / sizeof (juce::Colour))];
  }

  virtual void makeSelection(int x, int y)
  {
    bool isInside = (x >= 0 && y >= 0 && x < (int)matrix->getNumColumns() && y < (int)matrix->getNumRows());
    if (!isInside)
    {
      x = (int)matrix->getNumColumns();
      y = (int)matrix->getNumColumns();
      selectedRow = selectedColumn = -1;
    }
    else
      selectedRow = y, selectedColumn = x;

    if (isInside)
      sendSelectionChanged(Variable::pair(matrix, Variable::pair(selectedRow, selectedColumn)),
        T("Move(") + String((int)selectedRow) + T(", ") + String((int)selectedColumn) + T(")"));
    else
      sendSelectionChanged(Variable::pair(matrix, Variable::pair(x, y)), T("Pass"));
    repaint();
  }

  virtual juce::Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
  {
    const PairPtr& matrixAndPosition = variable.getObjectAndCast<Pair>();
    const PairPtr& position = matrixAndPosition->getSecond().getObjectAndCast<Pair>(); // (row, column)
    Variable positiveIntegerPair(new PositiveIntegerPair(position->getSecond().getInteger(), position->getFirst().getInteger())); // (x,y)

    ContainerPtr actionPerceptions = actionsPerceptionFunction->compute(context, state, availableActions).getObjectAndCast<Container>();
    Variable actionPerception;
    if (actionPerceptions)
    {
      ContainerPtr actions = state->getAvailableActions();
      jassert(actions->getNumElements() == actionPerceptions->getNumElements());
      for (size_t i = 0; i < actions->getNumElements(); ++i)
        if (actions->getElement(i) == positiveIntegerPair)
        {
          actionPerception = actionPerceptions->getElement(i);
          break;
        }
    }

    if (actionPerception.exists())
      return userInterfaceManager().createVariableTreeView(context, actionPerception, name + T(" perception"), false);
    else
      return NULL;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  GoStatePtr state;
  ContainerPtr availableActions;
  FunctionPtr actionsPerceptionFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_
