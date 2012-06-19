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
    NE, E, SE, S, none
  };

  MorpionDirection(Direction direction)
    : dir(direction) {}
  MorpionDirection() : dir(none) {}

  operator Direction ()
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
  {
    MorpionPoint res(x, y);
    if (delta)
    {
      res.x += delta * direction.getDx();
      res.y += delta * direction.getDy();
    }
    return res;
  }

  bool operator ==(const MorpionPoint& other) const
    {return x == other.x && y == other.y;}
    
  bool operator !=(const MorpionPoint& other) const
    {return x != other.x || y != other.y;}

private:
  int x, y;
};

/*
** Line
*/
class MorpionLine
{
public:
  
private:
  std::vector<MorpionPoint> v;
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

    for (int i = 0; i < sizeof (lines) / sizeof (int); i += 4)
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

  bool isOccupied(const MorpionPoint& point) const
    {return b[point.getX()][point.getY()] != 0;}
  
  bool isOccupied(int x, int y) const
    {return b[x][y] != 0;}

  void markAsOccupied(const MorpionPoint& point, bool occupied = true)
    {b[point.getX()][point.getY()] = occupied ? 1 : 0;}

  void markAsOccupied(int x, int y, bool occupied = true)
    {b[x][y] = occupied ? 1 : 0;}

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
};

/*
** Action
*/
class MorpionAction : public Object
{
public:
  MorpionAction(const MorpionPoint& position, const MorpionDirection& direction, size_t indexInLine)
    : position(position), direction(direction), indexInLine(indexInLine) {}
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

private:
  MorpionPoint position;
  MorpionDirection direction;
  size_t indexInLine; // in range [0, crossLength[
};

typedef ReferenceCountedObjectPtr<MorpionAction> MorpionActionPtr;
extern ClassPtr morpionActionClass;

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
      crossLength(crossLength), isDisjoint(isDisjoint), finalState(false)
  {
    board.initialize(crossLength);
  }
  MorpionState() : crossLength(0), isDisjoint(false) {}

	virtual TypePtr getActionType() const
	  {return morpionActionClass;}

	virtual ContainerPtr getAvailableActions() const
	{
		ObjectVectorPtr res = new ObjectVector(morpionActionClass, 0);
		
    std::vector<MorpionAction> possibleMoves;
    int minSizeX = board.getMinX()-1;
    int maxSizeX = board.getMaxX()+1;
    int minSizeY;
    int maxSizeY;
    board.getYRange(minSizeY, maxSizeY);
    minSizeY--;
    maxSizeY++;

    for(int x=minSizeX;x<=maxSizeX;++x)
      for (int y=minSizeY;y<=maxSizeY;++y)
        {
          if(!board.isOccupied(x,y))        
            exhaustiveList(x,y, res);// if it can form a line
        }
    /*
    // tmp ! --
    int xMin, xMax, yMin, yMax;
    board.getXRange(xMin, xMax);
    board.getYRange(yMin, yMax);
    --xMin, ++xMax, --yMin, ++yMax;
    RandomGeneratorPtr random = new RandomGenerator();
    while (res->getNumElements() < 10)
    {
      MorpionPoint point(random->sampleInt(xMin, xMax), random->sampleInt(yMin, yMax));
      if (board.isOccupied(point))
        continue;
      res->append(new MorpionAction(point,
        MorpionDirection((MorpionDirection::Direction)random->sampleSize(4)),
        random->sampleSize(crossLength)));
    }
    // --
    */
    return res;
	}

  void exhaustiveList(int x, int y, ObjectVectorPtr res) const
  {
    // Direction TODO remove hard code
    for(size_t dir=0;dir<4;++dir)
    { // there are 5 different index lines
      for(size_t index=0;index<crossLength;++index)
      { // look if it is a possible move
        bool isValid = true;
        MorpionAction action(MorpionPoint(x,y), MorpionDirection((MorpionDirection::Direction)dir),index) ;

        for(size_t position=0;position<crossLength;++position)
        {
          if(position!=index)
          {
            if(board.isOccupied(action.getLinePoint(position)))
              {

                if(!ensureDiscontinuity(action))
                  {isValid = false; break;}
              }
            else
              {isValid = false; break;}
          }
        }
        if(isValid) // TODO make sure it works
          res->append(new MorpionAction(MorpionPoint(x,y), MorpionDirection((MorpionDirection::Direction)dir),index));
      }
    }
  }

  bool ensureDiscontinuity(MorpionAction action) const
  {
    for (size_t line = 0; line < history.size();++line)
      if(history[line]->getDirection()==action.getDirection()) // quick check-up
        for(size_t check=0;check<crossLength;++check) // the 5 points of the existing line
          for(size_t newLine=0;newLine<crossLength;++newLine) // the 5 points of the new line
            if(history[line]->getLinePoint(check)==action.getLinePoint(newLine))
                return false;
  return true;
  }


	virtual void performTransition(ExecutionContext& context, const Variable& ac, double& reward, Variable* stateBackup = NULL)
	{
    MorpionActionPtr action = ac.getObjectAndCast<MorpionAction>();
  
    board.markAsOccupied(action->getPosition());

		history.push_back(action);
    reward = 1.0;

		// save in "stateBackup" information about the current state
/*		if (stateBackup)
			*stateBackup = Variable(new Backup(board,actionsBoard), objectClass);
 */
		
	
	}

	virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
	{
    // TODO we must either remove the cross or do a full restore
    board.markAsOccupied(history.back()->getPosition(),false);
    history.pop_back();


/*		// restore previous state given the information stored in stackBackup
		board = stateBackup.getObjectAndCast<Backup>()->board;
		actionsBoard = stateBackup.getObjectAndCast<Backup>()->actionsBoard;
    */
    return true;
	}

	virtual bool isFinalState() const
	  {return finalState;}

	virtual double getFinalStateReward() const
	  {return (double)history.size();}

	virtual String toShortString() const
    {return String((int)crossLength) + (isDisjoint ? "D" : "T") + " - " + String((int)history.size());}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const MorpionStatePtr& target = t.staticCast<MorpionState>();
    target->name = name;
    target->crossLength = crossLength;
    target->isDisjoint = isDisjoint;
    target->board = board;
    target->history = history;
    target->finalState = finalState;
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

	bool finalState;
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
    const int width = getWidth();
    const int height = getHeight();

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
    g.setColour(juce::Colours::black);
    g.drawLine(x1, y1, x2 + 1.f, y2 + 1.f, lineWidth);
    g.drawLine(x1, y2, x2 + 1.f, y1 + 1.f, lineWidth);
  }

  void paintLine(juce::Graphics& g, int startX, int startY, int endX, int endY, int baseSize)
  {
    g.setColour(juce::Colours::grey);
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
