/*-----------------------------------------.---------------------------------.
| Filename: BanditsMCTS.h                  | Bandits Sand Box                |
| Author  : David LS                       |                                 |
| Started : 24/04/2011 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDITS_MCTS_WORK_UNIT_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDITS_MCTS_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "../WorkUnit/GPSandBox.h"
# include "DiscreteBanditExperiment.h"
# include "../Problem/DamienDecisionProblem.h"

namespace lbcpp
{

class MCTSExpressionBuilderState : public GPExpressionBuilderState
{
public:
  MCTSExpressionBuilderState(const String& name, EnumerationPtr inputVariables, FunctionPtr objectiveFunction)
  : GPExpressionBuilderState(name, inputVariables, objectiveFunction)
  {
    areVariableUsed.resize(inputVariables->getNumElements(), false);

  }
  MCTSExpressionBuilderState() {}

  virtual String toShortString() const
  {
    if (expressions.empty())
      return T("<initial>");
    // return description;
    GPExpressionPtr expr = expressions.back();
    return expr->toShortString();// + T(" (") + String((int)expressions.size()) + T(" steps)");
  }

  virtual ContainerPtr getAvailableActions() const
  {
    //TODO also set rule from length max of formula
    ObjectVectorPtr res = new ObjectVector(gpExpressionBuilderActionClass);
    size_t numExpressions = expressions.size();

    // variables; Can be add multiple times
    if(numExpressions == 0)
      addVariables(res); // only add set of variables
    else if(numExpressions == 1)
    {
      addVariables(res);
      addUnary(res,numExpressions);
    }
    else // numExp >=2
    {
      addVariables(res);
      addUnary(res,numExpressions);
      addBinary(res,numExpressions);
    }
    return res;
  }

  virtual void addVariables(const ObjectVectorPtr& res) const
  {
    for (size_t i = 0; i < areVariableUsed.size(); ++i)
      res->append(new VariableGPExpressionBuilderAction(Variable(i, inputVariables)));
  }

  virtual void addUnary(const ObjectVectorPtr& res, size_t numExpressions) const
  {
    // unary expressions
    size_t n = gpPreEnumeration->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      res->append(new UnaryGPExpressionBuilderAction((GPPre)i, numExpressions - 1));
    }
  }

  virtual void addBinary(const ObjectVectorPtr& res, size_t numExpressions ) const
  {
    // binary expressions
    size_t n = gpOperatorEnumeration->getNumElements();
    size_t j = numExpressions - 1;
    size_t k = numExpressions - 2;
    for (size_t i = 0; i < n; ++i)
    {
      res->append(new BinaryGPExpressionBuilderAction((GPOperator)i, j, k));
    }
  }

  GPExpressionPtr expression;
  double score;

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    const GPExpressionBuilderActionPtr& builderAction = action.getObjectAndCast<GPExpressionBuilderAction>();
    GPExpressionPtr expression = builderAction->makeExpression(expressions);

    // follow RNP rule
    UnaryGPExpressionPtr unaryExpression = expression.dynamicCast<UnaryGPExpression>();
    if (unaryExpression)
      expressions.pop_back();
    BinaryGPExpressionPtr binaryExpression = expression.dynamicCast<BinaryGPExpression>();
    if (binaryExpression)
    { expressions.pop_back(); expressions.pop_back();}

    expressions.push_back(expression);

    double previousScore = expressionScores.size() ? expressionScores.back().toDouble() : objectiveFunction->compute(context, new ConstantGPExpression(0.0)).toDouble();

    Variable score = objectiveFunction->compute(context, expression);
    expressionScores.push_back(score);
    reward = previousScore - score.toDouble(); // score must be minimized, reward must be maximized

    if (description.isNotEmpty())
      description += T(" -> ");
    description += action.toShortString();

    if (stateBackup)
      *stateBackup = action; // knowing the action is enough to undo the transition
  }
  virtual bool isFinalState() const
  {return false;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<MCTSExpressionBuilderState>& target = t.staticCast<MCTSExpressionBuilderState>();
    target->name = name;
    target->inputVariables = inputVariables;
    target->objectiveFunction = objectiveFunction;
    target->areVariableUsed = areVariableUsed;
    target->expressions.resize(expressions.size());
    for (size_t i = 0; i < expressions.size(); ++i)
      target->expressions[i] = expressions[i]->cloneAndCast<GPExpression>(context);
    target->expressionScores = expressionScores;
    target->description = description;
  }

  virtual double getScore() const
  {return expressionScores.size() ? expressionScores.back().toDouble() : DBL_MAX;}

  virtual GPExpressionPtr getExpression() const
  {return expressions.size() ? expressions.back() : GPExpressionPtr();}

  virtual size_t getNumVariables() const
  {return expressions.size();}


protected:
  friend class MCTSExpressionBuilderStateClass;
  std::vector<GPExpressionPtr> expressions;
  std::vector<bool> areVariableUsed;

  std::vector<Variable> expressionScores;
  String description;
};

typedef ReferenceCountedObjectPtr<MCTSExpressionBuilderState> MCTSExpressionBuilderStatePtr;




class MCTSNode;
typedef ReferenceCountedObjectPtr<MCTSNode> MCTSNodePtr;

class MCTSNode : public Object
{
public:
  MCTSNode(DecisionProblemStatePtr state)
  : reward(0), parent(0), visit(0), isLeaf(true), depth(0), state(state), actionIndex(0) {}

  //TODO remove this
  void setState(DecisionProblemStatePtr s)
  {state = s;}

  void setAction(size_t index, const Variable& a)
  {
    actionIndex = index;
    action = a;
  }

  const DecisionProblemStatePtr& getState() const
  {return state;}

  String toString(String tab = T("")) const
  {
    String res = tab + String(pos) + T(" - Visit: ") + String(visit) + T(" - Reward: ") + String(reward) + T("\n");
    for (size_t i = 0; i < child.size(); ++i)
      res += child[i]->toString(tab + T("  "));
    return res;
  }


  //TODO implement as a singly linked list
  std::vector<MCTSNodePtr> child;       // list of children
  double reward;            // value accumulated so far
  int parent;              // position of the parent
  int visit;               // visit count
  bool isLeaf;			 // is a leaf node
  int depth;           // size of the legal action set
  DecisionProblemStatePtr state;

  size_t actionIndex;
  Variable action; // the action that lead here

  int pos;
};

class MCTS
{
public:
  MCTS(ExecutionContext& context, DecisionProblemStatePtr initialState, size_t maxDepth)
  : context(context), maxDepth(maxDepth), isFound(false)
  {
    root = new MCTSNode(initialState);
    tree.push_back(root);

    createAllChild(root);



    myfile.open ("treeXP.txt",ios::app);



  }

  void doOneIteration(size_t iterationNumber, bool verbose = false)
  {
    if (verbose)
      context.enterScope("Iteration" + String((int)iterationNumber));

    MCTSNodePtr cur = select(root);
    MCTSNodePtr p = expand(cur);
    double f=play(p);
    backpropagate(p->pos, f, p);

    if (verbose)
    {
      context.resultCallback("Iteration number", iterationNumber);
      context.resultCallback("Selected state", cur->getState());
      context.resultCallback("Reward", f);
      context.resultCallback("Expanded state", p->getState());

      context.leaveScope();
    }

  }

  /**
   * Sets the children of the root node
   * Called only once.
   */
  void createAllChild(MCTSNodePtr node)
  {
    ContainerPtr availableActions = node->getState()->getAvailableActions();
    size_t n = availableActions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = availableActions->getElement(i);
      DecisionProblemStatePtr newState = node->getState()->cloneAndCast<DecisionProblemState>();
      double reward;
      newState->performTransition(context, action, reward);

      MCTSNodePtr childNode = new MCTSNode(newState);
      childNode->setAction(i, action);
      childNode->pos = tree.size();
      tree.push_back(childNode);
      node->child.push_back(childNode);
    }
    node->isLeaf = false;
  }

  /**
   * Select the best node to expand ; BFR
   * Note: we have a copy of current.
   * No modif possible in the selection phase
   */
  MCTSNodePtr select(MCTSNodePtr cur)
  {
    std::vector<MCTSNodePtr> list;
    double bestScore=-DBL_MAX;
    double score;
    double c=sqrt(2.0);

    String s="";

    while (!cur->isLeaf)
    {
      bestScore = -DBL_MAX;
      for (size_t i = 0; i < cur->child.size(); ++i)
      {
        MCTSNodePtr tmp = cur->child[i];
        score = (tmp->visit>0) ? tmp->reward/tmp->visit + c*sqrt(log(1.0+tree[tmp->parent]->visit)/(1+tmp->visit))
        : DBL_MAX;

        if (score >= bestScore)
        {
          if (score > bestScore)
          {
            list.clear();
            bestScore = score;
          }
          list.push_back(tmp);
        }
      }
      // get the selected node
      cur = list[context.getRandomGenerator()->sampleSize(list.size())];

      s+=cur->getState()->toShortString();
      s+=" -> ";

    }
    s+="\n";
  //  myfile<<s;
    return cur;
  }

  /**
   * This method expand the selected node
   */
  MCTSNodePtr expand(MCTSNodePtr select)
  {
    // select a random action
    ContainerPtr availableActions = select->getState()->getAvailableActions();
    size_t n = availableActions->getNumElements();
    size_t actionIndex = context.getRandomGenerator()->sampleSize(n);
    Variable action = availableActions->getElement(actionIndex);

    // clone state and apply system dynamics
    DecisionProblemStatePtr newState = select->getState()->cloneAndCast<DecisionProblemState>();
    double reward;
    newState->performTransition(context, action, reward);

    // look if it exists already
    //TODO change when singly linked list ////////////
    for (size_t i = 0; i <select->child.size(); i++)
      if (select->child[i]->actionIndex == actionIndex)
        return select->child[i];

    // it did not exist ; set values
    MCTSNodePtr elem = new MCTSNode(newState);
    elem->setAction(actionIndex, action);
    elem->parent = select->pos;
    elem->pos = tree.size();
    elem->depth = select->depth+1;
    // add
    tree.push_back(elem);
    // increase children
    select->child.push_back(elem);

    ///////////// end /////////////

    // if generated every child, isLeaf=false
    if (select->child.size() == n)
      {select->isLeaf = false;}

    return elem;
  }


  /**
   * execute a "random" simulation
   */
  double play(MCTSNodePtr p)
  {
    //TODO THIS is only for a fixed length
    /*
    // depth
    // GPExpressionBuilderStatePtr state =p->getState().staticCast<GPExpressionBuilderState>();
    GPExpressionBuilderStatePtr clone =  p->getState()->cloneAndCast<GPExpressionBuilderState>();
    for(size_t i = p->depth ; i< maxDepth ; ++i)
    {
      // ajoute un action
      ContainerPtr availableActions = clone->getAvailableActions();
      size_t n = availableActions->getNumElements();
      size_t actionIndex = context.getRandomGenerator()->sampleSize(n);
      Variable action = availableActions->getElement(actionIndex);
      double reward;

      clone->performTransition(context, action, reward);

    }
    if(clone->getExpression()->size()>maxDepth)
      return -1.0;

*/




    //TODO This is for any length
    size_t depth = p->depth;
    MCTSExpressionBuilderStatePtr clone =  p->getState()->cloneAndCast<MCTSExpressionBuilderState>();

   // myfile << clone->getExpression()->toShortString() << " in the end " <<endl;

    while(depth < maxDepth && clone->getNumVariables()>1)
    {
      // ajoute un action
            ContainerPtr availableActions = clone->getAvailableActions();
            size_t n = availableActions->getNumElements();
            size_t actionIndex = context.getRandomGenerator()->sampleSize(n);
            Variable action = availableActions->getElement(actionIndex);
            double reward;

            clone->performTransition(context, action, reward);
            ++depth;
    }


if(clone->getNumVariables()>1)
  return -1.0;




    double score = clone->getScore();


    //  cout<<" score play ----  "<< score << endl;


    if(score==0) // we got a winner
    {
      isFound=true;
      bestOjectiveFunctionFound = clone;
    }
    //TODO it should not happen but it does!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if(score!=score)
    { cout << " i came here --------------------------- " << endl;return -1.0;}

    return -score;
  }
  /**
   * send the result back
   * Only works for 1 player game
   */
  void backpropagate(int pos, double rew, MCTSNodePtr select)
  {
    while (pos!=0)
    {
      select->reward += rew;
      select->visit++;
      pos = select->parent;
      select = tree[pos];
    }
  }

  /**
   * If the solution was not yet found
   * return the best we got
   * Called once at the end
   */
  void traverse(MCTSNodePtr cur)
  {

    // traverse
    while(!cur->isLeaf)
    {
      MCTSNodePtr best;
      for (size_t i = 0; i < cur->child.size(); ++i)
      {
        MCTSNodePtr tmp = cur->child[i];
        cout << " isLeaf " << tmp->isLeaf << " visit " << tmp->visit << endl;

        //if (tmp->visit > best) // FIXME: this is not correct, comparing an int with a MCTSNodePtr
        //  best=tmp;
      }
      cur=best;
    }
    bestOjectiveFunctionFound = cur->getState();
    //TODO must think of a solution here
    // if length != depthMax --
    myfile.close();
  }

  ExecutionContext& context;
  MCTSNodePtr root;
  std::vector<MCTSNodePtr> tree;
  size_t maxDepth;
  GPExpressionBuilderStatePtr bestOjectiveFunctionFound;
  bool isFound;


  ofstream myfile;

};



/*
This is the run method
 */
class BanditsMCTSWorkUnit : public WorkUnit
{
public:
  BanditsMCTSWorkUnit() : numIterations(1000), maxDepth(5) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope("My Scope");

    myfile.open ("fullTreeD3.txt",ios::app);

    DecisionProblemStatePtr initialState = createInitialState(context);
    String s="";

    MCTSExpressionBuilderStatePtr mc = ce(context);
  //  recursiveExhaustiveSearch(context, mc, 0,5,s);

       MCTS mcts(context, initialState, maxDepth);
    for (size_t i = 0; i < numIterations; ++i)
    {
      if(!mcts.isFound){
        mcts.doOneIteration(i, true);
        context.progressCallback(new ProgressionState(i + 1, numIterations, "iterations"));
      }
    }


    if(!mcts.isFound)
    {  // best we could do
      mcts.traverse(mcts.root);
    }
    //TODO marche pas
    context.resultCallback(T("expression"), mcts.bestOjectiveFunctionFound->getExpression());
    cout << mcts.bestOjectiveFunctionFound->getExpression()->toShortString() << " in the end " <<endl;
    cout << -mcts.bestOjectiveFunctionFound->getScore() << " score " << endl;
    context.leaveScope();



    myfile.close();


    return true;
  }


  MCTSExpressionBuilderStatePtr ce(ExecutionContext& context){
    DefaultEnumerationPtr inputVariables = new DefaultEnumeration(T("variables"));
    inputVariables->addElement(context, T("x[0]"));
    inputVariables->addElement(context, T("x[1]"));
    inputVariables->addElement(context, T("x[2]"));
    inputVariables->addElement(context, T("x[3]"));

    FunctionPtr objective = createObjectiveFunction(context);

    // TODO tmp
    // breadthFirstSearch(context, inputVariables, objective);

    return new MCTSExpressionBuilderState("hello", inputVariables, objective);
  }

  /**
   * Give the initial state to pass to MCTS
   */
  DecisionProblemStatePtr createInitialState(ExecutionContext& context) const
  {
    DefaultEnumerationPtr inputVariables = new DefaultEnumeration(T("variables"));
    inputVariables->addElement(context, T("x[0]"));
    inputVariables->addElement(context, T("x[1]"));
    inputVariables->addElement(context, T("x[2]"));
    inputVariables->addElement(context, T("x[3]"));

    FunctionPtr objective = createObjectiveFunction(context);
    if (!objective->initialize(context, gpExpressionClass))
      return DecisionProblemStatePtr();

    // TODO tmp
    // breadthFirstSearch(context, inputVariables, objective);

    return new MCTSExpressionBuilderState("hello", inputVariables, objective);
  }

  /**
   * create the Delta function : Formula -> Real number
   * invent formulae here
   */
  FunctionPtr createObjectiveFunction(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    std::vector<std::pair<std::vector<double> , double> > examples;
    for (size_t i = 0; i < 1000; ++i)
    {
      std::vector<double> x(4);

      x[0] = random->sampleDouble(1.0, 6.0);
      x[1] = random->sampleDouble(1.0, 6.0);
      x[2] = random->sampleDouble(1.0, 6.0);
      x[3] = random->sampleDouble(1.0, 6.0);
      double y = log(x[0]) + sqrt(x[1] * x[2])*x[0];// + x[2] + x[3];// 10.0 / (5.0 + (x[0] - 2) * (x[0] - 2) + (x[1] - 2) * (x[1] - 2));
      //      double y = 1.0 + x[0] + x[1];//cos(x[0] * (1 + cos(x[0])));
      examples.push_back(std::make_pair(x, y));
    }

    double lambda = 0.0;
    return new GPObjectiveFunction(examples, lambda);
  }

private:
  friend class BanditsMCTSWorkUnitClass;

  size_t numIterations;
  size_t maxDepth;


  void recursiveExhaustiveSearch(ExecutionContext& context, MCTSExpressionBuilderStatePtr state,
      size_t depth, size_t maxDepth, String s)
  {
    ContainerPtr actions = state->getAvailableActions();

    size_t n = actions->getNumElements();

    //   cout<< " depth "<< depth << " size of actions "<< actions->getNumElements()<< endl;


    for (size_t i = 0; i < n; ++i)
    {
      Variable action = actions->getElement(i);
      String s2 = s;
      s2+=" -> ";

      double reward;
      //context.enterScope(action.toShortString());
      MCTSExpressionBuilderStatePtr newState = state->cloneAndCast<MCTSExpressionBuilderState>();
      newState->performTransition(context, action, reward);
      s2+=newState->toShortString();

      if (depth < maxDepth - 1)
        recursiveExhaustiveSearch(context, newState, depth + 1, maxDepth,s2);
      else
      {
        //    cout << newState->getExpression()->toShortString()<< endl; cout<< endl;
        s2+="     and the result is ";
        s2+= newState->getExpression()->toShortString();
        s2+="\n";
        myfile << s2;

        //  cout<< newState->getExpression()->size() << " size "<<endl;

      }

      //context.leaveScope(true);
    }
  }


  bool breadthFirstSearch(ExecutionContext& context, EnumerationPtr inputVariables, const FunctionPtr& objective) const
  {
    size_t maxSearchNodes = 100000;

    DecisionProblemPtr problem = new GPExpressionBuilderProblem(inputVariables, objective);
    DecisionProblemStatePtr state = new MCTSExpressionBuilderState(T("toto"), inputVariables, objective);

    for (size_t depth = 0; depth < 1; ++depth)
    {
      context.enterScope(T("Depth ") + String((int)depth + 1));

      SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);

      SearchPolicyPtr searchPolicy = bestFirstSearchPolicy(new MinDepthSearchHeuristic());

      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);

      context.resultCallback(T("bestReturn"), searchTree->getBestReturn());
      bool isFinished = (searchTree->getBestReturn() <= 0.0);
      if (!isFinished)
      {
        context.resultCallback(T("bestAction"), searchTree->getBestAction());
        context.resultCallback(T("bestTrajectory"), searchTree->getBestNodeTrajectory());

        double transitionReward;
        state->performTransition(context, searchTree->getBestAction(), transitionReward);
        context.resultCallback(T("newState"), state->clone(context));
        context.resultCallback(T("transitionReward"), transitionReward);

        GPExpressionBuilderStatePtr expressionBuilderState = state.dynamicCast<MCTSExpressionBuilderState>();
        jassert(expressionBuilderState);
        context.resultCallback(T("expression"), expressionBuilderState->getExpression());
        context.resultCallback(T("score"), expressionBuilderState->getScore());
        context.informationCallback(expressionBuilderState->getExpression()->toShortString());

        context.leaveScope(expressionBuilderState->getScore());
      }
      else
      {
        context.leaveScope(state);
        break;
      }
    }
    return true;
  }

  ofstream myfile;


};

}; /* namespace lbcpp */

#endif // LBCPP_SEQUENTIAL_DECISION_BANDITS_MCTS_WORK_UNIT_H_

