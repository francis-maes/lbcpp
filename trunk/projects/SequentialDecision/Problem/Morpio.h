/*-----------------------------------------.---------------------------------.
| Filename: SudokuProblem.h                | Sudoku                          |
| Author  : David Lupien St-Pierre         |                                 |
| Started : 14/06/2012 21:30ggggg               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPIO_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPIO_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{
struct Point
{
public:
    Point() : x(0), y(0) {}
    Point(int inX, int inY) : x(inX), y(inY) {}
    int x, y;
};

bool operator <(const Point & lhs, const Point & rhs) // lhs = left-hand side
                                                      // rhs = right-hand side
{
    if (lhs.x != rhs.x)
    {
        return lhs.x < rhs.x;
    }
    else
    {
        return lhs.y < rhs.y;
    }
}
extern ClassPtr morpioStateClass;

class MorpioState : public DecisionProblemState
{
public:
	MorpioState(char rule = 'T',size_t crossLength = 5, size_t totalMoves =0)
	: finalState(false), finalStateReward(0.0), rule(rule), crossLength(crossLength) {}

	virtual TypePtr getActionType() const
	{return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual ContainerPtr getAvailableActions() const
	{
		ClassPtr actionType = getActionType();

		ObjectVectorPtr res = new ObjectVector(actionType, 0);

		
		//TODO get action list
    std::set<std::pair<Point, char> >::iterator it;
    for(it = actionsBoard.begin(); it != actionsBoard.end(); it++)
     size_t rere=0; // TODO find correct objectvector constructor
       // res->append(actionType, it&); TODO pkoi marche pas
		return res;
	}
  
  /*
	struct Backup : public Object
	{
		Backup(const std::unordered_map<char,int>& board,
				const std::unordered_map<char,int>& actionsBoard)
		: board(board),actionsBoard(actionsBoard) {}

		std::unordered_map<char,int> board;
		std::unordered_map<char,int> actionsBoard;
	};
  */
	virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
	{
		// First, we extract the position and the value which are encoded in the action
		//TODO to change
		PairPtr positionAndValue = action.getObjectAndCast<Pair>();
		size_t position = positionAndValue->getFirst().getInteger();
		size_t value = positionAndValue->getSecond().getInteger();

		// save in "stateBackup" information about the current state
/*		if (stateBackup)
			*stateBackup = Variable(new Backup(board,actionsBoard), objectClass);
 */
		//TODO add point on board

    // TODO remove from actionBoard

    // TODO update actionsBoard
	

    // TODO check for end game
	

	}

	



	virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
	{
/*		// restore previous state given the information stored in stackBackup
		board = stateBackup.getObjectAndCast<Backup>()->board;
		actionsBoard = stateBackup.getObjectAndCast<Backup>()->actionsBoard;
*/		return false;
	}

	virtual bool isFinalState() const
	{return finalState;}

	virtual double getFinalStateReward() const
	{return finalStateReward;}

	virtual String toShortString() const
	{
		return "coucou";
	}
 //std::make_pair 
	std::map<Point, std::vector<bool> > board;
	std::set<std::pair<Point, char> > actionsBoard;

	protected:
	friend class MorpioStateClass;

	bool finalState;
	double finalStateReward;
	char rule;
	size_t crossLength;
	size_t totalMoves;

};

typedef ReferenceCountedObjectPtr<MorpioState> MorpioStatePtr;

class MorpioProblem : public DecisionProblem
{
public:
	MorpioProblem(size_t crossLength = 5)
	: DecisionProblem(FunctionPtr(), 1.0, 999), crossLength(crossLength) {}

	virtual double getMaxReward() const
	{return 999.0;}

	virtual ClassPtr getStateClass() const
	{return morpioStateClass;}

	virtual TypePtr getActionType() const
	{return pairClass(positiveIntegerType, positiveIntegerType);}

	virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context)
	{
		MorpioStatePtr state = new MorpioState(crossLength);

    Point current (0,0);
		// TODO use mask instead
    // indicates legal directions
    // starts North, clockwise
    std::vector<bool> start;
    for(size_t i=0;i<8;++i)
        start.push_back(true);

    // fill the board :
    current = add(state, crossLength, 'E',current.x,current.y,start);
    current = add(state, crossLength, 'S',current.x,current.y,start);
    current = add(state, crossLength, 'E',current.x,current.y,start);
    current = add(state, crossLength, 'N',current.x,current.y,start);
    current = add(state, crossLength, 'E',current.x,current.y,start);
    current = add(state, crossLength, 'N',current.x,current.y,start);
    current = add(state, crossLength, 'O',current.x,current.y,start);
    current = add(state, crossLength, 'N',current.x,current.y,start);
    current = add(state, crossLength, 'O',current.x,current.y,start);
    current = add(state, crossLength, 'S',current.x,current.y,start);
    current = add(state, crossLength, 'O',current.x,current.y,start);
    current = add(state, crossLength, 'S',current.x,current.y,start);

    lookAvailablePoint(state,crossLength);
		return state;
	}
  /*
  * For each point on board, scan the 8 lines
  */
  void lookAvailablePoint(MorpioStatePtr state, size_t num)
  {
    std::map<Point, std::vector<bool> >::iterator it;
    bool add = false;
    for(it = state->board.begin(); it != state->board.end(); it++)
      {
        Point tmp(0,0);
        tmp.x = it->first.x;
        tmp.y = it->first.y;
        
        lookDir(state,tmp,0,1,num,'N', 4); //N
        lookDir(state,tmp,1,1,num,'M',5); // NE
        lookDir(state,tmp,1,0,num,'E',6); // E
        lookDir(state,tmp,1,-1,num,'D',7); // SE
        lookDir(state,tmp,0,-1,num,'S',0); // S
        lookDir(state,tmp,-1,-1,num,'S',1); // SW
        lookDir(state,tmp,-1,0,num,'S',2); // W
        lookDir(state,tmp,-1,1,num,'B',3); // NW
      }
  }
  
  void lookDir(MorpioStatePtr state, Point p, size_t modifX, size_t modifY,size_t num, char dir, size_t oppositeDirection)
  {
        bool add = true;
        Point tmp(0,0);
        for(size_t k=0;k<num-2;++k)
        {
            tmp.x=p.x+k*modifX;
            tmp.y=p.y+k*modifY;
            //TODO make if works
            // if point not there or point incoming already used (here south), 
            if(!(state->board.find(tmp)) || (state->board.find(tmp) && state->board.find(tmp)->second[oppositeDirection]==false) )//!= std::map<Point, std::vector<bool> >::end
                {add=false;}
        
        }
        // make sure there is NOT already a point there
        tmp.x=p.x+(num-1)*modifX;
        tmp.y=p.y+(num-1)*modifY;
        if((state->board.find(tmp)))
            add=false;

        if(add)
         state->actionsBoard.insert(std::make_pair<Point,char>(tmp,dir));
    
    }
 /*
  * Only use at the start to place the inital cross
  */
  Point add(MorpioStatePtr state, size_t num, char direction, int posX, int posY, std::vector<bool> start)
  {
    Point last(0,0);
    for(size_t i=0;i<num-1;++i)
    {
        // TODO use mask instead
        //TODO make p works
        std::vector<bool> dummy;
        dummy = start;
        switch(direction)
        {
        case 'E': 
            Point p(posX+i,posY);
            state->board.insert(std::make_pair(p,dummy)); 
            last.x=p.x;
            last.y=p.y;
            break;
        case 'S':
            Point p(posX,posY-i);
            state->board.insert(std::make_pair(pp,dummy));
            last.x=p.x;
            last.y=p.y;
            break;
        case 'O':  
            Point p(posX-i,posY);
            state->board.insert(std::make_pair(p,dummy)); 
            last.x=p.x;
            last.y=p.y;
            break;
        case 'N':  
            Point p(posX,posY+i);
            state->board.insert(std::make_pair(p,dummy)); 
            last.x=p.x;
            last.y=p.y;
            break;
        default: break;

        }
    }
    return last;
  }

protected:
	friend class MorpioProblemClass;

	size_t crossLength;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_SUDOKU_H_
