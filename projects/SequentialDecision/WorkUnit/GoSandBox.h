/*-----------------------------------------.---------------------------------.
| Filename: GoSandBox.h                    | Go Sand Box                     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_

# include "../Problem/GoProblem.h"
# include "../Problem/LoadSGFFileFunction.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Core/CompositeFunction.h>

namespace lbcpp
{

///////////////////////////////
// More/less generic DP stuff /
///////////////////////////////

  // State -> Container[Action]
class GetAvailableActionsFunction : public SimpleUnaryFunction
{
public:
  GetAvailableActionsFunction(TypePtr actionType)
    : SimpleUnaryFunction(decisionProblemStateClass, containerClass(actionType), T("Actions")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DecisionProblemStatePtr& state = input.getObjectAndCast<DecisionProblemState>();
    return state->getAvailableActions();
  }
};

// State, Action -> DoubleVector
// TODO: transform into function FindElementInContainer: 
//     Container<T>, T -> PositiveInteger
// et gerer la supervision avec PositiveInteger dans le Ranking
class DecisionProblemStateActionsRankingCostsFunction : public SimpleBinaryFunction
{
public:
  DecisionProblemStateActionsRankingCostsFunction()
    : SimpleBinaryFunction(decisionProblemStateClass, variableType, denseDoubleVectorClass(positiveIntegerEnumerationEnumeration)) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DecisionProblemStatePtr& state = inputs[0].getObjectAndCast<DecisionProblemState>();
    const Variable& action = inputs[1];

    ContainerPtr availableActions = state->getAvailableActions();
    size_t n = availableActions->getNumElements();

    DenseDoubleVectorPtr res(new DenseDoubleVector(outputType, n, 0.0));
    bool actionFound = false;
    for (size_t i = 0; i < n; ++i)
      if (availableActions->getElement(i) == action)
      {
        res->setValue(i, -1);
        actionFound = true;
      }

    if (!actionFound)
      context.warningCallback(T("Could not find action ") + action.toShortString() + T(" in state ") + state->toShortString());
    return res;
  }
};

// State, Supervision Action -> Ranking Example
class DecisionProblemStateActionsRankingExample : public CompositeFunction
{
public:
  DecisionProblemStateActionsRankingExample(FunctionPtr actionsPerception = FunctionPtr())
    : actionsPerception(actionsPerception) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(decisionProblemStateClass, T("state"));
    size_t supervision = builder.addInput(anyType, T("supervision"));
    size_t perceptions = builder.addFunction(actionsPerception, state);
    if (actionsPerception->getOutputType())
    {
      size_t costs = builder.addFunction(new DecisionProblemStateActionsRankingCostsFunction(), state, supervision);
      builder.addFunction(createObjectFunction(pairClass(actionsPerception->getOutputType(), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration))), perceptions, costs);
    }
  }

protected:
  friend class DecisionProblemStateActionsRankingExampleClass;

  FunctionPtr actionsPerception; // State -> Container[DoubleVector]
};

///////////////////////////////
// Go Action Features /////////
///////////////////////////////

// GoState -> GoBoard
class GetGoBoardWithCurrentPlayerAsBlack : public SimpleUnaryFunction
{
public:
  GetGoBoardWithCurrentPlayerAsBlack() : SimpleUnaryFunction(goStateClass, goBoardClass, T("Board")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GoStatePtr& state = input.getObjectAndCast<GoState>();
    return Variable(state->getBoardWithCurrentPlayerAsBlack(), outputType);
  }
};

// GoAction -> DoubleVector
class GoActionPositionFeature : public FeatureGenerator
{
public:
  GoActionPositionFeature(size_t size = 19)
    : size(size) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return goActionClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("positions"));
    for (size_t i = 0; i < size; ++i)
      for (size_t j = 0; j < size; ++j)
      {
        String shortName;
        shortName += (juce::tchar)('a' + i);
        shortName += (juce::tchar)('a' + j);
        res->addElement(context, String((int)i) + T(", ") + String((int)j), String::empty, shortName);
      }
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const GoActionPtr& action = inputs[0].getObjectAndCast<GoAction>();
    callback.sense(action->getY() * size + action->getX(), 1.0);
  }

protected:
  size_t size;
};

class GoActionDistanceFeatureGenerator : public FeatureGenerator
{
public:
  GoActionDistanceFeatureGenerator(size_t boardSize = 19, size_t numAxisDistanceIntervals = 10, size_t numDiagDistanceIntervals = 10)
    : boardSize(boardSize), numAxisDistanceIntervals(numAxisDistanceIntervals), numDiagDistanceIntervals(numDiagDistanceIntervals) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return goActionClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    axisDistanceFeatureGenerator = signedNumberFeatureGenerator(softDiscretizedLogNumberFeatureGenerator(0.0, log10(boardSize + 1.0), numAxisDistanceIntervals, false));
    diagDistanceFeatureGenerator = softDiscretizedLogNumberFeatureGenerator(0.0, log10(sqrt((double)(boardSize * boardSize * 2)) + 1.0), numDiagDistanceIntervals, false);

    if (!axisDistanceFeatureGenerator->initialize(context, doubleType))
      return EnumerationPtr();
    if (!diagDistanceFeatureGenerator->initialize(context, doubleType))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("goActionDistanceFeatures"));
    res->addElementsWithPrefix(context, axisDistanceFeatureGenerator->getFeaturesEnumeration(), T("hor."), T("h."));
    i1 = res->getNumElements();
    res->addElementsWithPrefix(context, axisDistanceFeatureGenerator->getFeaturesEnumeration(), T("ver."), T("v."));
    i2 = res->getNumElements();
    res->addElementsWithPrefix(context, diagDistanceFeatureGenerator->getFeaturesEnumeration(), T("diag."), T("d."));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const GoActionPtr& action1 = inputs[0].getObjectAndCast<GoAction>();
    const GoActionPtr& action2 = inputs[1].getObjectAndCast<GoAction>();
    double x1 = (double)action1->getX();
    double y1 = (double)action1->getY();
    double x2 = (double)action2->getX();
    double y2 = (double)action2->getY();
    double dx = x2 - x1;
    double dy = y2 - y1;

    Variable h(dx);
    callback.sense(0, axisDistanceFeatureGenerator, &h, 1.0);

    Variable v(dy);
    callback.sense(i1, axisDistanceFeatureGenerator, &v, 1.0);

    Variable d(sqrt(dx * dx + dy * dy));
    callback.sense(i2, diagDistanceFeatureGenerator, &d, 1.0);
  }

private:
  size_t boardSize;
  size_t numAxisDistanceIntervals;
  size_t numDiagDistanceIntervals;

  FeatureGeneratorPtr axisDistanceFeatureGenerator;
  FeatureGeneratorPtr diagDistanceFeatureGenerator;
  size_t i1, i2;
};

class GoStatePreFeatures : public Object
{
public:
  GoStatePtr state;
  GoActionVectorPtr previousActions;
  GoBoardPtr board; // with current player as black
  MatrixPtr boardPrimaryFeatures;
  // 4-connexity-graph
  // 8-connexity-graph
  // ...
};

extern ClassPtr goStatePreFeaturesClass(TypePtr primaryFeaturesEnumeration);

////////////////////

// GoState -> Container[DoubleVector]
class GoActionsPerception : public CompositeFunction
{
public:
  GoActionsPerception(size_t boardSize = 19)
    : boardSize(boardSize)
  {
  }

  Variable getRelativePositionFeatures(ExecutionContext& context, const Variable& previousAction, const Variable& currentAction) const
  {
    const GoActionPtr& a1 = previousAction.getObjectAndCast<GoAction>();
    const GoActionPtr& a2 = currentAction.getObjectAndCast<GoAction>();
    size_t index = (((a1->getX() * boardSize) + a1->getY()) * boardSize + a2->getX()) * boardSize + a2->getY();
    jassert(index < relativePositionFeatures.size());
    return relativePositionFeatures[index];
  }

  virtual void actionFeatures(CompositeFunctionBuilder& builder)
  {
    size_t action = builder.addInput(goActionClass, T("action"));
    size_t preFeatures = builder.addInput(goStatePreFeaturesClass(enumValueType), T("preFeatures"));

    size_t previousActions = builder.addFunction(getVariableFunction(T("previousActions")), preFeatures);
    size_t boardPrimaryFeatures = builder.addFunction(getVariableFunction(T("boardPrimaryFeatures")), preFeatures);

    size_t row = builder.addFunction(getVariableFunction(1), action);
    size_t column = builder.addFunction(getVariableFunction(0), action);

    FunctionPtr fun = lbcppMemberBinaryFunction(GoActionsPerception, getRelativePositionFeatures, goActionClass, goActionClass,
                                                relativePositionFeatures[0]->getClass());
    size_t previousActionRelationFeatures = builder.addFunction(mapContainerFunction(fun), previousActions, action);

    builder.startSelection();

      size_t i1 = builder.addFunction(matrixWindowFeatureGenerator(5, 5), boardPrimaryFeatures, row, column, T("window"));
      size_t i2 = builder.addFunction(new GoActionPositionFeature(boardSize), action, T("position"));

      builder.addFunction(fixedContainerWindowFeatureGenerator(0, 10), previousActionRelationFeatures, T("previousAction"));
      //builder.addFunction(cartesianProductFeatureGenerator(true), i1, i1, T("prod"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  virtual void preFeaturesFunction(CompositeFunctionBuilder& builder)
  {
    builder.startSelection();

      size_t state = builder.addInput(goStateClass, T("state"));
      size_t previousActions = builder.addFunction(getVariableFunction(T("previousActions")), state, T("previousActions"));
      size_t board = builder.addFunction(new GetGoBoardWithCurrentPlayerAsBlack(), state, T("board"));
      size_t boardPrimaryFeatures = builder.addFunction(mapContainerFunction(enumerationFeatureGenerator(false)), board);
      EnumerationPtr primaryFeaturesEnumeration = DoubleVector::getElementsEnumeration(Container::getTemplateParameter(builder.getOutputType()));
      jassert(primaryFeaturesEnumeration);

    builder.finishSelectionWithFunction(createObjectFunction(goStatePreFeaturesClass(primaryFeaturesEnumeration)), T("goPreFeatures"));
  }

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    makePrecalculations(builder.getContext());

    size_t state = builder.addInput(goStateClass, T("state"));
    size_t preFeatures = builder.addFunction(lbcppMemberCompositeFunction(GoActionsPerception, preFeaturesFunction), state);
    size_t actions = builder.addFunction(new GetAvailableActionsFunction(goActionClass), state, T("actions"));
    
    builder.addFunction(mapContainerFunction(lbcppMemberCompositeFunction(GoActionsPerception, actionFeatures)), actions, preFeatures);
  }

private:
  size_t boardSize;

  // x1 -> y1 -> x2 -> y2 -> features
  std::vector<DoubleVectorPtr> relativePositionFeatures;
  // todo: en faire une symmetricMatrix

  void relativePositionFeaturesFunction(CompositeFunctionBuilder& builder)
  {
    size_t p1 = builder.addInput(goActionClass, T("position1"));
    size_t p2 = builder.addInput(goActionClass, T("position2"));

    builder.startSelection();

      builder.addFunction(new GoActionDistanceFeatureGenerator(boardSize), p1, p2, T("dist"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false), T("relativePos"));
  }

  bool makePrecalculations(ExecutionContext& context)
  {
    FunctionPtr function = lbcppMemberCompositeFunction(GoActionsPerception, relativePositionFeaturesFunction);
    if (!function->initialize(context, goActionClass, goActionClass))
      return false;

    relativePositionFeatures.resize(boardSize * boardSize * boardSize * boardSize);
    size_t index = 0;
    for (size_t x1 = 0; x1 < boardSize; ++x1)
      for (size_t y1 = 0; y1 < boardSize; ++y1)
      {
        GoActionPtr a1(new GoAction(x1, y1));
        for (size_t x2 = 0; x2 < boardSize; ++x2)
          for (size_t y2 = 0; y2 < boardSize; ++y2)
          {
            GoActionPtr a2(new GoAction(x2, y2));
            relativePositionFeatures[index++] = function->compute(context, a1, a2).getObjectAndCast<DoubleVector>();
          }
      }

    return true;
  }
};

///////////////////////////////
/////// GoEpisodeFunction /////
///////////////////////////////

// InitialState, Trajectory -> Nil
class GoEpisodeFunction : public SimpleBinaryFunction
{
public:
  GoEpisodeFunction(LearnerParametersPtr learningParameters = LearnerParametersPtr(), FunctionPtr rankingExampleCreator = FunctionPtr(), FunctionPtr rankingMachine = FunctionPtr())
    : SimpleBinaryFunction(goStateClass, containerClass(goActionClass), objectVectorClass(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration))),
      learningParameters(learningParameters), rankingExampleCreator(rankingExampleCreator), rankingMachine(rankingMachine)
  {
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!rankingExampleCreator->initialize(context, goStateClass, goActionClass))
      return TypePtr();
    TypePtr rankingExampleType = rankingExampleCreator->getOutputType();
    if (!rankingMachine->initialize(context, rankingExampleType->getMemberVariableType(0), rankingExampleType->getMemberVariableType(1)))
      return TypePtr();
    return SimpleBinaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& initialState = inputs[0].getObjectAndCast<GoState>();
    const ContainerPtr& trajectory = inputs[1].getObjectAndCast<Container>();

    GoStatePtr state = initialState->cloneAndCast<GoState>();

    size_t n = trajectory->getNumElements();
    ObjectVectorPtr res = new ObjectVector(getOutputType());
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = trajectory->getElement(i);

      Variable rankingExample = rankingExampleCreator->compute(context, state, action);
      DenseDoubleVectorPtr scores = rankingMachine->computeWithInputsObject(context, rankingExample.getObject()).getObjectAndCast<DenseDoubleVector>();
      res->append(scores);

      double reward;
      state->performTransition(action, reward);
    }
    return res;
  }

  const FunctionPtr& getRankingMachine() const
    {return rankingMachine;}
 
protected:
  friend class GoEpisodeFunctionClass;

  LearnerParametersPtr learningParameters;
  FunctionPtr rankingExampleCreator;
  FunctionPtr rankingMachine;
};

typedef ReferenceCountedObjectPtr<GoEpisodeFunction> GoEpisodeFunctionPtr;

///////////////////////////////
/////// Evaluators ////////////
///////////////////////////////

class CallbackBasedEvaluator : public Evaluator
{
public:
  CallbackBasedEvaluator(EvaluatorPtr evaluator)
    : evaluator(evaluator), callback(NULL) {}

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const = 0;

  struct Callback : public FunctionCallback
  {
    Callback(const EvaluatorPtr& evaluator, const ScoreObjectPtr& scores)
      : evaluator(evaluator), scores(scores) {}

    EvaluatorPtr evaluator;
    ScoreObjectPtr scores;

    virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
    {
      ObjectPtr inputsObject = Object::create(function->getInputsClass());
      for (size_t i = 0; i < inputsObject->getNumVariables(); ++i)
        inputsObject->setVariable(i, inputs[i]);
      evaluator->updateScoreObject(context, scores, inputsObject, output);
    }
  };

  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
  {
    ScoreObjectPtr res = evaluator->createEmptyScoreObject(context, function);
    FunctionPtr functionToListen = getFunctionToListen(function);
    functionToListen->addPostCallback(const_cast<CallbackBasedEvaluator* >(this)->callback = new Callback(evaluator, res));
    return res;
  }

  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
    {return true;}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
  {
    evaluator->finalizeScoreObject(scores, function);
    getFunctionToListen(function)->removePostCallback(callback);
    deleteAndZero(const_cast<CallbackBasedEvaluator* >(this)->callback);
  }

protected:
  friend class CallbackBasedEvaluatorClass;

  EvaluatorPtr evaluator;
  Callback* callback; // pas bien: effet de bord
};

////

class GoActionScoringScoreObject : public ScoreObject
{
public:
  GoActionScoringScoreObject() 
    : predictionRate(new ScalarVariableMean(T("predictionRate"))), 
      rankOfAction(new ScalarVariableStatistics(T("rankOfAction"))),
      unsupervisedRate(new ScalarVariableMean(T("unsupervisedRate"))) {}

  static int getRank(const std::multimap<double, size_t>& sortedScores, size_t index)
  {
    int res = 0;
    for (std::multimap<double, size_t>::const_iterator it = sortedScores.begin(); it != sortedScores.end(); ++it, ++res)
      if (it->second == index)
        return res;
    return -1;
  }

  bool add(ExecutionContext& context, const DenseDoubleVectorPtr& scores, const DenseDoubleVectorPtr& costs)
  {
    std::multimap<double, size_t> sortedScores;
    for (size_t i = 0; i < scores->getNumElements(); ++i)
      sortedScores.insert(std::make_pair(-(scores->getValue(i)), i));
    
    if (sortedScores.empty())
    {
      context.errorCallback(T("No scores"));
      return false;
    }

    // prediction rate
    size_t selectedAction = sortedScores.begin()->second;
    predictionRate->push(costs->getValue(selectedAction) < 0 ? 1.0 : 0.0);

    // rank of selected action
    size_t index = costs->getIndexOfMinimumValue();
    if (index >= 0 && costs->getValue(index) < 0)
    {
      int rank = getRank(sortedScores, index);
      if (rank >= 0)
      {
        rankOfAction->push((double)rank);
        unsupervisedRate->push(0.0);
      }
      else
        unsupervisedRate->push(1.0);
    }
    else
      unsupervisedRate->push(1.0);
    return true;
  }

  virtual double getScoreToMinimize() const
    //{return 1.0 - predictionRate->getMean();} // prediction error
    {return rankOfAction->getMean();} // mean rank of best action

private:
  friend class GoActionScoringScoreObjectClass;

  ScalarVariableMeanPtr predictionRate;
  ScalarVariableStatisticsPtr rankOfAction;
  ScalarVariableMeanPtr unsupervisedRate;
};

class GoActionScoringEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual TypePtr getRequiredSupervisionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new GoActionScoringScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {result.staticCast<GoActionScoringScoreObject>()->add(context, prediction.getObjectAndCast<DenseDoubleVector>(), supervision.getObjectAndCast<DenseDoubleVector>());}
};

class GoEpisodeFunctionEvaluator : public CallbackBasedEvaluator
{
public:
  GoEpisodeFunctionEvaluator() : CallbackBasedEvaluator(new GoActionScoringEvaluator()) {}

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const
  {
    const GoEpisodeFunctionPtr& episodeFunction = evaluatedFunction.staticCast<GoEpisodeFunction>();
    return episodeFunction->getRankingMachine();
  }
};

///////////////////////////////
////////// SandBox ////////////
///////////////////////////////

juce::Component* GoStateComponent::createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
{
  const PairPtr& matrixAndPosition = variable.getObjectAndCast<Pair>();
  const PairPtr& position = matrixAndPosition->getSecond().getObjectAndCast<Pair>();
  Variable goAction(new GoAction(position->getSecond().getInteger(), position->getFirst().getInteger()));

  FunctionPtr perception = new GoActionsPerception();
  ContainerPtr actionPerceptions = perception->compute(context, state).getObjectAndCast<Container>();
  Variable actionPerception;
  if (actionPerceptions)
  {
    ContainerPtr actions = state->getAvailableActions();
    jassert(actions->getNumElements() == actionPerceptions->getNumElements());
    for (size_t i = 0; i < actions->getNumElements(); ++i)
      if (actions->getElement(i) == goAction)
      {
        actionPerception = actionPerceptions->getElement(i);
        break;
      }
  }

  if (actionPerception.exists())
    return userInterfaceManager().createVariableTreeView(context, actionPerception, name + T(" perception"), false);
  else
    return NULL;
}

class GoSandBox : public WorkUnit
{
public:
  GoSandBox() : maxCount(0), numFolds(7), learningParameters(new StochasticGDParameters(constantIterationFunction(1.0)))
  {
  }

  ContainerPtr loadGames(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    if (!gamesDirectory.isDirectory())
    {
      context.errorCallback(T("Invalid games directory"));
      return ContainerPtr();
    }

    return directoryFileStream(context, directory, T("*.sgf"))->load(maxCount, false)->apply(context, new LoadSGFFileFunction(), Container::parallelApply);
  }

  virtual Variable run(ExecutionContext& context)
  {
    // create problem
    DecisionProblemPtr problem = new GoProblem(0);

    // load games
    ContainerPtr games = loadGames(context, gamesDirectory, maxCount);
    if (!games)
      return false;
    ContainerPtr trainingGames = games->invFold(0, numFolds);
    ContainerPtr validationGames = games->fold(0, numFolds);
    context.informationCallback(String((int)trainingGames->getNumElements()) + T(" training games, ") +
                                String((int)validationGames->getNumElements()) + T(" validation games"));

#if 0
    // TMP
    PairPtr pair = trainingGames->getElement(0).getObjectAndCast<Pair>();
    DecisionProblemStatePtr state = pair->getFirst().getObjectAndCast<DecisionProblemState>();
    ContainerPtr trajectory  = pair->getSecond().getObjectAndCast<Container>();
    for (size_t i = 0; i < 20; ++i)
    {
      double r;
      state->performTransition(trajectory->getElement(i), r);
    }
    context.resultCallback(T("state"), state);

    FunctionPtr perception = new GoActionsPerception();
    context.resultCallback(T("actionFeatures"), perception->compute(context, state));
    return true;
    // -
#endif // 0

    // create ranking machine
    if (!learningParameters)
    {
      context.errorCallback(T("No learning parameters"));
      return false;
    }
    FunctionPtr rankingExampleCreator = new DecisionProblemStateActionsRankingExample(new GoActionsPerception());
    StochasticGDParametersPtr sgdParameters = learningParameters.dynamicCast<StochasticGDParameters>();
    if (!sgdParameters)
    {
      context.errorCallback(T("Learning parameters type not supported"));
      return false;
    }
    FunctionPtr rankingMachine = linearRankingMachine(new StochasticGDParameters(sgdParameters->getLearningRate(), StoppingCriterionPtr(), 0,
                                                                                 sgdParameters->doPerEpisodeUpdates(), sgdParameters->doNormalizeLearningRate(),
                                                                                 false, true, false));
    //rankingMachine->setEvaluator(new GoActionScoringEvaluator());

    FunctionPtr goEpisodeFunction = new GoEpisodeFunction(learningParameters, rankingExampleCreator, rankingMachine);
    if (!goEpisodeFunction->initialize(context, goStateClass, containerClass(goActionClass)))
      return false;
    EvaluatorPtr evaluator = new GoEpisodeFunctionEvaluator();
    evaluator->setUseMultiThreading(true);
    goEpisodeFunction->setEvaluator(evaluator);
    goEpisodeFunction->setBatchLearner(learningParameters->createBatchLearner(context));
    goEpisodeFunction->setOnlineLearner(
      compositeOnlineLearner(evaluatorOnlineLearner(false, true), stoppingCriterionOnlineLearner(sgdParameters->getStoppingCriterion()), restoreBestParametersOnlineLearner()));
    //rankingMachine->setOnlineLearner(perEpisodeGDOnlineLearner(FunctionPtr(), constantIterationFunction(1.0), true));
    
    goEpisodeFunction->train(context, trainingGames, validationGames, T("Training"), true);

    goEpisodeFunction->evaluate(context, trainingGames, EvaluatorPtr(), T("Evaluating on training examples"));
    goEpisodeFunction->evaluate(context, validationGames, EvaluatorPtr(), T("Evaluating on validation examples"));
    return true;

   // return learnOnline(context, rankingMachine, trainingExamples, validationExamples);

    /*
    // check validity
    context.enterScope(T("Check validity"));
    bool ok = true;
    for (size_t i = 0; i < games->getNumElements(); ++i)
    {
      context.progressCallback(new ProgressionState(i, games->getNumElements(), T("Games")));
      PairPtr stateAndTrajectory = games->getElement(i).getObjectAndCast<Pair>();
      if (stateAndTrajectory)
      {
        DecisionProblemStatePtr state = stateAndTrajectory->getFirst().getObject()->cloneAndCast<DecisionProblemState>();
        ok &= state->checkTrajectoryValidity(context, stateAndTrajectory->getSecond().getObjectAndCast<Container>());
      }
      
    }
    context.leaveScope(ok);
    return true;
    */

  }

private:
  friend class GoSandBoxClass;

  File gamesDirectory;
  size_t maxCount;
  size_t numFolds;
  LearnerParametersPtr learningParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
