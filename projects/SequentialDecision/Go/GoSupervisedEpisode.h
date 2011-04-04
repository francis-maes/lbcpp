/*-----------------------------------------.---------------------------------.
| Filename: GoSupervisedEpisode.h          | Go Supervised Episode Function  |
| Author  : Francis Maes                   |                                 |
| Started : 25/03/2011 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GO_SUPERVISED_EPISODE_FUNCTION_H_
# define LBCPP_SEQUENTIAL_DECISION_GO_SUPERVISED_EPISODE_FUNCTION_H_

# include "GoProblem.h"

namespace lbcpp
{

// State, Action -> Action
class SupervisedLinearRankingBasedDecisionMaker : public CompositeFunction
{
public:
  SupervisedLinearRankingBasedDecisionMaker(FunctionPtr actionsPerceptionFunction, StochasticGDParametersPtr sgdParameters)
    : actionsPerceptionFunction(actionsPerceptionFunction), sgdParameters(sgdParameters) {}
  SupervisedLinearRankingBasedDecisionMaker() {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)decisionProblemStateClass;}

  virtual String getOutputPostFix() const
    {return T("Action");}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(decisionProblemStateClass, T("state"));
    size_t supervision = builder.addInput(variableType, T("supervision"));

    actionsType = positiveIntegerPairClass; // FIXME: dependency on the type of actions
    size_t availableActions = builder.addFunction(getAvailableActionsFunction(actionsType), state);

    size_t actionPerceptions = builder.addFunction(actionsPerceptionFunction, state, availableActions, T("perceptions"));
    size_t actionCosts = builder.addFunction(lbcppMemberBinaryFunction(SupervisedLinearRankingBasedDecisionMaker, computeRankingCosts,
                                                                        containerClass(), variableType, denseDoubleVectorClass(positiveIntegerEnumerationEnumeration)),
                                             availableActions, supervision, T("costs"));
    
    if (!rankingMachine)
      rankingMachine = linearRankingMachine(new StochasticGDParameters(sgdParameters->getLearningRate(), StoppingCriterionPtr(), 0,
                                                                               sgdParameters->doPerEpisodeUpdates(), sgdParameters->doNormalizeLearningRate(),
                                                                               false, true, false));
    size_t actionScores = builder.addFunction(rankingMachine, actionPerceptions, actionCosts, T("scores"));
    

    builder.addFunction(lbcppMemberBinaryFunction(SupervisedLinearRankingBasedDecisionMaker, getPredictedAction,
                                containerClass(), denseDoubleVectorClass(), actionsType), availableActions, actionScores);
  }

  const FunctionPtr& getRankingMachine() const
    {return rankingMachine;}

protected:
  friend class SupervisedLinearRankingBasedDecisionMakerClass;

  FunctionPtr actionsPerceptionFunction; // State, Container[Action] -> Container[Perceptions] 
  StochasticGDParametersPtr sgdParameters;
  FunctionPtr rankingMachine;

  TypePtr actionsType;

  Variable getPredictedAction(ExecutionContext& context, const Variable& availableActions, const Variable& predictedScores) const
  {
    const ContainerPtr& actions = availableActions.getObjectAndCast<Container>();
    const DenseDoubleVectorPtr& scores = predictedScores.getObjectAndCast<DenseDoubleVector>();
    if (!scores)
      return Variable::missingValue(actionsType);
    size_t n = actions->getNumElements();
    jassert(n == scores->getNumElements());
    int best = scores->getIndexOfMaximumValue();
    if (best < 0)
      return Variable::missingValue(actionsType);
    return actions->getElement(best);
  }

  Variable computeRankingCosts(ExecutionContext& context, const Variable& availableActions, const Variable& correctAction) const
  {
    if (!correctAction.exists())
      return Variable::missingValue(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration));
    const ContainerPtr& actions = availableActions.getObjectAndCast<Container>();
    size_t n = actions->getNumElements();

    DenseDoubleVectorPtr res(new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, n, 0.0));
    bool actionFound = false;
    for (size_t i = 0; i < n; ++i)
      if (actions->getElement(i) == correctAction)
      {
        res->setValue(i, -1);
        actionFound = true;
        break;
      }

    if (!actionFound)
      context.warningCallback(T("Could not find action ") + correctAction.toShortString());
    return res;
  }
};

// InitialState, Trajectory -> State
class DecisionProblemSupervisedEpisode : public Function
{
public:
  DecisionProblemSupervisedEpisode(const FunctionPtr& supervisedDecisionMaker = FunctionPtr())
    : supervisedDecisionMaker(supervisedDecisionMaker) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? containerClass() : decisionProblemStateClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    const TypePtr& stateType = inputVariables[0]->getType();
    const TypePtr& actionType = Container::getTemplateParameter(inputVariables[1]->getType());
    return supervisedDecisionMaker->initialize(context, stateType, actionType) ? stateType : TypePtr();
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DecisionProblemStatePtr& initialState = inputs[0].getObjectAndCast<DecisionProblemState>();
    const ContainerPtr& trajectory = inputs[1].getObjectAndCast<Container>();

    DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();
    size_t n = trajectory->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = trajectory->getElement(i);
      supervisedDecisionMaker->compute(context, state, action);
      double reward;
      state->performTransition(action, reward);
    }
    return state;
  }

  const FunctionPtr& getSupervisedDecisionMaker() const
    {return supervisedDecisionMaker;}
 
protected:
  friend class DecisionProblemSupervisedEpisodeClass;

  FunctionPtr supervisedDecisionMaker;
};

typedef ReferenceCountedObjectPtr<DecisionProblemSupervisedEpisode> DecisionProblemSupervisedEpisodePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GO_SUPERVISED_EPISODE_FUNCTION_H_
