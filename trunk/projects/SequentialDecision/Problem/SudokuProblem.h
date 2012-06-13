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
	SudokuState(size_t sudokuSize = 3) : board(sudokuSize * sudokuSize), finalState(false), finalStateReward(0.0) {	std::cout<<"came l22"<<std::endl;}

	virtual TypePtr getActionType() const
    {return pairClass(positiveIntegerType, positiveIntegerType);}

  virtual ContainerPtr getAvailableActions() const
  {
  	ClassPtr actionType = getActionType();

    ObjectVectorPtr res = new ObjectVector(actionType, 0);
	
   	// first get the board position where it is the smallest
	size_t smallest=sudokuSize*sudokuSize;
	for(size_t i=0;i<board.size();++i)
		for(size_t j=0;j<board.size();++j)
			if(board[i][j]==0 && actionsBoard[i][j].size()<smallest)
				smallest=actionsBoard[i][j].size();

	// now enumerate all legal moves
	for(size_t i=0;i<board.size();++i)
		for(size_t j=0;j<board.size();++j)
			if(board[i][j]==0 && actionsBoard[i][j].size()==smallest)
			{
				set<size_t>::iterator it;
				for(it = actionsBoard[i][j].begin();it != actionsBoard[i][j].end();it++)
					res->append(new Pair(actionType, i*sudokuSize+j, *it));
			}
    return res;
  }

  	struct Backup : public Object
	{
	 Backup(const std::vector<std::vector<size_t> >& board,
			const std::vector<std::vector<std::set<size_t> > >& actionsBoard)
      : board(board),actionsBoard(actionsBoard) {}

	 //TODO why do I need this!!!
	 Backup(){}
		std::vector< std::vector<size_t> > board;
		std::vector<std::vector<std::set<size_t> > > actionsBoard;
	};

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    // First, we extract the position and the value which are encoded in the action
    PairPtr positionAndValue = action.getObjectAndCast<Pair>();
    size_t position = positionAndValue->getFirst().getInteger();
    size_t value = positionAndValue->getSecond().getInteger();
    
    // save in "stateBackup" information about the current state
     if (stateBackup)
      *stateBackup = Variable(new Backup(board,actionsBoard), objectClass);

    // update board by setting value at position
	 size_t row = position/board.size();
	 size_t col = position - row*board.size();
	 board[row][col]=value;

	 //update move list
	 updateActions(row,col, sudokuSize, value);
	
	// check if the game is finished
	// compute final reward: either 1 (win) or 0 (loose)
    if (isLost()) 
    {
		finalState = true;
		finalStateReward = 0.0; 
    }
	else if(isWon())
	{
		finalState = true;
		finalStateReward = 1.0; 
	}
  
  }

    virtual void updateActions(size_t row, size_t col, size_t bound, size_t value)
  {
	  // remove from row and col
	  for(size_t i=0;i<bound*bound;++i)
	  {
		actionsBoard[row][i].erase(value);
		actionsBoard[i][col].erase(value);
	  }
	  // find region3
	  size_t rowRegion = row/bound;
	  size_t colRegion = col/bound;
	  for(size_t i=0;i<bound;++i)
		  for(size_t j=0;j<bound;++j)
			  actionsBoard[rowRegion*bound+i][colRegion*bound+j].erase(value);
  }

  virtual bool isWon()
  {
	  for(size_t i=0;i<board.size();++i)
		  for(size_t j=0;j<board[i].size();++j)
			  if(board[i][j]==0)
				  return false;
	  return true;
  }
    virtual bool isLost()
  {
	  for(size_t i=0;i<board.size();++i)
		  for(size_t j=0;j<board[i].size();++j)
			  if(board[i][j]==0)
				  if(actionsBoard[i][j].size()<1)
					  return true;
	  return false;
  }



  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    // restore previous state given the information stored in stackBackup
	  board = stateBackup.getObjectAndCast<Backup>()->board;
	  actionsBoard = stateBackup.getObjectAndCast<Backup>()->actionsBoard;
    return false;
  }

  virtual bool isFinalState() const
    {return finalState;}

  virtual double getFinalStateReward() const
    {return finalStateReward;}



  std::vector< std::vector<size_t> > board;
  std::vector<std::vector<std::set<size_t> > > actionsBoard;
  
protected:
  friend class SudokuStateClass;

  bool finalState;
  double finalStateReward;
  size_t sudokuSize;
};

typedef ReferenceCountedObjectPtr<SudokuState> SudokuStatePtr;

class SudokuProblem : public DecisionProblem
{
public:
	SudokuProblem(size_t sudokuSize = 3)
    : DecisionProblem(FunctionPtr(), 1.0, sudokuSize * sudokuSize), sudokuSize(sudokuSize) {	std::cout<<"came l167"<<std::endl;}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual ClassPtr getStateClass() const
    {return sudokuStateClass;}

  virtual TypePtr getActionType() const
    {return pairClass(positiveIntegerType, positiveIntegerType);}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context)
  {
	  std::cout<<"came l180"<<std::endl;
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

	std::cout<<"came l207"<<std::endl;

	//TODO build protection
	// fill X% of the board
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
							state->updateActions(i,j, sudokuSize, *it);
							break;
						}
						else
							++count;				
			}
  	return state;
  }



protected:
  friend class SudokuProblemClass;

  size_t sudokuSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
