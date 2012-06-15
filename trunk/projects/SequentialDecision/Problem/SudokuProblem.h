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
	: finalState(false), finalStateReward(0.0), baseSize(baseSize), boardSize(baseSize * baseSize),
    smallest(baseSize * baseSize), biggest(0)
  {
  }

	virtual TypePtr getActionType() const
	  {return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual ContainerPtr getAvailableActions() const
	{
		ClassPtr actionType = getActionType();

		ObjectVectorPtr res = new ObjectVector(actionType, 0);
    // fill up
    for(size_t i=0;i<board.size();++i)
    {
   	  std::vector<size_t> tmp = getAvailableValues(i/boardSize,i-i/boardSize*boardSize);
      if (tmp.size() == smallest)
        for (size_t j=0;j<tmp.size();++j)
          res->append(new Pair(actionType, i, tmp[j]));   
    }
		return res;
	}

	struct Backup : public Object
	{
		Backup(const std::vector<size_t>& board)
		: board(board) {}

		std::vector<size_t> board;
	};

	virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
	{
		// First, we extract the position and the value which are encoded in the action
		PairPtr positionAndValue = action.getObjectAndCast<Pair>();
		size_t position = positionAndValue->getFirst().getInteger();
		size_t value = positionAndValue->getSecond().getInteger();

		// save in "stateBackup" information about the current state
		if (stateBackup)
			*stateBackup = Variable(new Backup(board), objectClass);

    // execute action && update board
    size_t row = position/boardSize;
    size_t col = position%boardSize;
    std::vector<size_t> tmp = getAvailableValues(row,col);
    for (size_t j = 0; j < tmp.size(); ++j)
    {
      if (tmp[j] == value)
      {
        addValue(row, col, value);
        updateActions(row, col, value);
      }
      else // keep only the value for this position
        removeValue(row, col, tmp[j]);
    }
    prepareNextIteration();
	}

	void updateActions(size_t row, size_t col, size_t value)
	{
    // for each row : col fixed
    // for each col : row fixed
    for (size_t i = 0; i < boardSize; ++i)
		{
			removeValue(i, col, value);
			removeValue(row, i, value);
		}
    // for the region
    size_t rowRegion = row / baseSize;
		size_t colRegion = col / baseSize;
		for (size_t i = 0; i < baseSize; ++i)
			for (size_t j = 0; j < baseSize; ++j)
			 if (i != row || j != col) // already removed or if row=i && col=j ; we dont want to remove
        removeValue(rowRegion*baseSize+i, colRegion*baseSize+j, value);
	}

	virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
	{
		// restore previous state given the information stored in stackBackup
		//board = stateBackup.getObjectAndCast<Backup>()->board;
			return false;
	}
  /* apply the mask across the whole board */
	void initializeBoard()
	{
		size_t initialValue = (1 << boardSize) - 1;
    board.clear();
		board.resize(boardSize * boardSize, initialValue);
	}
  /* return the mask of a specific position */
	size_t getState(size_t row, size_t column) const
	  {return board[row * boardSize + column];}

	size_t& getState(size_t row, size_t column)
	  {return board[row * boardSize + column];}

  /* remove value from the mask of a position */
	void removeValue(size_t row, size_t column, size_t value)
	{
		size_t& state = getState(row, column);
		state &= ~(1 << value);
	}

	void addValue(size_t row, size_t column, size_t value)
	{
		size_t& state = getState(row, column);
		state |= (1 << value);
	}

	std::vector<size_t> getAvailableValues(size_t row, size_t column) const
	{
		size_t state = getState(row, column);

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

  void prepareNextIteration()
  {
  // prepare next iteration
    biggest = 0;
    smallest = boardSize;
    for(size_t i=0;i<board.size();++i)
    {
   	std::vector<size_t> tmp = getAvailableValues(i/boardSize,i-i/boardSize*boardSize);
    if(tmp.size()<smallest)
      smallest=tmp.size();
    if(tmp.size()>biggest)
      biggest=tmp.size();
    }
    // check end game
    if(smallest==0)
      {finalState=true;finalStateReward=0.0;}
    if(biggest==1)
      {finalState=true;finalStateReward=1.0;}
  }



  virtual bool isFinalState() const
	  {return finalState;}

  void setFinalState(bool value)
    {finalState = value;}

  void setReward(double value)
    {finalStateReward = 0.0;}

	virtual double getFinalStateReward() const
	  {return finalStateReward;}

	virtual String toShortString() const
	{
    String res;
    for (size_t i = 0; i < boardSize; ++i)
    {
      for (size_t j = 0; j < boardSize; ++j)
        res += String(board[i]) + T(" ");
      res += "\n";
    }
    return res;
	}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const SudokuStatePtr& target = t.staticCast<SudokuState>();
    target->board = board;
    target->finalState = finalState;
    target->finalStateReward = finalStateReward;
    target->baseSize = baseSize;
    target->boardSize = boardSize;
    target->smallest = smallest;
    target->biggest = biggest;
  }

  void printBoard()
  {
    for(size_t i=0;i<board.size();++i)
    {
      if (i % boardSize == 0)
        std::cout << "\n";
      std::cout << board[i] <<" ";
    }
  }

  size_t getBaseSize() const
    {return baseSize;}

  size_t getBoardSize() const
    {return boardSize;}

protected:
	friend class SudokuStateClass;

	std::vector<size_t> board;
	bool finalState;
	double finalStateReward;
	size_t baseSize;
  size_t boardSize;
  size_t smallest;
  size_t biggest;
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
	  {return 1.0;}

	virtual ClassPtr getStateClass() const
	  {return sudokuStateClass;}

	virtual TypePtr getActionType() const
	  {return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
	{
		SudokuStatePtr state = new SudokuState(baseSize);

    RandomGeneratorPtr random = context.getRandomGenerator();
	  state->initializeBoard();
    state->printBoard();
    
    size_t boardSize = baseSize * baseSize;

    double thres = 1.0/3;
    for (size_t i = 0; i < boardSize * boardSize; ++i)
      if (random->sampleDouble() < thres)
      {
        size_t row = i / boardSize;
	      size_t col = i % boardSize;
        std::vector<size_t> tmp = state->getAvailableValues(row, col);
        if (tmp.size() > 0)
        {
          //TODO must be a better way...
          size_t value = tmp[random->sampleSize(tmp.size())];
          for (size_t j = 0; j < tmp.size(); ++j)
          {
            if (tmp[j] == value)
            {
              state->addValue(row, col, value);
              state->updateActions(row, col, value);
            }
            else // keep only the value for this position
              state->removeValue(row, col, tmp[j]);
          }
        }
        //TODO do we give .5 ...
        else // game generated cannot be solved
        {
          state->setFinalState(true);
          state->setReward(0.0);
        }
      }

    state->prepareNextIteration();
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
        std::vector<size_t> values = state->getAvailableValues(y, x);
        if (values.empty())
          continue;
        if (values.size() == 1)
        {
          g.setFont(largeFont);
          g.drawText(String(values[0]+1), x0 + x * edgePixels, y0 + y * edgePixels, edgePixels, edgePixels, juce::Justification::centred, false);
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
              str += String(values[valueIndex]+1);
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
