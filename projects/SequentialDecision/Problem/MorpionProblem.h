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

namespace lbcpp
{

/*
** Direction
*/
class MorpionDirection
{
public:
  enum Direction
  {
    NE = 0, E, SE, S, none
  };

  MorpionDirection(Direction direction)
    : dir(direction) {}
  MorpionDirection() : dir(none) {}

  operator Direction () const
    {return dir;}

  int getDx() const
    {static int dxs[] = {1, 1, 1, 0, 0}; return dxs[dir];}

  int getDy() const
    {static int dys[] = {-1, 0, 1, 1, 0}; return dys[dir];}

  String toString() const
    {static String strs[] = {T("NE"), T("E"), T("SE"), T("S"), T("none")}; return strs[dir];}
    
  bool operator ==(const MorpionDirection& other) const
    {return dir == other.dir;}

  bool operator !=(const MorpionDirection& other) const
    {return dir != other.dir;}

private:
  Direction dir;
};

/*
** Point
*/
class MorpionPoint
{
public:
  MorpionPoint(int x, int y)
    : x(x), y(y) {}
  MorpionPoint() : x(0), y(0) {}

  int getX() const
    {return x;}

  int getY() const
    {return y;}

  String toString() const
    {return T("(") + String(x) + T(", ") + String(y) + T(")");}

  MorpionPoint moveIntoDirection(const MorpionDirection& direction, int delta) const
    {return MorpionPoint(x + delta * direction.getDx(), y + delta * direction.getDy());}
  
  void incrementIntoDirection(const MorpionDirection& direction)
    {x += direction.getDx(); y += direction.getDy();}

  bool operator ==(const MorpionPoint& other) const
    {return x == other.x && y == other.y;}
    
  bool operator !=(const MorpionPoint& other) const
    {return x != other.x || y != other.y;}

private:
  int x, y;
};

/*
** Board
*/
template<class T>
class MorpionBidirectionalVector
{
public:
  MorpionBidirectionalVector(size_t initialNegSize, size_t initialPosSize, const T& initialValue)
    : neg(initialNegSize, initialValue), pos(initialPosSize, initialValue), initialValue(initialValue) {}
  MorpionBidirectionalVector() {}

  const T& operator [](int index) const
    {return index >= 0 ? get(pos, posIndex(index)) : get(neg, negIndex(index));}
  
  T& operator [](int index)
    {return index >= 0 ? resizeAndGet(pos, posIndex(index)) : resizeAndGet(neg, negIndex(index));}

  size_t getPositiveSize() const
    {return pos.size();}

  size_t getNegativeSize() const
    {return neg.size();}

  int getMinIndex() const
    {return -(int)neg.size();}

  int getMaxIndex() const
    {return pos.size() - 1;}

private:
  std::vector<T> neg;
  std::vector<T> pos;
  T initialValue;

  inline size_t posIndex(int index) const
    {return (size_t)index;}

  inline size_t negIndex(int index) const
    {return (size_t)(-(index + 1));}

  const T& get(const std::vector<T>& v, size_t index) const
    {return index < v.size() ? v[index] : initialValue;}

  T& resizeAndGet(std::vector<T>& v, size_t index)
  {
    if (v.size() <= index)
      v.resize(index + 1, initialValue);
    return v[index];
  }
};

class MorpionBoard
{
public:
  MorpionBoard() : b(0, 0, MorpionBidirectionalVector< char >(0, 0, 0)) {}

  enum
  {
    flagOccupied = 0x01,
    flagNE = 0x02,
    flagE = 0x04,
    flagSE = 0x08,
    flagS = 0x10,
    flagNeighbor = 0x20,
  };

  void initialize(size_t crossLength)
  {
    int n = crossLength - 2;
    int lines[] = {
            n, 0, 1, 0,
      0, n, 1, 0,    2 * n, n, 1, 0,
      0, 2 * n, 1, 0,    2 * n, 2 * n, 1, 0,
            n, 3 * n, 1, 0,
      n, 0, 0, 1,     2 * n, 0, 0, 1,
      0, n, 0, 1,     3 * n, n, 0, 1,
      n, 2 * n, 0, 1, 2 * n, 2 * n, 0, 1
    };

    for (size_t i = 0; i < sizeof (lines) / sizeof (int); i += 4)
    {
      int x = lines[i];
      int y = lines[i+1];
      int dx = lines[i+2];
      int dy = lines[i+3];
      for (int j = 0; j <= n; ++j)
      {
        markAsOccupied(x, y);
        x += dx;
        y += dy;
      }
    }
  }

  char getState(int x, int y) const
    {return b[x][y];}

  bool isOccupied(int x, int y) const
    {return (b[x][y] & flagOccupied) == flagOccupied;}

  bool isOccupied(const MorpionPoint& point) const
    {return isOccupied(point.getX(), point.getY());}
  
  void markAsOccupied(int x, int y, bool occupied = true)
  {
    if (occupied)
    {
      b[x][y] |= flagOccupied;
      markAsNeighbor(x - 1, y - 1);
      markAsNeighbor(x - 1, y);
      markAsNeighbor(x - 1, y + 1);
      markAsNeighbor(x, y - 1);
      markAsNeighbor(x, y + 1);
      markAsNeighbor(x + 1, y - 1);
      markAsNeighbor(x + 1, y);
      markAsNeighbor(x + 1, y + 1);
    }
    else
    {
      b[x][y] &= ~flagOccupied;
      undoNeighborState(x - 1, y - 1);
      undoNeighborState(x - 1, y);
      undoNeighborState(x - 1, y + 1);
      undoNeighborState(x, y - 1);
      undoNeighborState(x, y + 1);
      undoNeighborState(x + 1, y - 1);
      undoNeighborState(x + 1, y);
      undoNeighborState(x + 1, y + 1);
    }
  }

  bool isNeighbor(int x, int y) const
    {return (b[x][y] & flagNeighbor) == flagNeighbor;}

  void markAsOccupied(const MorpionPoint& point, bool occupied = true)
    {markAsOccupied(point.getX(), point.getY(), occupied);}

  void addSegment(const MorpionPoint& point, const MorpionDirection& direction)
    {b[point.getX()][point.getY()] |= getFlag(direction);}

  void removeSegment(const MorpionPoint& point, const MorpionDirection& direction)
    {b[point.getX()][point.getY()] &= ~getFlag(direction);}

  bool hasSegment(const MorpionPoint& point, const MorpionDirection& direction) const
    {return (b[point.getX()][point.getY()] & getFlag(direction)) != 0;}

  int getMinX() const
    {return b.getMinIndex();}

  int getMaxX() const
    {return b.getMaxIndex();}

  int getMinY(int x) const
    {return b[x].getMinIndex();}

  int getMaxY(int x) const
    {return b[x].getMaxIndex();}

  void getXRange(int& minIndex, int& maxIndex) const
    {minIndex = getMinX(); maxIndex = getMaxX();}

  void getYRange(int& minIndex, int& maxIndex) const
  {
    int x1 = getMinX();
    int x2 = getMaxX();
    minIndex = 0x7FFFFFFF;
    maxIndex = -0x7FFFFFFF;
    for (int x = x1; x <= x2; ++x)
    {
      minIndex = juce::jmin(minIndex, getMinY(x));
      maxIndex = juce::jmax(maxIndex, getMaxY(x));
    }
  }

private:
  MorpionBidirectionalVector< MorpionBidirectionalVector< char > > b;

  static int getFlag(const MorpionDirection::Direction& dir)
  {
    switch (dir)
    {
    case MorpionDirection::NE: return flagNE;
    case MorpionDirection::E: return flagE;
    case MorpionDirection::SE: return flagSE;
    case MorpionDirection::S: return flagS;
    default: jassert(false); return 0;
    }
  }

  void undoNeighborState(int x, int y)
  {
    if (!computeIfNeighbor(x, y))
      b[x][y] &= ~flagNeighbor;
  }

  bool computeIfNeighbor(int x, int y) const
  {
    return isOccupied(x - 1, y - 1) || isOccupied(x - 1, y) || isOccupied(x - 1, y + 1) ||
            isOccupied(x, y - 1) || isOccupied(x, y + 1) ||
            isOccupied(x + 1, y - 1) || isOccupied(x + 1, y) || isOccupied(x + 1, y + 1);
  }

  void markAsNeighbor(int x, int y)
    {b[x][y] |= flagNeighbor;}
};

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

  SparseDoubleVectorPtr computeActionFeatures(const MorpionActionPtr& action) const
  {
    SparseDoubleVectorPtr res = new SparseDoubleVector(simpleSparseDoubleVectorClass);

    int x = action->getPosition().getX();
    int y = action->getPosition().getY();
    int d = (int)(MorpionDirection::Direction)action->getDirection();
    int i = action->getIndexInLine();

    int position = (x + 50) * 100 + (y + 50);
    int index = i + crossLength * (d + 4 * position);
    if (index >= 0)
      res->appendValue((size_t)index, 1.0);

    return res;
  }

  virtual ObjectVectorPtr computeActionFeatures(const ContainerPtr& actions) const
  {
    size_t n = actions->getNumElements();
    ObjectVectorPtr res = new ObjectVector(simpleSparseDoubleVectorClass, n);
    for (size_t i = 0; i < n; ++i)
    {
      MorpionActionPtr action = actions->getElement(i).getObjectAndCast<MorpionAction>();
      res->set(i, computeActionFeatures(action));
    }
    return res;
  }

	virtual void performTransition(ExecutionContext& context, const Variable& ac, double& reward, Variable* stateBackup = NULL)
	{
    MorpionActionPtr action = ac.getObjectAndCast<MorpionAction>();
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

    MorpionPoint position = action->getStartPosition();
    for (size_t i = 0; i < crossLength - 1; ++i)
    {
      board.removeSegment(position, action->getDirection());
      position.incrementIntoDirection(action->getDirection());
    }
    board.markAsOccupied(action->getPosition(), false);
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
