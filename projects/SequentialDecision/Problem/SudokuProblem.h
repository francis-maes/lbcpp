/*-----------------------------------------.---------------------------------.
| Filename: SudokuProblem.h                | Sudoku                          |
| Author  : David Lupien St-Pierre         |                                 |
| Started : 12/06/2012 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <list>

namespace lbcpp
{

/*
** State
*/
extern ClassPtr sudokuStateClass;

class SudokuState;
typedef ReferenceCountedObjectPtr<SudokuState> SudokuStatePtr;

class SudokuState : public DecisionProblemState
{
public:
	SudokuState(size_t baseSize = 3)
    : finalState(false), baseSize(baseSize), boardSize(baseSize * baseSize) {}

	virtual TypePtr getActionType() const
	  {return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual ContainerPtr getAvailableActions() const
	{
		ClassPtr actionType = getActionType();
		ObjectVectorPtr res = new ObjectVector(actionType, 0);
    
    size_t smallest = boardSize;
    for (size_t i = 0; i < board.size(); ++i)
     if (doneList.find(i) == doneList.end())
     {
      size_t n = getNumAvailableValues(i);
      if (n < smallest)
         smallest = n;
     }

    for (size_t i = 0; i < board.size(); ++i)
    {
 	    std::vector<size_t> values = getAvailableValues(i);
      if (values.size() == smallest && doneList.find(i) == doneList.end())
        for (size_t j = 0; j < values.size(); ++j)
          res->append(new Pair(actionType, i, values[j]));  
    }
    return res;
	}

	struct Backup : public Object
	{
		Backup(const std::vector<size_t>& board, size_t lastPosition)
		: board(board), lastPosition(lastPosition) {}

		std::vector<size_t> board;
    size_t lastPosition;
	};

	virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
	{
		// First, we extract the position and the value which are encoded in the action
		PairPtr positionAndValue = action.getObjectAndCast<Pair>();
		size_t position = positionAndValue->getFirst().getInteger();
		size_t value = positionAndValue->getSecond().getInteger();

		// save in "stateBackup" information about the current state
		if (stateBackup)
			*stateBackup = Variable(new Backup(board, position), objectClass);

    // execute action && update board
    //std::vector<size_t> tmp = getAvailableValues(position);
    setValueAndRemoveFromRowColumnAndRegion(position, value);

    reward = 1.0;
  }

	void setValueAndRemoveFromRowColumnAndRegion(size_t index, size_t value)
	{
    size_t x = index % boardSize;
    size_t y = index / boardSize;
    doneList.insert(index);

    // for each row : col fixed
    // for each col : row fixed
    for (size_t i = 0; i < boardSize; ++i)
	  {
      if (i != x)
        removeValue(i, y, value);
      if (i != y)
        removeValue(x, i, value);
	  }
    // for the region
    size_t xRegion = x / baseSize;
		size_t yRegion = y / baseSize;
		for (size_t i = 0; i < baseSize; ++i)
			for (size_t j = 0; j < baseSize; ++j)
      {
        size_t ii = xRegion*baseSize+i;
        size_t jj = yRegion*baseSize+j;
        if (ii != x || jj != y)
          removeValue(ii, jj, value);
      }
   
    // set value
    setValue(x, y, value);

    // test if game is won
    if (doneList.size() == board.size())
      finalState = true;
  }

  /* remove value from the mask of a position */
	void removeValue(size_t x, size_t y, size_t value)
	{
    size_t& state = getState(x, y);
    state &= ~(1 << value);
    if (state == 0)
      finalState = true;
  }

	virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
	{
    const ReferenceCountedObjectPtr<Backup>& b = stateBackup.getObjectAndCast<Backup>();
		// restore previous state given the information stored in stackBackup
    board = b->board;
    doneList.erase(b->lastPosition);
    finalState = false;
		return false;
	}
  
  /* apply the mask across the whole board */
	void initializeBoard()
	{
		size_t initialValue = (1 << boardSize) - 1;
    board.clear();
		board.resize(boardSize * boardSize, initialValue);
    doneList.clear();
    finalState = false;
	}
  
  /* return the mask of a specific position */
	size_t getState(size_t x, size_t y) const
	  {return board[x + y * boardSize];}

	size_t& getState(size_t x, size_t y)
	  {return board[x + y * boardSize];}

	void addValue(size_t x, size_t y, size_t value)
	  {getState(x, y) |= (1 << value);}

  void setValue(size_t x, size_t y, size_t value)
    {getState(x, y) = (1 << value);}

  size_t getNumAvailableValues(size_t index) const
  {
    size_t state = board[index];
    size_t res = 0;
		size_t bit = 1;
		for (size_t i = 0; i < boardSize; ++i)
		{
			if ((state & bit) == bit)
				++res;
			bit <<= 1;
		}
    return res;
  }

  std::vector<size_t> getAvailableValues(size_t index) const
  {
    size_t state = board[index];
    std::vector<size_t> res;
		size_t bit = 1;
		for (size_t i = 0; i < boardSize; ++i)
		{
			if ((state & bit) == bit)
				res.push_back(i);
			bit <<= 1;
		}
    return res;
  }

	size_t getNumAvailableValues(size_t x, size_t y) const
    {return getNumAvailableValues(x + y * boardSize);}

	std::vector<size_t> getAvailableValues(size_t x, size_t y) const
    {return getAvailableValues(x + y * boardSize);}

  virtual bool isFinalState() const
	  {return finalState;}

	virtual double getFinalStateReward() const
	  {return (double)doneList.size();}

	virtual String toShortString() const
	{
    String res;
    for (size_t j = 0; j < boardSize; ++j)
    {
      for (size_t i = 0; i < boardSize; ++i)
        res += String((int)getState(i, j)) + T(" ");
      res += "\n";
    }
    return res;
	}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const SudokuStatePtr& target = t.staticCast<SudokuState>();
    target->board = board;
    target->doneList = doneList;
    target->finalState = finalState;
    target->baseSize = baseSize;
    target->boardSize = boardSize;
  }

  size_t getBaseSize() const
    {return baseSize;}

  size_t getBoardSize() const
    {return boardSize;}

protected:
	friend class SudokuStateClass;

	std::vector<size_t> board;
  std::set<size_t> doneList;
	bool finalState;
	size_t baseSize; // e.g. 3
  size_t boardSize; // e.g. 9, board.size = boardSize * boardSize, e.g. 81
};

/*
** Problem
*/
class SudokuProblem : public DecisionProblem
{
public:
	SudokuProblem(size_t baseSize = 3)
	: DecisionProblem(FunctionPtr(), 1.0, baseSize * baseSize * baseSize * baseSize), baseSize(baseSize) {}

	virtual double getMaxReward() const
	  {return pow((double)baseSize, 4.0);}

	virtual ClassPtr getStateClass() const
	  {return sudokuStateClass;}

	virtual TypePtr getActionType() const
	  {return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
	{
    RandomGeneratorPtr random = context.getRandomGenerator();
		SudokuStatePtr state = new SudokuState(baseSize);
    do
    {
	    state->initializeBoard();
      size_t boardSize = baseSize * baseSize;
      for (size_t i = 0; i < boardSize * boardSize && !state->isFinalState(); ++i)
        if (random->sampleBool(1.0 / 3.0))
        {
          std::vector<size_t> values = state->getAvailableValues(i);
          jassert(values.size());
          size_t value = values[random->sampleSize(values.size())];
          state->setValueAndRemoveFromRowColumnAndRegion(i, value);
        }
    }
    while (state->isFinalState());
		return state;
	}

protected:
	friend class SudokuProblemClass;

	size_t baseSize;
};

/*
** UserInterface
*/
class SudokuStateComponent : public juce::Component
{
public:
  SudokuStateComponent(SudokuStatePtr state, const String& name)
    : state(state) {}

  virtual void paint(juce::Graphics& g)
  {
    size_t baseSize = state->getBaseSize();
    size_t boardSize = state->getBoardSize();

    int sx = getWidth() / (boardSize + 1);
    int sy = getHeight() / (boardSize + 1);
    int edgePixels = juce::jmin(sx, sy);
    int size = edgePixels * boardSize;
    int x0 = (getWidth() - size) / 2;
    int y0 = (getHeight() - size) / 2;

    g.setColour(juce::Colours::black);

    juce::Font largeFont(juce::jmax(10.f, edgePixels * 0.6f));
    juce::Font smallFont(juce::jmax(8.f, edgePixels * 0.3f));

    // draw grid
    for (size_t i = 0; i <= boardSize; ++i)
    {
      size_t w = (i % baseSize) == 0 ? 2 : 1;
      for (size_t j = 0; j < w; ++j)
      {
        g.drawHorizontalLine(y0 + i * edgePixels + j, (float)x0, (float)(x0 + size + 1));
        g.drawVerticalLine(x0 + i * edgePixels + j, (float)y0, (float)(y0 + size));
      }
    }

    // draw content
    for (size_t x = 0; x < boardSize; ++x)
      for (size_t y = 0; y < boardSize; ++y)
      {
        std::vector<size_t> values = state->getAvailableValues(x, y);
        if (values.empty())
          continue;
        if (values.size() == 1)
        {
          g.setFont(largeFont);
          g.drawText(String((int)values[0]+1), x0 + x * edgePixels, y0 + y * edgePixels, edgePixels, edgePixels, juce::Justification::centred, false);
        }
        else
        {
          g.setFont(smallFont);
          size_t numLines = (values.size() + baseSize - 1) / baseSize;
          size_t valueIndex = 0;
          for (size_t line = 0; line < numLines; ++line)
          {
            String str;
            for (size_t i = 0; i < baseSize && valueIndex < values.size(); ++i, ++valueIndex)
            {
              if (str.isNotEmpty())
                str += T(" ");
              str += String((int)values[valueIndex]+1);
            }
            g.drawText(str, x0 + x * edgePixels, y0 + y * edgePixels + (line * edgePixels / numLines), edgePixels, edgePixels / numLines, juce::Justification::centred, false);
          }
        }
      }
  }

protected:
  SudokuStatePtr state;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
