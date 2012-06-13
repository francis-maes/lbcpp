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
	SudokuState(size_t sudokuSize = 3) : board(sudokuSize * sudokuSize), finalState(false), finalStateReward(0.0) {}
  virtual TypePtr getActionType() const
    {return pairClass(positiveIntegerType, positiveIntegerType);}

  virtual ContainerPtr getAvailableActions() const
  {
  	ClassPtr actionType = getActionType();

    ObjectVectorPtr res = new ObjectVector(actionType, 0);
	
    // TODO: loop over the board and enumerate legal moves
    // first insert every number
    // example:
    //res->append(new Pair(actionType, position, value));
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    // First, we extract the position and the value which are encoded in the action
    PairPtr positionAndValue = action.getObjectAndCast<Pair>();
    int position = positionAndValue->getFirst().getInteger();
    int value = positionAndValue->getSecond().getInteger();
    
    // TODO: save in "stateBackup" information about the current state
    
    // TODO: update board by setting value at position
  
    if (false) // TODO: check if the game is finished
    {
      finalState = true;
      finalStateReward = 1.0; // TODO: compute final reward: either 1 (win) or 0 (loose)
    }
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    // restore previous state given the information stored in stackBackup
    return false;
  }

  virtual bool isFinalState() const
    {return finalState;}

  virtual double getFinalStateReward() const
    {return finalStateReward;}

  //virtual std::vector< std::vector<size_t> >& getBoard(){return board;}
  std::vector< std::vector<size_t> > board;
  std::vector<std::vector<std::set<size_t> > > actionsBoard;
protected:
  friend class SudokuStateClass;

  
  bool finalState;
  double finalStateReward;
};

typedef ReferenceCountedObjectPtr<SudokuState> SudokuStatePtr;

class SudokuProblem : public DecisionProblem
{
public:
	SudokuProblem(size_t sudokuSize = 3)
    : DecisionProblem(FunctionPtr(), 1.0, sudokuSize * sudokuSize), sudokuSize(sudokuSize) {}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual ClassPtr getStateClass() const
    {return sudokuStateClass;}

  virtual TypePtr getActionType() const
    {return pairClass(positiveIntegerType, positiveIntegerType);}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context)
  {
  	SudokuStatePtr state = new SudokuState(sudokuSize);
   	
	// prepare a list of legal actions 
	std::set<size_t> numbers;
	for(size_t i=0;i<sudokuSize*sudokuSize;++i)
		numbers.insert(i);

	std::vector<size_t> zeros;
	for(size_t i=0;i<sudokuSize*sudokuSize;++i)
		zeros.push_back(0);

	// generate the possibility board and the real board	
	for (size_t i=0;i<sudokuSize*sudokuSize;++i)
		{
			std::vector<std::set<size_t> > region;
			std::vector<size_t> boardZeros;
			boardZeros=zeros;
			for (size_t j=0;j<sudokuSize*sudokuSize;++j)
			{
				std::set<size_t> actions;
				actions = numbers;
				region.push_back(actions);
			}
			state->actionsBoard.push_back(region);
			state->board.push_back(boardZeros);
		}

	// fill 33% of the board
	double thres = 1.0/3;

	for(size_t i=0;i<sudokuSize*sudokuSize;++i)
		for(size_t j=0;j<sudokuSize*sudokuSize;++j)
			if(context.getRandomGenerator()->sampleDouble()<thres)
			{
				size_t pos = context.getRandomGenerator()->sampleSize(state->actionsBoard[i][j].size());
				jassert(state->board[i][j]==0);

				size_t count = 0;
				set<size_t>::iterator it;
				for(it = state->actionsBoard[i][j].begin();it != state->actionsBoard[i][j].end();it++)
						if(count==pos)
						{
							state->board[i][j]=*it;
							updateActions(state,i,j, sudokuSize, *it);
							break;
						}
						else
							++count;				
			}
  	return state;
  }

  virtual void updateActions(SudokuStatePtr state, size_t row, size_t col, size_t bound, size_t value)
  {
	  // remove from row
	  for(size_t i=0;i<bound*bound;++i)
	  {


	  }
	  // remove from col

	  // remove from region
  }



protected:
  friend class SudokuProblemClass;

  size_t sudokuSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
