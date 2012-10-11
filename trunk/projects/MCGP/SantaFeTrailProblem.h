/*-----------------------------------------.---------------------------------.
| Filename: SantaFeTrailProblem.h          | Sante Fe Trail Problem          |
| Author  : Francis Maes                   |                                 |
| Started : 11/10/2012 12:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SANTA_FE_TRAIL_PROBLEM_H_
# define LBCPP_MCGP_SANTA_FE_TRAIL_PROBLEM_H_

# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

class SantaFeTrailWorld : public Object
{
public:
  SantaFeTrailWorld() : numPellets(0)
  {
    static const char* s = 
      ".###............................"
      "...#............................"
      "...#.....................###...."
      "...#....................#....#.."
      "...#....................#....#.."
      "...####.#####........##........."
      "............#................#.."
      "............#.......#..........."
      "............#.......#........#.."
      "............#.......#..........."
      "....................#..........."
      "............#................#.."
      "............#..................."
      "............#.......#.....###..."
      "............#.......#..#........"
      ".................#.............."
      "................................"
      "............#...........#......."
      "............#...#..........#...."
      "............#...#..............."
      "............#...#..............."
      "............#...#.........#....."
      "............#..........#........"
      "............#..................."
      "...##..#####....#..............."
      ".#..............#..............."
      ".#..............#..............."
      ".#......#######................."
      ".#.....#........................"
      ".......#........................"
      "..####.........................."
      "................................";
    static const size_t width = 32;
    static const size_t height = 32;
    jassert(strlen(s) == width * height);

    state.resize(width, std::vector<char>(height));
    const char* ptr = s;
    for (size_t j = 0; j < height; ++j)
      for (size_t i = 0; i < width; ++i)
      {
        char st = (*ptr++ == '#' ? pelletState : emptyState);
        state[i][j] = st;
        if (st)
          ++numPellets;
      }
  }
  
  size_t getInitialNumPellets() const
    {return numPellets;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    const ReferenceCountedObjectPtr<SantaFeTrailWorld>& t = target.staticCast<SantaFeTrailWorld>();
    t->state = state;
    t->numPellets = numPellets;
  }

  bool hasPellet(size_t i, size_t j) const
    {return state[i][j] == pelletState;}

  void clear(size_t i, size_t j)
    {state[i][j] = emptyState;}

  size_t getWidth() const
    {return state.size();}

  size_t getHeight() const
    {return state[0].size();}

private:
  enum
  {
    emptyState = 0,
    pelletState,
  };

  std::vector< std::vector<char> > state;
  size_t numPellets;
};

typedef ReferenceCountedObjectPtr<SantaFeTrailWorld> SantaFeTrailWorldPtr;

class SantaFeTrailState : public Object
{
public:
  SantaFeTrailState(SantaFeTrailWorldPtr world, size_t maxNumSteps)
    : world(world->cloneAndCast<SantaFeTrailWorld>()), maxNumSteps(maxNumSteps), numSteps(0), numEatenPellets(0)
  {
    position.x = position.y = 0;
    direction = E;
    jassert(!world->hasPellet(position.x, position.y));
  }
  SantaFeTrailState() {}

  bool isFoodAhead() const
  {
    Position p = getNextPosition();
    return world->hasPellet(p.x, p.y);
  }

  void move()
  {
    jassert(!isTimeExhausted());
    position = getNextPosition();
    ++numSteps;
    if (world->hasPellet(position.x, position.y))
    {
      ++numEatenPellets;
      world->clear(position.x, position.y);
    }
  }

  void left()
  {
    jassert(!isTimeExhausted());
    direction = (Direction)(direction == N ? W : direction - 1);
    ++numSteps;
  }

  void right()
  {
    jassert(!isTimeExhausted());
    direction = (Direction)(direction == W ? N : direction + 1);
    ++numSteps;
  }

  size_t getNumEatenPellets() const
    {return numEatenPellets;}

  bool isTimeExhausted() const
    {return numSteps >= maxNumSteps;}

  bool areAllPelletsEaten() const
    {return getNumEatenPellets() == world->getInitialNumPellets();}

private:
  friend class SantaFeTrailStateClass;

  SantaFeTrailWorldPtr world;
  size_t maxNumSteps;

  size_t numSteps;
  size_t numEatenPellets;

  struct Position {size_t x, y;};
  Position position;

  enum Direction {N, E, S, W};
  Direction direction;

  Position getNextPosition() const
  {
    Position res = position;
    if (direction == N)
      decrement(res.y, world->getHeight());
    else if (direction == E)
      increment(res.x, world->getWidth());
    else if (direction == S)
      increment(res.y, world->getHeight());
    else if (direction == W)
      decrement(res.x, world->getWidth());
    else
      jassertfalse;
    return res;
  }

  static void increment(size_t& position, size_t size)
    {position = (position == size - 1 ? 0 : position + 1);}

  static void decrement(size_t& position, size_t size)
    {position = (position == 0 ? size - 1 : position - 1);}
};

typedef ReferenceCountedObjectPtr<SantaFeTrailState> SantaFeTrailStatePtr;

/*
** Actions
*/
class SantaFeTrailAction : public Object
{
public:
  virtual void execute(SantaFeTrailStatePtr state) = 0;
};

extern ClassPtr santaFeTrailActionClass;
typedef ReferenceCountedObjectPtr<SantaFeTrailAction> SantaFeTrailActionPtr;

class MoveSantaFeTrailAction : public SantaFeTrailAction
{
public:
  virtual String toShortString() const
    {return "move";}

  virtual void execute(SantaFeTrailStatePtr state)
    {state->move();}
};

class LeftSantaFeTrailAction : public SantaFeTrailAction
{
public:
  virtual String toShortString() const
    {return "left";}

  virtual void execute(SantaFeTrailStatePtr state)
    {state->left();}
};

class RightSantaFeTrailAction : public SantaFeTrailAction
{
public:
  virtual String toShortString() const
    {return "right";}

  virtual void execute(SantaFeTrailStatePtr state)
    {state->right();}
};

class IfFoodAheadSantaFeTrailAction : public SantaFeTrailAction
{
public:
  IfFoodAheadSantaFeTrailAction(SantaFeTrailActionPtr thenAction, SantaFeTrailActionPtr elseAction)
    : thenAction(thenAction), elseAction(elseAction) {}
  IfFoodAheadSantaFeTrailAction() {}

  virtual void execute(SantaFeTrailStatePtr state)
  {
    if (state->isFoodAhead())
      thenAction->execute(state);
    else
      elseAction->execute(state);
  }

protected:
  friend class IfFoodAheadSantaFeTrailActionClass;

  SantaFeTrailActionPtr thenAction;
  SantaFeTrailActionPtr elseAction;
};

class SequenceSantaFeTrailAction : public SantaFeTrailAction
{
public:
  SequenceSantaFeTrailAction(const std::vector<SantaFeTrailActionPtr>& actions)
    : actions(actions) {}
  SequenceSantaFeTrailAction() {}
  
  virtual void execute(SantaFeTrailStatePtr state)
  {
    for (size_t i = 0; i < actions.size() && !state->isTimeExhausted(); ++i)
      actions[i]->execute(state);
  }
  
protected:
  friend class SequenceSantaFeTrailActionClass;

  std::vector<SantaFeTrailActionPtr> actions;
};

/*
** Functions
*/
class SantaFeTrailFunction : public Function
{
public:
  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
	{return type->inheritsFrom(santaFeTrailActionClass);}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return santaFeTrailActionClass;}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
  {
    String res = toShortString() + "(";
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      res += inputs[0]->toShortString();
      if (i < inputs.size() - 1)
        res += ", ";
    }
    res += ")";
    return res;
  }
};

class Progn2SantaFeTrailFunction : public SantaFeTrailFunction
{
public:
  virtual size_t getNumInputs() const
    {return 2;}

  virtual String toShortString() const
    {return "progn2";}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    std::vector<SantaFeTrailActionPtr> actions(2);
    actions[0] = inputs[0].getObjectAndCast<SantaFeTrailAction>();
    actions[1] = inputs[1].getObjectAndCast<SantaFeTrailAction>();
    return new SequenceSantaFeTrailAction(actions);
  }
};

class Progn3SantaFeTrailFunction : public SantaFeTrailFunction
{
public:
  virtual size_t getNumInputs() const
    {return 3;}

  virtual String toShortString() const
    {return "progn3";}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    std::vector<SantaFeTrailActionPtr> actions(3);
    actions[0] = inputs[0].getObjectAndCast<SantaFeTrailAction>();
    actions[1] = inputs[1].getObjectAndCast<SantaFeTrailAction>();
    actions[2] = inputs[2].getObjectAndCast<SantaFeTrailAction>();
    return new SequenceSantaFeTrailAction(actions);
  }
};

class IfFoodAheadSantaFeTrailFunction : public SantaFeTrailFunction
{
public:
  virtual size_t getNumInputs() const
    {return 2;}

  virtual String toShortString() const
    {return "if-food-ahead";}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return new IfFoodAheadSantaFeTrailAction(inputs[0].getObjectAndCast<SantaFeTrailAction>(), inputs[1].getObjectAndCast<SantaFeTrailAction>());}
};

/*
** Problem
*/
class SantaFeTrailProblem : public ExpressionProblem
{
public:
  SantaFeTrailProblem(size_t maxNumSteps = 400) : maxNumSteps(maxNumSteps)
    {initialize();}

  virtual void initialize()
  {
    // define domain
    domain->addConstant(Variable(new MoveSantaFeTrailAction(), santaFeTrailActionClass));
    domain->addConstant(Variable(new LeftSantaFeTrailAction(), santaFeTrailActionClass));
    domain->addConstant(Variable(new RightSantaFeTrailAction(), santaFeTrailActionClass));

    domain->addFunction(new Progn2SantaFeTrailFunction());
    domain->addFunction(new Progn3SantaFeTrailFunction());
    domain->addFunction(new IfFoodAheadSantaFeTrailFunction());

    domain->addTargetType(santaFeTrailActionClass);

    // create world
    world = new SantaFeTrailWorld();

    // fitness limits
    limits->setLimits(0, 0, world->getInitialNumPellets()); // the aim: maximize the number of eaten pellets
  }
  
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
#if 0
	  std::vector<SantaFeTrailActionPtr> elseActions;
	  elseActions.push_back(new RightSantaFeTrailAction());
	  elseActions.push_back(new RightSantaFeTrailAction());
	  elseActions.push_back(new IfFoodAheadSantaFeTrailAction(new MoveSantaFeTrailAction(), new RightSantaFeTrailAction()));
	  std::vector<SantaFeTrailActionPtr> actions;
	  actions.push_back(new MoveSantaFeTrailAction());
	  actions.push_back(new RightSantaFeTrailAction());
	  actions.push_back(new IfFoodAheadSantaFeTrailAction(new MoveSantaFeTrailAction(), new SequenceSantaFeTrailAction(elseActions)));
	  SantaFeTrailActionPtr testAction = new SequenceSantaFeTrailAction(actions);

	  SantaFeTrailStatePtr testState = new SantaFeTrailState(world, maxNumSteps);
      while (!testState->isTimeExhausted() && !testState->areAllPelletsEaten())
        testAction->execute(testState);
	  std::cout << "PPPFPFPFP: " << testState->getNumEatenPellets() << std::endl;
#endif // 0

    // retrieve expression and compute it
    ExpressionPtr expression = object.staticCast<Expression>();
    Variable dummy;
    SantaFeTrailActionPtr action = expression->compute(context, &dummy).getObjectAndCast<SantaFeTrailAction>();

    // initialize state and execute action
    SantaFeTrailStatePtr state = new SantaFeTrailState(world, maxNumSteps);
    while (!state->isTimeExhausted() && !state->areAllPelletsEaten())
      action->execute(state);
    
    // construct fitness
    std::vector<double> fitness(1);
    fitness[0] = (double)state->getNumEatenPellets();
    return new Fitness(fitness, limits);
  }

protected:
  friend class SantaFeTrailProblemClass;

  size_t maxNumSteps;

  SantaFeTrailWorldPtr world;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SANTA_FE_TRAIL_PROBLEM_H_
