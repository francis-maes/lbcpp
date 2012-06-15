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

namespace lbcpp
{

extern ClassPtr sudokuStateClass;

class SudokuState : public DecisionProblemState
{
public:
	SudokuState(size_t sudokuSize = 3)
	: board(sudokuSize * sudokuSize), finalState(false), finalStateReward(0.0), sudokuSize(sudokuSize), boardSize(sudokuSize * sudokuSize),
    smallest(sudokuSize * sudokuSize),biggest(0){}

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
    if(tmp.size()==smallest)
        for(size_t j=0;j<tmp.size();++j)
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
    size_t col = position-position/boardSize*boardSize;
    std::vector<size_t> tmp = getAvailableValues(row,col);
    for(size_t j=0;j<tmp.size();++j)
    {
     if(tmp[j]==value)
      { addValue(row,col,value);
        updateActions(row,col,boardSize,value);
      }
      else // keep only the value for this position
        removeValue(row,col,tmp[j]);
    }
    prepareNextIteration();
	}

	void updateActions(size_t row, size_t col, size_t bound, size_t value)
	{
    // for each row : col fixed
    // for each col : row fixed
    for(size_t i=0;i<bound;++i)
		{
			removeValue(i*bound,col,value);
			removeValue(row,i,value);
		}
    // for the region
    size_t rowRegion = row/sudokuSize;
		size_t colRegion = col/sudokuSize;
		for(size_t i=0;i<sudokuSize;++i)
			for(size_t j=0;j<sudokuSize;++j)
			 if(i!=row || j!=col) // already removed or if row=i && col=j ; we dont want to remove
        removeValue(rowRegion*sudokuSize+i,colRegion*sudokuSize+j,value);	
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
  {finalState=value;}

  void setReward(double value)
  {finalStateReward=0.0;}

	virtual double getFinalStateReward() const
	{return finalStateReward;}

	virtual String toShortString() const
	{
		//	printBoard();
		return "coucou";
	}

  void printBoard()
  {
  for(size_t i=0;i<board.size();++i)
  {
  if (i%boardSize==0)
    std::cout<<"\n";
  std::cout<<board[i]<<" ";
  }

  }


	std::vector<size_t> board;
  protected:
	friend class SudokuStateClass;

	bool finalState;
	double finalStateReward;
	size_t sudokuSize;
  size_t boardSize;
  size_t smallest;
  size_t biggest;
};

typedef ReferenceCountedObjectPtr<SudokuState> SudokuStatePtr;

class SudokuProblem : public DecisionProblem
{
public:
	SudokuProblem(size_t sudokuSize = 3)
	: DecisionProblem(FunctionPtr(), 1.0, sudokuSize * sudokuSize * sudokuSize * sudokuSize), sudokuSize(sudokuSize),
    row(0), col(0), bound(sudokuSize*sudokuSize) {std::cout<<"create prob"<<std::endl;}

	virtual double getMaxReward() const
	{return 1.0;}

	virtual ClassPtr getStateClass() const
	{return sudokuStateClass;}

	virtual TypePtr getActionType() const
	{return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context)
	{

   std::cout<<"started prlb l 230"<<std::endl;
		SudokuStatePtr state = new SudokuState(sudokuSize);

    std::cout<<"started prlb l 233"<<std::endl;

	  state->initializeBoard();

    state->printBoard();
    
    double thres = 1.0/3;
    for(size_t i=0;i<state->board.size();++i)
      if(context.getRandomGenerator()->sampleDouble()<thres)
        {
          row = i/bound;
		      col = i - row*bound;
          std::vector<size_t> tmp=state->getAvailableValues(row,col);
          if(tmp.size()>0)
          {
          //TODO must be a better way...
           size_t value=tmp[context.getRandomGenerator()->sampleSize(tmp.size())];
           for(size_t j=0;j<tmp.size();++j)
           {
            if(tmp[j]==value)
             { state->addValue(row,col,value);
               state->updateActions(row,col,bound,value);
             }
            else // keep only the value for this position
              state->removeValue(row,col,tmp[j]);
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

	size_t sudokuSize;
  size_t row;
  size_t col;
  size_t bound;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
