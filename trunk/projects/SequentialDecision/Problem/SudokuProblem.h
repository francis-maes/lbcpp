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
	SudokuState(size_t sudokuSize = 3) : board(sudokuSize * sudokuSize) {}

  virtual TypePtr getActionType() const
    {return pairClass(positiveIntegerType, positiveIntegerType);}

  virtual ContainerPtr getAvailableActions() const
  {
  	ClassPtr actionType = getActionType();

    ObjectVectorPtr res = new ObjectVector(actionType, 0);

    //res->append(new Pair(actionType, position, number));

    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
  }

protected:
  friend class SudokuStateClass;

  std::vector< std::set<size_t> > board;
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
