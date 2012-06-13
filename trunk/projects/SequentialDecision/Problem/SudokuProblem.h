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
	// TODO test svn windows
    // TODO: loop over the board and enumerate legal moves
      
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

  virtual bool getFinalStateReward() const
    {return finalStateReward;}

protected:
  friend class SudokuStateClass;

  std::vector< std::set<size_t> > board;
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

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
  {
  	SudokuStatePtr state = new SudokuState(sudokuSize);
    // todo : fill state
  	return state;
  }

protected:
  friend class SudokuProblemClass;

  size_t sudokuSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
