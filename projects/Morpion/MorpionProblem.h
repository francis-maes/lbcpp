/*-----------------------------------------.---------------------------------.
| Filename: MorpionProblem.h               | Solitaire Morpion               |
| Author  : David Lupien St-Pierre         |                                 |
| Started : 14/06/2012 21:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MORPION_PROBLEM_H_
# define LBCPP_MORPION_PROBLEM_H_

# include <lbcpp-ml/Search.h>
# include <lbcpp-ml/Problem.h>
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
  MorpionAction(const MorpionPoint& position, const MorpionDirection& direction, size_t requestedIndexInLine, size_t indexInLine)
    : Object(morpionActionClass), position(position), direction(direction), requestedIndexInLine(requestedIndexInLine), indexInLine(indexInLine) {}
  MorpionAction() : requestedIndexInLine(-1), indexInLine(-1) {}

  const MorpionPoint& getPosition() const
    {return position;}

  const MorpionDirection& getDirection() const
    {return direction;}

  size_t getIndexInLine() const
    {return indexInLine;}

  size_t getRequestedIndexInLine() const
    {return requestedIndexInLine;}

  MorpionPoint getStartPosition() const
    {return position.moveIntoDirection(direction, -(int)indexInLine);}

  MorpionPoint getEndPosition(size_t crossLength) const
    {return position.moveIntoDirection(direction, (int)(crossLength - 1 - indexInLine));}

  MorpionPoint getLinePoint(size_t index) const
    {return position.moveIntoDirection(direction, (int)index - (int)indexInLine);}

  virtual String toShortString() const
    {return position.toString() + ", " + direction.toString() + ", " + String((int)indexInLine);}

  bool operator ==(const MorpionAction& other) const
    {return position == other.position && direction == other.direction && requestedIndexInLine == other.requestedIndexInLine;}

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
    return (int)requestedIndexInLine - (int)other->requestedIndexInLine;
  }

  virtual void saveToXml(XmlExporter& exporter) const
  {
    exporter.setAttribute("x", position.getX());
    exporter.setAttribute("y", position.getY());
    exporter.setAttribute("dir", direction.toString());
    exporter.setAttribute("ri", requestedIndexInLine);
    exporter.setAttribute("i", indexInLine);
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    position = MorpionPoint(importer.getIntAttribute("x"), importer.getIntAttribute("y"));
    direction = MorpionDirection::fromString(importer.getStringAttribute("dir"));
    requestedIndexInLine = (size_t)importer.getIntAttribute("ri");
    indexInLine = (size_t)importer.getIntAttribute("i");
    return true;
  }

private:
  friend class MorpionActionClass;

  MorpionPoint position;
  MorpionDirection direction;
  size_t requestedIndexInLine;
  size_t indexInLine; // in range [0, crossLength[
};

/*
** State
*/
class MorpionState;
typedef ReferenceCountedObjectPtr<MorpionState> MorpionStatePtr;

class MorpionState : public SearchState
{
public:
	MorpionState(size_t crossLength, bool isDisjoint)
    : crossLength(crossLength), isDisjoint(isDisjoint)
  {
    board.initialize(crossLength);
    updateAvailableActions();    
  }
  MorpionState() : crossLength(0), isDisjoint(false) {}

  virtual String toShortString() const
    {return String((int)crossLength) + (isDisjoint ? "D" : "T");}
  
	virtual String toString() const
  {
    String res = toShortString();
    for (size_t i = 0; i < history.size(); ++i)
      res += T(" ") + history[i]->toShortString();
    return res;
  }

  virtual DomainPtr getActionDomain() const
    {return availableActions;}
  
  
#if 0
  virtual DoubleVectorPtr getActionFeatures(const SearchStatePtr& state, const ObjectPtr& action) const
  {
    /* enum {featuresComplexity = 2};

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
    return res;*/
    jassertfalse;
    return DoubleVectorPtr();
  }
#endif // 0

	virtual void performTransition(ExecutionContext& context, const ObjectPtr& ac, Variable* stateBackup = NULL)
	{
    MorpionActionPtr action = ac.staticCast<MorpionAction>();
    addLineOnBoard(action);
		history.push_back(action);
    if (stateBackup)
      *stateBackup = availableActions;
    updateAvailableActions();

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

	virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
	{
    availableActions = stateBackup.getObjectAndCast<DiscreteDomain>();
    jassert(history.size());
    MorpionActionPtr action = history.back();
    history.pop_back();
    removeLineFromBoard(action);
	}

	virtual bool isFinalState() const
	  {return availableActions->getNumElements() == 0;}

  size_t getCrossLength() const
    {return crossLength;}

  bool getIsDisjointFlag() const
    {return isDisjoint;}

  const MorpionBoard& getBoard() const
    {return board;}

  const std::vector<MorpionActionPtr>& getHistory() const
    {return history;}
 
  virtual void saveToXml(XmlExporter& exporter) const
  {
    exporter.enter("history");
    exporter.setAttribute("size", history.size());
    for (size_t i = 0; i < history.size(); ++i)
    {
      exporter.enter("move");
      exporter.setAttribute("index", i);
      history[i]->saveToXml(exporter);
      exporter.leave();
    }
    exporter.leave();
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!importer.enter("history"))
    {
      importer.errorMessage("MorpionState", "No history");
      return false;
    }
    history.resize(importer.getIntAttribute("size"));
    forEachXmlChildElementWithTagName(*importer.getCurrentElement(), elt, T("move"))
    {
      importer.enter(elt);
      size_t index = (size_t)importer.getIntAttribute("index");
      MorpionActionPtr action = new MorpionAction();
      if (!action->loadFromXml(importer))
        return false;
      history[index] = action;
      importer.leave();
    }
    importer.leave();

    board.clear();
    board.initialize(crossLength);
    for (size_t i = 0; i < history.size(); ++i)
      addLineOnBoard(history[i]);
    updateAvailableActions();    
    return true;
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const MorpionStatePtr& target = t.staticCast<MorpionState>();
    target->crossLength = crossLength;
    target->isDisjoint = isDisjoint;
    target->board = board;
    target->history = history;
    target->availableActions = availableActions;
  }

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const MorpionStatePtr& other = otherObject.staticCast<MorpionState>();
    if (crossLength != other->crossLength)
      return (int)crossLength - (int)other->crossLength;
    if (isDisjoint != other->isDisjoint)
      return isDisjoint ? 1 : -1;
    if (history.size() != other->history.size())
      return (int)history.size() - (int)other->history.size();
    for (size_t i = 0; i < history.size(); ++i)
    {
      int cmp = history[i]->compare(other->history[i]);
      if (cmp != 0)
        return cmp;
    }
    return 0;
  }

protected:
	friend class MorpionStateClass;

  size_t crossLength;
  bool isDisjoint;

  MorpionBoard board;
  std::vector<MorpionActionPtr> history;

  DiscreteDomainPtr availableActions;

  void addLineOnBoard(const MorpionActionPtr& action)
  {
#ifdef JUCE_DEBUG
    bool ok = false;
    for (size_t i = 0; i < availableActions->getNumElements(); ++i)
      if (availableActions->getElement(i)->compare(action) == 0)
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

  void updateAvailableActions()
  {
    //if (!availableActions)
      availableActions = new DiscreteDomain();
   // else
   //   availableActions->clear();
    int minX, minY, maxX, maxY;
    board.getXYRange(minX, minY, maxX, maxY);
    for (int x = minX; x <= maxX; ++x)
      for (int y = minY; y <= maxY; ++y)
          if (!board.isOccupied(x, y) && board.isNeighbor(x, y))     
            addActionsWithPosition(MorpionPoint(x, y)); // if it can form a line
  }

  void addActionsWithPosition(const MorpionPoint& position) const
  {
    for (size_t dir = MorpionDirection::NE; dir <= MorpionDirection::S; ++dir)
    {
      MorpionDirection direction((MorpionDirection::Direction)dir);
      if (board.isOccupied(position.moveIntoDirection(direction, 1)) || // fast check to discard directions
          board.isOccupied(position.moveIntoDirection(direction, -1)))
        addActionsWithPositionAndDirection(position, direction);
    }
  }

  void addActionsWithPositionAndDirection(const MorpionPoint& point, const MorpionDirection& direction) const
  {
    MorpionPoint previousPt;
    int delta;
   
    // find lowest missing point and lowest existing segment
    previousPt = point;
    for (delta = -1; delta > -(int)crossLength; --delta)
    {
      MorpionPoint pt = previousPt.moveIntoDirection(direction, -1);
      if (!board.isOccupied(pt))
        break;
      previousPt = pt;
    }
    int lowestPoint = delta + 1;
    
    previousPt = isDisjoint ? point.moveIntoDirection(direction, -1) : point;
    for (delta = -1; delta >= -(int)crossLength; --delta) // FIXME: 2 * crossLength to enable action domination
    {
      MorpionPoint pt = previousPt.moveIntoDirection(direction, -1);
      if (board.hasSegment(pt, direction))
        break;
      previousPt = pt;
    }
    int lowestSegment = delta + 1;
    
    // find highest missing point and highest existing segment
    previousPt = point;
    for (delta = 1; delta < (int)crossLength; ++delta)
    {
      MorpionPoint pt = previousPt.moveIntoDirection(direction, 1);
      if (!board.isOccupied(pt))
        break;
      previousPt = pt;
    }
    int highestPoint = delta - 1;
    
    previousPt = isDisjoint ? point : point.moveIntoDirection(direction, -1);
    for (delta = 1; delta <= (int)crossLength; ++delta) // FIXME: 2 * crossLength to enable action domination
    {
      MorpionPoint pt = previousPt.moveIntoDirection(direction, 1);
      if (board.hasSegment(pt, direction))
        break;
      previousPt = pt;
    }
    int highestSegment = delta - 1;
    
    // compute minIndexInLine and maxIndexInLine
    int minDelta = juce::jmax(lowestPoint, lowestSegment);
    int maxDelta = juce::jmin(highestPoint, highestSegment);
    int maxIndexInLine = juce::jmin(crossLength - 1, -minDelta);
    int minIndexInLine = juce::jmax(0, crossLength - 1 - maxDelta);
    
    if (minIndexInLine > maxIndexInLine)
      return;
    else if (minIndexInLine == maxIndexInLine)
      availableActions->addElement(new MorpionAction(point, direction, maxIndexInLine, maxIndexInLine));
    else
    {
       for (int indexInLine = minIndexInLine; indexInLine <= maxIndexInLine; ++indexInLine)
         availableActions->addElement(new MorpionAction(point, direction, indexInLine, indexInLine));
#if 0
      /*std::cout << "Lowest Point: " << lowestPoint << " Segment: " << lowestSegment
              << ", Highest Point: " << highestPoint << " Segment: " << highestSegment
              << ", Indices: " << minIndexInLine << " -- " << maxIndexInLine << std::endl;*/
      //std::cout << "Penalties: ";
      std::vector<size_t> penalties(maxIndexInLine - minIndexInLine + 1);
      size_t bestPenalty = (size_t)-1;
      int bestIndexInLine = 0;
      int minDistance = (int)crossLength + (isDisjoint ? 1 : -1);
      for (int indexInLine = minIndexInLine; indexInLine <= maxIndexInLine; ++indexInLine)
      {
        int firstPoint = -indexInLine;
        int lastPoint = firstPoint + (int)crossLength - 1;
        //std::cout << lowestSegment << " " << firstPoint << " " << lastPoint << " " << highestSegment << std::endl;
        int distanceFromLowestSegment = firstPoint - lowestSegment;
        int distanceFromHighestSegment = highestSegment - lastPoint;
        jassert(distanceFromLowestSegment >= 0 && distanceFromHighestSegment >= 0);
        
        size_t penalty = (distanceFromLowestSegment < minDistance ? distanceFromLowestSegment : 0) +
                          (distanceFromHighestSegment < minDistance ? distanceFromHighestSegment : 0);
        //std::cout << penalty << std::flush;
        penalties[indexInLine - minIndexInLine] = penalty;
        if (penalty < bestPenalty)
          bestPenalty = penalty, bestIndexInLine = indexInLine;
      }
      //std::cout << std::endl;
      for (int indexInLine = minIndexInLine; indexInLine <= maxIndexInLine; ++indexInLine)
      {
        size_t requestedIndexInLine = (size_t)indexInLine;
        size_t obtainedIndexInLine = requestedIndexInLine;

        // change the index in line if the action is dominated
        if (penalties[indexInLine - minIndexInLine] > bestPenalty)
        {
          size_t nearestIndexDistance = crossLength;
          for (int indexInLine2 = minIndexInLine; indexInLine2 <= maxIndexInLine; ++indexInLine2)
            if (penalties[indexInLine2 - minIndexInLine] == bestPenalty)
            {
              size_t distance = (size_t)abs(indexInLine2 - indexInLine);
              if (distance < nearestIndexDistance)
                nearestIndexDistance = distance, obtainedIndexInLine = (size_t)indexInLine2;
            }
        }
        availableActions->append(MorpionAction(point, direction, requestedIndexInLine, obtainedIndexInLine));
      }
#endif // 0
    }
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
            res->append(new MorpionAction(position, direction, index, index));
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

class MorpionActionCodeGenerator : public SearchActionCodeGenerator
{
public:
  virtual size_t getCode(const SearchStatePtr& s, const ObjectPtr& a)
  {
    const MorpionStatePtr& state = s.staticCast<MorpionState>();
    const MorpionActionPtr& action = a.staticCast<MorpionAction>();
    int x = action->getPosition().getX();
    int y = action->getPosition().getY();
    int position = (x + 25) * 100 + (y + 25);
    int d = (int)(MorpionDirection::Direction)(action->getDirection());
    int indexInLine = action->getRequestedIndexInLine();
    return (size_t)(indexInLine + state->getCrossLength() * (d + 4 * position));
  }
};

class MorpionProblem : public Problem
{
public:
  MorpionProblem(size_t crossLength = 5, bool isDisjoint = false)
    : crossLength(crossLength), isDisjoint(isDisjoint)
  {
    domain = new SearchDomain(new MorpionState(crossLength, isDisjoint));
    limits = new FitnessLimits(std::vector< std::pair<double, double> >(1, std::make_pair(0.0, 200.0)));
  }

  virtual DomainPtr getDomain() const
    {return domain;}

  virtual FitnessLimitsPtr getFitnessLimits() const
    {return limits;}

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    size_t numLines = object.staticCast<SearchTrajectory>()->getLength();
    return new Fitness((double)numLines, limits);
  }

protected:
  friend class MorpionProblemClass;

  size_t crossLength;
  bool isDisjoint;

  SearchDomainPtr domain;
  FitnessLimitsPtr limits;
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
    board.getXYRange(xMin, yMin, xMax, yMax);
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
    for (int x = 0; x < (int)initialBoard.getWidth(); ++x)
      for (int y = 0; y < (int)initialBoard.getHeight(); ++y)
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

#endif // !LBCPP_MORPION_PROBLEM_H_
