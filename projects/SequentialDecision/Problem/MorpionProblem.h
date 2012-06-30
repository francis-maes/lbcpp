/*-----------------------------------------.---------------------------------.
| Filename: MorpionProblem.h               | Solitaire Morpion               |
| Author  : David Lupien St-Pierre         |                                 |
| Started : 14/06/2012 21:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPION_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPION_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include "MorpionBoard.h"

namespace lbcpp
{

/*
** Action
*/
extern ClassPtr morpionActionClass;
class MorpionAction;
typedef ReferenceCountedObjectPtr<MorpionAction> MorpionActionPtr;

class MorpionAction : public Object
{
public:
  MorpionAction(const MorpionPoint& position, const MorpionDirection& direction, size_t indexInLine)
    : Object(morpionActionClass), position(position), direction(direction), indexInLine(indexInLine) {}
  MorpionAction() : indexInLine(-1) {}

  const MorpionPoint& getPosition() const
    {return position;}

  const MorpionDirection& getDirection() const
    {return direction;}

  int getIndexInLine() const
    {return indexInLine;}

  MorpionPoint getStartPosition() const
    {return position.moveIntoDirection(direction, -(int)indexInLine);}

  MorpionPoint getEndPosition(size_t crossLength) const
    {return position.moveIntoDirection(direction, (int)(crossLength - 1 - indexInLine));}

  MorpionPoint getLinePoint(size_t index) const
    {return position.moveIntoDirection(direction, (int)index - (int)indexInLine);}

  virtual String toShortString() const
    {return position.toString() + ", " + direction.toString() + ", " + String((int)indexInLine);}

  bool operator ==(const MorpionAction& other) const
    {return position == other.position && direction == other.direction && indexInLine == other.indexInLine;}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    MorpionActionPtr other = otherObject.dynamicCast<MorpionAction>();
    if (!other)
      return Object::compare(otherObject);
    if (position.getX() != other->position.getX())
      return position.getX() - other->position.getX();
    if (position.getY() != other->position.getY())
      return position.getY() - other->position.getY();
    if (direction != other->direction)
      return (int)(MorpionDirection::Direction)direction - (int)(MorpionDirection::Direction)other->direction;
    return (int)indexInLine - (int)other->indexInLine;
  }

private:
  MorpionPoint position;
  MorpionDirection direction;
  size_t indexInLine; // in range [0, crossLength[
};

/*
** State
*/
class MorpionState;
typedef ReferenceCountedObjectPtr<MorpionState> MorpionStatePtr;

class MorpionState : public DecisionProblemState
{
public:
	MorpionState(size_t crossLength, bool isDisjoint)
    : DecisionProblemState(String((int)crossLength) + (isDisjoint ? "D" : "T")),
      crossLength(crossLength), isDisjoint(isDisjoint)
  {
    board.initialize(crossLength);
    availableActions = computeAvailableActions();    
  }
  MorpionState() : crossLength(0), isDisjoint(false) {}

	virtual TypePtr getActionType() const
	  {return morpionActionClass;}
  
	virtual ContainerPtr getAvailableActions() const
    {return availableActions;}

  virtual ObjectVectorPtr computeActionFeatures(ExecutionContext& context, const ContainerPtr& actions) const
  {
    enum {featuresComplexity = 4};

    MorpionFeatures features;

    SparseDoubleVectorPtr stateFeatures = features.compute(board, featuresComplexity);

    size_t n = actions->getNumElements();
    ObjectVectorPtr res = new ObjectVector(simpleSparseDoubleVectorClass, n);
    for (size_t i = 0; i < n; ++i)
    {
      MorpionActionPtr action = actions->getElement(i).getObjectAndCast<MorpionAction>();
      const_cast<MorpionState* >(this)->addLineOnBoard(action);
      SparseDoubleVectorPtr actionFeatures = features.compute(board, featuresComplexity);
      const_cast<MorpionState* >(this)->removeLineFromBoard(action);
      stateFeatures->addWeightedTo(actionFeatures, 0, -1.0);
      actionFeatures->pruneValues();
      res->set(i, actionFeatures);
    }
    return res;
  }

	virtual void performTransition(ExecutionContext& context, const Variable& ac, double& reward, Variable* stateBackup = NULL)
	{
    MorpionActionPtr action = ac.getObjectAndCast<MorpionAction>();
    addLineOnBoard(action);
		history.push_back(action);
    reward = 1.0;
    if (stateBackup)
      *stateBackup = availableActions;
    availableActions = computeAvailableActions();

#if 0
    ObjectVectorPtr dbg = computeAvailableActionsReference();
    size_t correctSize = dbg->getNumElements();
    size_t newSize = availableActions->getNumElements();
    //jassert(correctSize == newSize);
    if (correctSize != newSize)
    {
      context.enterScope(T("CorrectSize: ") + String((int)correctSize) + T(" NewSize: ") + String((int)newSize));
      context.resultCallback("state", cloneAndCast<DecisionProblemState>());
      context.resultCallback("correct", dbg);
      context.resultCallback("new", availableActions);
      context.leaveScope();
    }
    if (newSize > correctSize)
    {
      for (size_t i = 0; i < newSize; ++i)
      {
        MorpionActionPtr action = availableActions->getAndCast<MorpionAction>(i);
        size_t j;
        for (j = 0; j < correctSize; ++j)
          if (*dbg->getAndCast<MorpionAction>(j) == *action)
            break;
        if (j == correctSize)
          context.informationCallback(T("TOO MUCH ACTION: ") + action->toShortString());
      }
    }
#endif // 0
	}

	virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
	{
    availableActions = stateBackup.getObjectAndCast<ObjectVector>();
    jassert(history.size());
    MorpionActionPtr action = history.back();
    history.pop_back();
    removeLineFromBoard(action);
    return true;
	}

	virtual bool isFinalState() const
	  {return availableActions->getNumElements() == 0;}

	virtual double getFinalStateReward() const
	  {return (double)history.size();}

	virtual String toShortString() const
  {
    String res = String((int)crossLength) + (isDisjoint ? "D" : "T") + " - " + String((int)history.size());
    for (size_t i = 0; i < history.size(); ++i)
      res += T(" ") + history[i]->toShortString();
    return res;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const MorpionStatePtr& target = t.staticCast<MorpionState>();
    target->name = name;
    target->crossLength = crossLength;
    target->isDisjoint = isDisjoint;
    target->board = board;
    target->history = history;
    target->availableActions = availableActions;
  }

  size_t getCrossLength() const
    {return crossLength;}

  bool getIsDisjointFlag() const
    {return isDisjoint;}

  const MorpionBoard& getBoard() const
    {return board;}

  const std::vector<MorpionActionPtr>& getHistory() const
    {return history;}

protected:
	friend class MorpionStateClass;

  size_t crossLength;
  bool isDisjoint;

  MorpionBoard board;
  std::vector<MorpionActionPtr> history;

  ObjectVectorPtr availableActions;

  void addLineOnBoard(const MorpionActionPtr& action)
  {
    #ifdef JUCE_DEBUG
    bool ok = false;
    for (size_t i = 0; i < availableActions->getNumElements(); ++i)
      if (*availableActions->getAndCast<MorpionAction>(i) == *action)
        ok = true;
    jassert(ok);
#endif // JUCE_DEBUG
        
    int x = action->getPosition().getX();
    int y = action->getPosition().getY();
    jassert(!board.isOccupied(x, y));
    board.markAsOccupied(x, y);
   
    MorpionPoint position = action->getStartPosition();
    for (size_t i = 0; i < crossLength - 1; ++i)
    {
      jassert(!board.hasSegment(position, action->getDirection()));
      board.addSegment(position, action->getDirection());
      position.incrementIntoDirection(action->getDirection());
    }
  }

  void removeLineFromBoard(const MorpionActionPtr& action)
  {
    MorpionPoint position = action->getStartPosition();
    for (size_t i = 0; i < crossLength - 1; ++i)
    {
      board.removeSegment(position, action->getDirection());
      position.incrementIntoDirection(action->getDirection());
    }
    board.markAsOccupied(action->getPosition(), false);
  }

  ObjectVectorPtr computeAvailableActions() const
  {
    ObjectVectorPtr res = new ObjectVector(availableActions ? availableActions->getClass() : objectVectorClass(morpionActionClass));
    int minSizeX, maxSizeX, minSizeY, maxSizeY;
    board.getXRange(minSizeX, maxSizeX);
    board.getYRange(minSizeY, maxSizeY);
    for (int x = minSizeX; x <= maxSizeX; ++x)
      for (int y = minSizeY; y <= maxSizeY; ++y)
          if (!board.isOccupied(x, y) && board.isNeighbor(x, y))     
            addActionsWithPosition(MorpionPoint(x, y), res); // if it can form a line
    return res;
  }

  void addActionsWithPosition(const MorpionPoint& position, ObjectVectorPtr& res) const
  {
    for (size_t dir = MorpionDirection::NE; dir <= MorpionDirection::S; ++dir)
    {
      MorpionDirection direction((MorpionDirection::Direction)dir);
      if (board.isOccupied(position.moveIntoDirection(direction, 1)) || // fast check to discard directions
          board.isOccupied(position.moveIntoDirection(direction, -1)))
        addActionsWithPositionAndDirection(position, direction, res);
    }
  }

  void addActionsWithPositionAndDirection(const MorpionPoint& point, const MorpionDirection& direction, ObjectVectorPtr& res) const
  {
    MorpionPoint pt;
    int delta;
    
    pt = point;
    for (delta = 0; delta > 1 - (int)crossLength; --delta)
    {
      MorpionPoint nextPt = pt.moveIntoDirection(direction, -1);
      if (board.hasSegment(nextPt, direction) || !board.isOccupied(nextPt))
        break;
      pt = nextPt;
    }
    int minDelta = delta;
    if (isDisjoint && board.hasSegment(pt.moveIntoDirection(direction, -1), direction))
      ++minDelta;

    pt = point;
    for (delta = 0; delta < (int)crossLength - 1; ++delta)
    {
      MorpionPoint nextPt = pt.moveIntoDirection(direction, 1);
      if (board.hasSegment(pt, direction) || !board.isOccupied(nextPt))
        break;
      pt = nextPt;
    }
    int maxDelta = delta;
    if (isDisjoint && board.hasSegment(pt, direction))
      --maxDelta;

    int maxIndexInLine = -minDelta;
    int minIndexInLine = crossLength - 1 - maxDelta;
    for (int indexInLine = minIndexInLine; indexInLine <= maxIndexInLine; ++indexInLine)
      res->append(new MorpionAction(point, direction, indexInLine));
  }

#if 0
  ObjectVectorPtr computeAvailableActionsReference() const
  {
		ObjectVectorPtr res = new ObjectVector(morpionActionClass, 0);
    int minSizeX, maxSizeX, minSizeY, maxSizeY;
    board.getXRange(minSizeX, maxSizeX);
    board.getYRange(minSizeY, maxSizeY);
    for (int x = minSizeX - 1; x <= maxSizeX + 1; ++x)
      for (int y = minSizeY - 1; y <= maxSizeY + 1; ++y)
          if (!board.isOccupied(x,y))        
            exhaustiveList(x,y, res); // if it can form a line
    return res;    
  }

  void exhaustiveList(int x, int y, ObjectVectorPtr res) const
  {
    MorpionPoint position(x, y);
    
    for (size_t dir = 0; dir < 4; ++dir)
    { 
      MorpionDirection direction((MorpionDirection::Direction)dir);
      
      if (!board.isOccupied(position.moveIntoDirection(direction, 1)) &&
          !board.isOccupied(position.moveIntoDirection(direction, -1)))
        continue;
      
      // there are 5 different index lines
      for (size_t index=0;index<crossLength;++index)
      {
        // look if it is a possible move
        MorpionAction action(position, direction, index);
        jassert(ensureDiscontinuity(action) == ensureDiscontinuity2(action));
        if (ensureDiscontinuity(action))
        {
          bool isValid = true;
          MorpionPoint otherPosition = action.getStartPosition();
          for (size_t i = 0; i < crossLength; ++i)
          {
            if (i != index && !board.isOccupied(otherPosition))
            {
              isValid = false;
              break;
            }
            otherPosition.incrementIntoDirection(direction);
          }
            
          if (isValid)
            res->append(new MorpionAction(position, direction, index));
        }
      }
    }
  }

  static int project(const MorpionPoint& point, const MorpionDirection& direction)
    {return point.getX() * direction.getDx() + point.getY() * direction.getDy();}

  static int projectOrtho(const MorpionPoint& point, const MorpionDirection& direction)
    {return point.getX() * direction.getDy() - point.getY() * direction.getDx();}

  bool ensureDiscontinuity(const MorpionAction& action) const
  {
    MorpionDirection dir = action.getDirection();
    int a = project(action.getStartPosition(), dir);
    int b = project(action.getEndPosition(crossLength), dir);
    int o = projectOrtho(action.getPosition(), dir);
    
    jassert(a < b);

    for (size_t line = 0; line < history.size(); ++line)
      if (history[line]->getDirection() == dir && o == projectOrtho(history[line]->getPosition(), dir)) // quick check-up
      {
        int c = project(history[line]->getStartPosition(), dir);
        int d = project(history[line]->getEndPosition(crossLength), dir);
        jassert(c < d);
        
        bool ok = c > b || d < a;
        if (!ok)
          return false;
      }
    return true;
  }
  
  bool ensureDiscontinuity2(const MorpionAction& action) const
  {
    for (size_t line = 0; line < history.size(); ++line)
      if (history[line]->getDirection() == action.getDirection())
        for(size_t check=0;check<crossLength;++check) // the 5 points of the existing line
          for(size_t newLine=0;newLine<crossLength;++newLine) // the 5 points of the new line
            if(history[line]->getLinePoint(check)==action.getLinePoint(newLine))
                return false;
    return true;
  }
#endif // 0
};

extern ClassPtr morpionStateClass;

/*
** Problem
*/
class MorpionProblem : public DecisionProblem
{
public:
	MorpionProblem(size_t crossLength = 5, bool isDisjoint = false)
	  : DecisionProblem(FunctionPtr(), 1.0, 999), crossLength(crossLength), isDisjoint(isDisjoint) {}

	virtual double getMaxReward() const
	  {return 100.0;}

	virtual ClassPtr getStateClass() const
	  {return morpionStateClass;}

	virtual TypePtr getActionType() const
	  {return morpionActionClass;}

	virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
	  {return new MorpionState(crossLength, isDisjoint);}

protected:
	friend class MorpionProblemClass;

	size_t crossLength;
  bool isDisjoint;
};

/*
** User Interface
*/
class MorpionStateComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  MorpionStateComponent(MorpionStatePtr state, const String& name)
    : state(state) {}

  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getDefaultHeight() const
    {return 600;}

  virtual void paint(juce::Graphics& g)
  {
    const MorpionBoard& board = state->getBoard();

    // get board range
    int xMin, xMax, yMin, yMax;
    board.getXRange(xMin, xMax);
    board.getYRange(yMin, yMax);
    if (xMax < xMin || yMax < yMin)
      return;

    // compute board positionning
    int s1 = getWidth() / (xMax - xMin + 1);
    int s2 = getHeight() / (yMax - yMin + 1);
    int edgePixels = juce::jmin(s1, s2);
    int boardPixelsWidth = (xMax - xMin + 1) * edgePixels;
    int boardPixelsHeight = (yMax - yMin + 1) * edgePixels;
    int boardPixelsX = (edgePixels + getWidth() - boardPixelsWidth) / 2;
    int boardPixelsY = (edgePixels + getHeight() - boardPixelsHeight) / 2;

    // paint lines
    const std::vector<MorpionActionPtr>& history = state->getHistory();
    g.setColour(juce::Colours::grey);
    for (size_t i = 0; i < history.size(); ++i)
    {
      MorpionPoint startPosition = history[i]->getStartPosition();
      int sx = boardPixelsX + (startPosition.getX() - xMin) * edgePixels;
      int sy = boardPixelsY + (startPosition.getY() - yMin) * edgePixels;
      MorpionPoint endPosition = history[i]->getEndPosition(state->getCrossLength());
      int ex = boardPixelsX + (endPosition.getX() - xMin) * edgePixels;
      int ey = boardPixelsY + (endPosition.getY() - yMin) * edgePixels;
      paintLine(g, sx, sy, ex, ey, edgePixels);
    }

    // paint initial crosses
    g.setColour(juce::Colours::black);
    MorpionBoard initialBoard;
    initialBoard.initialize(state->getCrossLength());
    for (int x = initialBoard.getMinX(); x <= initialBoard.getMaxX(); ++x)
      for (int y = initialBoard.getMinY(x); y <= initialBoard.getMaxY(x); ++y)
        if (initialBoard.isOccupied(x, y))
        {
          int px = boardPixelsX + (x - xMin) * edgePixels;
          int py = boardPixelsY + (y - yMin) * edgePixels;
          paintPoint(g, px, py, edgePixels);
        }

    // paint actions
    for (size_t i = 0; i < history.size(); ++i)
    {
      MorpionPoint position = history[i]->getPosition();
      int px = boardPixelsX + (position.getX() - xMin) * edgePixels;
      int py = boardPixelsY + (position.getY() - yMin) * edgePixels;
      paintAction(g, i, px, py, edgePixels);
    }

    /* paint test
    g.setColour(juce::Colours::red);
    for (int x = board.getMinX(); x <= board.getMaxX(); ++x)
      for (int y = board.getMinY(x); y <= board.getMaxY(x); ++y)
      {
        char state = board.getState(x, y);
        int px = boardPixelsX + (x - xMin) * edgePixels;
        int py = boardPixelsY + (y - yMin) * edgePixels;

        if ((state & MorpionBoard::flagOccupied) != 0)
          paintPoint(g, px, py, edgePixels);
        ++px, ++py; // tmp !
        if ((state & MorpionBoard::flagNE) != 0)
          paintLine(g, px, py, px + edgePixels, py - edgePixels, edgePixels);
        if ((state & MorpionBoard::flagE) != 0)
          paintLine(g, px, py, px + edgePixels, py, edgePixels);
        if ((state & MorpionBoard::flagSE) != 0)
          paintLine(g, px, py, px + edgePixels, py + edgePixels, edgePixels);
        if ((state & MorpionBoard::flagS) != 0)
          paintLine(g, px, py, px, py + edgePixels, edgePixels);
      }*/
  }

  void paintPoint(juce::Graphics& g, int x, int y, int baseSize)
  {
    enum {s = 5};
    int pointHalfSize = baseSize / 10;
    float x1 = (float)(x - pointHalfSize);
    float y1 = (float)(y - pointHalfSize);
    float x2 = (float)(x + pointHalfSize);
    float y2 = (float)(y + pointHalfSize);
    float lineWidth = juce::jmax(1.f, baseSize / 20.f);
    g.drawLine(x1, y1, x2 + 1.f, y2 + 1.f, lineWidth);
    g.drawLine(x1, y2, x2 + 1.f, y1 + 1.f, lineWidth);
  }

  void paintLine(juce::Graphics& g, int startX, int startY, int endX, int endY, int baseSize)
  {
    g.drawLine((float)startX, (float)startY, (float)endX, (float)endY);
  }

  void paintAction(juce::Graphics& g, size_t index, int x, int y, int baseSize)
  {
    int pointHalfSize = baseSize / 3;
    float x1 = (float)(x - pointHalfSize);
    float y1 = (float)(y - pointHalfSize);
    float size = (float)(2 * pointHalfSize);
    float cornerSize = (float)(baseSize / 10);

    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(x1, y1, size, size, cornerSize);
    g.setColour(juce::Colours::black);    
    g.drawRoundedRectangle(x1, y1, size, size, cornerSize, 1.f);
    g.setFont(juce::Font(juce::jmax(8.f, baseSize / 3.f)));
    g.drawText(String((int)index + 1), (int)x1, (int)y1, (int)size, (int)size, juce::Justification::centred, false);
  }

protected:
  MorpionStatePtr state;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPION_H_
