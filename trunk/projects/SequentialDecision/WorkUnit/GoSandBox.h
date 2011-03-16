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
# include <lbcpp/UserInterface/MatrixComponent.h>

namespace lbcpp
{

/////////////////

// State, Action -> DoubleVector
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
    for (size_t i = 0; i < n; ++i)
      if (availableActions->getElement(i) == action)
        res->setValue(i, -1);
    return res;
  }
};

// State -> Container[DoubleVector]
class DecisionProblemStateActionsPerception : public Function
{
public:
  DecisionProblemStateActionsPerception(FunctionPtr stateActionPerception = FunctionPtr())
    : stateActionPerception(stateActionPerception) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return decisionProblemStateClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!stateActionPerception->initialize(context, inputVariables[0]->getType(), stateActionPerception->getRequiredInputType(1, 2)))
      return TypePtr();
    return vectorClass(stateActionPerception->getOutputType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DecisionProblemStatePtr& state = input.getObjectAndCast<DecisionProblemState>();
    ContainerPtr availableActions = state->getAvailableActions();
    size_t n = availableActions->getNumElements();

    VectorPtr res = vector(stateActionPerception->getOutputType(), n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = availableActions->getElement(i);
      res->setElement(i, stateActionPerception->compute(context, state, action));
    }
    return res;
  }

protected:
  friend class DecisionProblemStateActionsPerceptionClass;

  FunctionPtr stateActionPerception; // State, Action -> Perception
};


// State, Supervision Action -> Ranking Example
class DecisionProblemStateActionsRankingExample : public CompositeFunction
{
public:
  DecisionProblemStateActionsRankingExample(FunctionPtr stateActionPerception = FunctionPtr())
    : stateActionPerception(stateActionPerception) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(decisionProblemStateClass, T("state"));
    size_t supervision = builder.addInput(anyType, T("supervision"));
    size_t perceptions = builder.addFunction(new DecisionProblemStateActionsPerception(stateActionPerception), state);
    if (stateActionPerception->getOutputType())
    {
      size_t costs = builder.addFunction(new DecisionProblemStateActionsRankingCostsFunction(), state, supervision);
      builder.addFunction(createObjectFunction(pairClass(vectorClass(stateActionPerception->getOutputType()), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration))), perceptions, costs);
    }
  }

protected:
  friend class DecisionProblemStateActionsRankingExampleClass;

  FunctionPtr stateActionPerception;
};

////////////////////////////////////////////

class GoActionIdentifierFeature : public FeatureGenerator
{
public:
  GoActionIdentifierFeature(size_t size = 19)
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
    GoActionPtr action = inputs[0].getObjectAndCast<GoAction>();
    callback.sense(action->getY() * size + action->getX(), 1.0);
  }

protected:
  size_t size;
};

class GoStateActionFeatures : public CompositeFunction
{
public:
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? goActionClass : goStateClass;}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(goStateClass, T("state"));
    size_t action = builder.addInput(goActionClass, T("action"));
    
    builder.addFunction(new GoActionIdentifierFeature(), action);
  }
};

////////////////////////////////////////////

class GoActionScoringScoreObject : public ScoreObject
{
public:
  GoActionScoringScoreObject() : predictionRate(new ScalarVariableMean(T("predictionRate"))) {}

  bool add(ExecutionContext& context, const DenseDoubleVectorPtr& scores, size_t correctAction)
  {
    int index = scores->getIndexOfMaximumValue();
    if (index < 0)
    {
      context.errorCallback(T("Could not find maximum score"));
      return false;
    }
    predictionRate->push((size_t)index == correctAction ? 1.0 : 0.0);
    return true;
  }

  virtual double getScoreToMinimize() const
    {return 1.0 - predictionRate->getMean();} // prediction error

private:
  friend class GoActionScoringScoreObjectClass;

  ScalarVariableMeanPtr predictionRate;
};

class GoActionScoringEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual TypePtr getRequiredSupervisionType() const
    {return positiveIntegerType;}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context) const
    {return new GoActionScoringScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {result.staticCast<GoActionScoringScoreObject>()->add(context, prediction.getObjectAndCast<DenseDoubleVector>(), (size_t)supervision.getInteger());}
};

//////////

class GoStateComponent : public MatrixComponent
{
public:
  GoStateComponent(GoStatePtr state, const String& name)
    : MatrixComponent(state->getBoard()) {}
 
  virtual juce::Colour selectColour(const Variable& element) const
  {
    if (!element.exists())
      return Colours::lightgrey;
    const juce::Colour colours[] = {juce::Colours::beige, juce::Colours::black, juce::Colours::white, juce::Colours::grey};
    return colours[element.getInteger() % (sizeof (colours) / sizeof (juce::Colour))];
  }
};

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
    //setEvaluator(containerSupervisedEvaluator(new GoActionScoringEvaluator()));
    setBatchLearner(learningParameters->createBatchLearner(context));
    return SimpleBinaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& state = inputs[0].getObjectAndCast<GoState>();
    const ContainerPtr& trajectory = inputs[1].getObjectAndCast<Container>();

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
 
protected:
  friend class GoEpisodeFunctionClass;

  LearnerParametersPtr learningParameters;
  FunctionPtr rankingExampleCreator;
  FunctionPtr rankingMachine;
};


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

  ContainerPtr convertGamesToRankingExamples(ExecutionContext& context, const ContainerPtr& games) const
  {
    FunctionPtr createRankingExampleFunction = new DecisionProblemStateActionsRankingExample(new GoStateActionFeatures());
    if (!createRankingExampleFunction->initialize(context, goStateClass, goActionClass))
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(createRankingExampleFunction->getOutputType(), 0);
    size_t numGames = games->getNumElements();

    for (size_t i = 0; i < numGames; ++i)
    {
      PairPtr game = games->getElement(i).getObjectAndCast<Pair>();
      if (!game)
        continue;

      DecisionProblemStatePtr state = game->getFirst().getObject()->cloneAndCast<DecisionProblemState>();
      const ContainerPtr& trajectory = game->getSecond().getObjectAndCast<Container>();
      size_t n = trajectory->getNumElements();
      for (size_t j = 0; j < n; ++j)
      {
        Variable action = trajectory->getElement(j);

        res->append(createRankingExampleFunction->compute(context, state, action));

        double reward;
        state->performTransition(action, reward);
      }
    }

    return res;
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

    /* make ranking examples
    context.enterScope(T("Converting games to ranking examples ..."));
    ContainerPtr trainingExamples = convertGamesToRankingExamples(context, trainingGames);
    ContainerPtr validationExamples = convertGamesToRankingExamples(context, validationGames);
    context.leaveScope(Variable());
    if (!trainingExamples || !validationExamples)
      return false;
    context.informationCallback(String((int)trainingExamples->getNumElements()) + T(" training examples, ") +
                                String((int)validationExamples->getNumElements()) + T(" validation examples"));
    */

    //context.resultCallback(T("training examples"), trainingExamples);
    //context.resultCallback(T("validation examples"), validationExamples);

    // create ranking machine
    if (!learningParameters)
    {
      context.errorCallback(T("No learning parameters"));
      return false;
    }
    FunctionPtr rankingExampleCreator = new DecisionProblemStateActionsRankingExample(new GoStateActionFeatures());
    FunctionPtr rankingMachine = linearRankingMachine(learningParameters);
    //rankingMachine->setEvaluator(new GoActionScoringEvaluator());

    FunctionPtr goEpisodeFunction = new GoEpisodeFunction(learningParameters, rankingExampleCreator, rankingMachine);
    if (!goEpisodeFunction->initialize(context, goStateClass, containerClass(goActionClass)))
      return false;
    
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

  
#if 0
    if (!fileToParse.existsAsFile())
    {
      context.errorCallback(T("File to parse does not exist"));
      return false;
    }

    XmlElementPtr xml = (new SGFFileParser(context, fileToParse))->next().dynamicCast<XmlElement>();
    if (!xml)
      return false;

    //context.resultCallback(T("XML"), xml);

    FunctionPtr convert = new ConvertSGFXmlToStateAndTrajectory();
    PairPtr stateAndTrajectory = convert->compute(context, xml).getObjectAndCast<Pair>();
    if (!stateAndTrajectory)
      return false;
  
    Variable initialState = stateAndTrajectory->getFirst();
    ContainerPtr trajectory = stateAndTrajectory->getSecond().getObjectAndCast<Container>();

    context.resultCallback(T("Initial State"), initialState);
    context.resultCallback(T("Trajectory"), trajectory);

    DecisionProblemPtr problem = new GoProblem(0);
    if (!problem->initialize(context))
      return false;

    Variable finalState = problem->computeFinalState(context, initialState, trajectory);
    context.resultCallback(T("Final State"), finalState);
    return true;
#endif // 0
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
