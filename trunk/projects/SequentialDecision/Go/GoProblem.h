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

  virtual TypePtr getElementsType() const
    {return playerEnumeration;}

  size_t getSize() const
    {jassert(getNumColumns() == getNumRows()); return getNumRows();}

  void set(size_t x, size_t y, Player player)
    {set(Position(x, y), player);}

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

  void getFreePositions(PositionSet& res);

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

  void setTime(size_t time)
    {this->time = time;}

  const GoBoardPtr& getBoard() const
    {return board;}

  size_t getBoardSize() const
    {return board->getSize();}

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
  ** Prisoners
  */
  size_t getWhitePrisonerCount() const
    {return whitePrisonerCount;}

  void setWhitePrisonerCount(int count)
    {whitePrisonerCount = count;}

  size_t getBlackPrisonerCount() const
    {return blackPrisonerCount;}

  void setBlackPrisonerCount(int count)
    {blackPrisonerCount = count;}
  /*
  ** DecisionProblemState
  */
  virtual TypePtr getActionType() const;

  void recomputeAvailableActions();
  virtual ContainerPtr getAvailableActions() const
    {return availableActions;}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL);

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;
  virtual String toString() const;
  virtual String toShortString() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

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

  virtual TypePtr getActionType() const
    {return positiveIntegerPairClass;}

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

class GoStateAndScoresComponent : public GoStateComponent
{
public:
  GoStateAndScoresComponent(PairPtr stateAndScores, const String& name);

  virtual bool doPaintShortString(const Variable& element, int width, int height)
    {return false;}

  virtual void paintEntry(juce::Graphics& g, size_t row, size_t column, int x1, int y1, int width, int height, const Variable& element)
  {
    Player player = state->getBoard()->get(GoBoard::Position(column, row));
    if (player == noPlayers)
    {
      Variable score = scores->getElement(row, column).getDouble();
      double normalizedScore = juce::jlimit(0.0, 1.0, score.getDouble() / 10.0 + 0.5);
      g.setColour(juce::Colour((float)normalizedScore, 1.f, 0.75f, (juce::uint8)255));
      g.fillRect(x1, y1, width, height);
      g.setColour(juce::Colours::black);
      if (width > 5)
      {
        g.setFont(juce::jlimit(5.f, 18.f, width / 2.f));
        g.drawText(score.toShortString(), x1, y1, width, height, juce::Justification::centred, false); 
      }
    }
    else
      GoStateComponent::paintEntry(g, row, column, x1, y1, width, height, Variable(player, playerEnumeration));
  }

  lbcpp_UseDebuggingNewOperator

protected:
  MatrixPtr scores;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_GO_H_
