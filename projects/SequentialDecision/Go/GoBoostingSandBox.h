/*-----------------------------------------.---------------------------------.
| Filename: GoBoostingSandBox.h            | Go Boosting Sand Box            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 11:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_H_

# include "GoProblem.h"
# include "LoadSGFFileFunction.h"
# include "GoActionsPerception.h"
# include "GoSupervisedEpisode.h"
# include "GoActionScoringEvaluator.h"
# include <lbcpp/Execution/WorkUnit.h>

// for boosting
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class SGFToStateSamples : public WorkUnit
{
public:
  SGFToStateSamples() : maxCount(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("Loading input trajectories from ") + context.getFilePath(input));
    ContainerPtr trajectories = loadSGFTrajectories(context, input, maxCount);
    size_t n = trajectories ? trajectories->getNumElements() : 0;
    context.leaveScope(n);
    if (!n)
      return false;
    
    if (!outputDirectory.exists())
      outputDirectory.createDirectory();

    context.enterScope(T("Converting"));
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr stateAndTrajectory = trajectories->getElement(i).getObjectAndCast<Pair>();
      if (stateAndTrajectory)
      {
        PairPtr stateAndAction = sampleStateActionPairFromTrajectory(context, stateAndTrajectory->getFirst().getObjectAndCast<GoState>(), stateAndTrajectory->getSecond().getObjectAndCast<Container>());
        GoStatePtr state = stateAndAction->getFirst().getObjectAndCast<GoState>();
        String name = state->getName();
        if (name.endsWith(T(".sgf")))
          name = name.substring(0, name.length() - 4);
        name += T(".sa");
        stateAndAction->saveToFile(context, outputDirectory.getChildFile(name));
      }
      context.progressCallback(new ProgressionState(i+1, n, "games"));
    }
    context.leaveScope();
    return true;
  }

private:
  friend class SGFToStateSamplesClass;

  File input;
  size_t maxCount;
  File outputDirectory;

  PairPtr sampleStateActionPairFromTrajectory(ExecutionContext& context, GoStatePtr initialState, ContainerPtr actions) const
  {
    size_t trajectoryLength = actions->getNumElements();
    size_t l = context.getRandomGenerator()->sampleSize(0, trajectoryLength);

    GoStatePtr res = initialState->cloneAndCast<GoState>();
    for (size_t i = 0; i < l; ++i)
    {
      Variable action = actions->getElement(i);
      double reward;
      res->performTransition(context, action, reward);
    }
    return new Pair(res, actions->getElement(l));
  }
};
# if 0

//////////////////
////////////////// BoostingWeakModel
//////////////////

class BoostingWeakModel : public SimpleUnaryFunction
{
public:
  BoostingWeakModel() : SimpleUnaryFunction(variableType, doubleType) {}

  virtual double predict(ExecutionContext& context, const Variable& input) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return predict(context, input);}
};

typedef ReferenceCountedObjectPtr<BoostingWeakModel> BoostingWeakModelPtr;

extern ClassPtr boostingWeakModelClass;

class IsFeatureActiveWeakModel : public BoostingWeakModel
{
public:
  IsFeatureActiveWeakModel(EnumerationPtr features = EnumerationPtr(), size_t index = 0)
    : features(features), index(index) {}
   
  virtual String toShortString() const
    {return T("isActive(") + features->getElementName(index) + T(")");}

  virtual double predict(ExecutionContext& context, const Variable& input) const
  {
    DenseDoubleVectorPtr denseVector = input.dynamicCast<DenseDoubleVector>();
    if (denseVector)
      return denseVector->getValue(index) != 0.0 ? 1.0 : 0.0;
    else
    {
      DoubleVectorPtr vector = input.getObjectAndCast<DoubleVector>();
      return vector->getElement(index).getDouble() != 0.0 ? 1.0 : 0.0;
    }
  }

protected:
  friend class IsFeatureActiveWeakModelClass;

  EnumerationPtr features;
  size_t index;
};

class IsFeatureGreaterThanWeakModel : public BoostingWeakModel
{
public:
  IsFeatureGreaterThanWeakModel(EnumerationPtr features = EnumerationPtr(), size_t index = 0, double threshold = 0.0)
    : features(features), index(index), threshold(threshold) {}
   
  virtual String toShortString() const
    {return features->getElementName(index) + " > " + String(threshold);}

  virtual double predict(ExecutionContext& context, const Variable& input) const
  {
    DenseDoubleVectorPtr denseVector = input.dynamicCast<DenseDoubleVector>();
    if (denseVector)
      return denseVector->getValue(index) > threshold ? 1.0 : 0.0;
    else
    {
      DoubleVectorPtr vector = input.getObjectAndCast<DoubleVector>();
      return vector->getElement(index).getDouble() > threshold ? 1.0 : 0.0;
    }
  }

protected:
  friend class IsFeatureGreaterThanWeakModelClass;

  EnumerationPtr features;
  size_t index;
  double threshold;
};


//////////////////
////////////////// BoostingStrongModel
//////////////////

class BoostingStrongModel : public Function
{
public:
  BoostingStrongModel() 
  {
    clear();
  }
 
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return variableType;}

  void clear()
  {
    weights = new DenseDoubleVector(0, 0.0);
    weakModels = new ObjectVector(boostingWeakModelClass, 0);
    votes = VectorPtr();
  }

  void addWeakModel(double weight, const BoostingWeakModelPtr& model, const Variable& vote = Variable())
  {
    size_t index = getNumWeakModels();
    weakModels->append(model);
    weights->appendValue(weight);
    if (vote.exists())
    {
      if (!votes)
        votes = vector(vote.getType());
      if (votes->getNumElements() <= index)
        votes->resize(index + 1);
      votes->setElement(index, vote);
    }
  }

  size_t getNumWeakModels() const
    {return weakModels->getNumElements();}

protected:
  friend class BoostingStrongModelClass;

  DenseDoubleVectorPtr weights;
  ObjectVectorPtr weakModels;
  VectorPtr votes; // only for multi-class / multi-label

  double computeWeakModelsPrediction(ExecutionContext& context, const Variable& input) const
  {
    size_t n = getNumWeakModels();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += weights->getValue(i) * weakModels->getAndCast<BoostingWeakModel>(i)->predict(context, input);
    return res;
  }
};

extern ClassPtr boostingStrongModelClass;

typedef ReferenceCountedObjectPtr<BoostingStrongModel> BoostingStrongModelPtr;

class ScalarBoostingStrongModel : public BoostingStrongModel
{
public:
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? variableType : variableType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    Variable subInput;
    if (inputs[0].inheritsFrom(doubleVectorClass()))
      subInput = inputs[0].getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
    else
      subInput = inputs[0];
    return computeWeakModelsPrediction(context, subInput);
  }
};

class RankingBoostingStrongModel : public BoostingStrongModel
{
public:
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? denseDoubleVectorClass() : containerClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ContainerPtr inputPerceptions = inputs[0].getObjectAndCast<Container>();
    size_t n = inputPerceptions->getNumElements();

    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
    for (size_t i = 0; i < n; ++i)
    {
      Variable perception = inputPerceptions->getElement(i);
      res->setValue(i, computeWeakModelsPrediction(context, perception));
    }
    return res;
  }
};

//////////////////
////////////////// BoostingLearner
//////////////////

class BoostingLearner : public BatchLearner
{
public:
  BoostingLearner(size_t maxIterations = 0)
    : maxIterations(maxIterations) {}

  virtual TypePtr getRequiredFunctionType() const
    {return boostingStrongModelClass;}

  virtual ContainerPtr getExamples(const std::vector<ObjectPtr>& data) const = 0;

  virtual double computeWeakModelScore(ExecutionContext& context, const BoostingWeakModelPtr& weakModel, const ContainerPtr& examples, const DenseDoubleVectorPtr& weights) const = 0;
  virtual double computeWeakModelWeight(double score) const = 0;

  virtual double computeExampleWeightMultiplier(ExecutionContext& context, const Variable& example, double weakModelWeight, const BoostingWeakModelPtr& weakModel) const = 0;

  BoostingWeakModelPtr learnWeakModel(ExecutionContext& context, const ContainerPtr& examples, const DenseDoubleVectorPtr& weights, double& score) const
  {
    // tmp !  the way to enumerate candidate weak models should be abstracted through a sequential decision problem
    DoubleVectorPtr doubleVector = examples->getElement(0).getObjectAndCast<Pair>()->getFirst().getObjectAndCast<DoubleVector>();
    EnumerationPtr inputFeatures = doubleVector->getElementsEnumeration();
    // --

    BoostingWeakModelPtr bestWeakModel;
    double bestScore = 0.0;
    size_t numCandidates = inputFeatures->getNumElements() * 10;

    for (size_t i = 0; i < numCandidates; ++i)
    {
      BoostingWeakModelPtr weakModel = new IsFeatureGreaterThanWeakModel(inputFeatures, i / 10, -1.0 + (i % 10) * 2.0 / 9.0);
      double score = computeWeakModelScore(context, weakModel, examples, weights);
      if (fabs(score) > fabs(bestScore))
      {
        bestScore = score, bestWeakModel = weakModel;
        context.informationCallback(T("New best: ") + weakModel->toShortString() + T(" [") + String(bestScore) + T("]"));
      }

      context.progressCallback(new ProgressionState(i+1, numCandidates, "Candidate Weak Learners"));
    }

    score = bestScore;
    return bestWeakModel;
  }

  void updateExampleWeights(ExecutionContext& context, const ContainerPtr& examples, double weakModelWeight, const BoostingWeakModelPtr& weakModel, DenseDoubleVectorPtr weights) const
  {
    double weightsSum = 0.0;
    size_t n = weights->getNumValues();
    double* w = weights->getValuePointer(0);
    for (size_t i = 0; i < n; ++i)
    {
      w[i] *= computeExampleWeightMultiplier(context, examples->getElement(i), weakModelWeight, weakModel);
      weightsSum += w[i];
    }
    weights->multiplyByScalar(1.0 / weightsSum);
  }

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const BoostingStrongModelPtr& model = f.staticCast<BoostingStrongModel>();
    model->clear();

    ContainerPtr examples = getExamples(trainingData);
    size_t numExamples = examples->getNumElements();
    DenseDoubleVectorPtr weights(new DenseDoubleVector(numExamples, 1.0 / (double)numExamples));
  
    for (size_t iteration = 0; iteration < maxIterations; ++iteration)
    {
      context.enterScope("Iteration " + String((int)iteration + 1));
      context.resultCallback(T("iteration"), iteration + 1);

      double score;
      BoostingWeakModelPtr weakModel = learnWeakModel(context, examples, weights, score);
      double weakModelWeight = computeWeakModelWeight(score);
      model->addWeakModel(weakModelWeight, weakModel);
      
      updateExampleWeights(context, examples, weakModelWeight, weakModel, weights);

      ScoreObjectPtr validationScore = model->evaluate(context, validationData, EvaluatorPtr(), "Evaluating");
      context.resultCallback(T("validationScore"), validationScore->getScoreToMinimize());


      context.resultCallback(T("weakModel"), weakModel);
      context.resultCallback(T("weakModelScore"), score);
      context.resultCallback(T("weakModelWeight"), weakModelWeight);

      context.leaveScope(true);
      context.progressCallback(new ProgressionState(iteration + 1, maxIterations,  "Iterations"));
    }
    return true;
  }

protected:
  friend class BoostingLearnerClass;

  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<BoostingLearner> BoostingLearnerPtr;

//////////////////
////////////////// RankBoost
//////////////////

class RankBoostLearner : public BoostingLearner
{
public:
  RankBoostLearner(size_t maxIterations = 0)
    : BoostingLearner(maxIterations) {}

  virtual ContainerPtr getExamples(const std::vector<ObjectPtr>& data) const
  {
    ObjectVectorPtr res = new ObjectVector(pairClass(doubleVectorClass(), doubleVectorClass()));

    for (size_t i = 0; i < data.size(); ++i)
    {
      const PairPtr& p = data[i].staticCast<Pair>();
      const ContainerPtr& featureVectors = p->getFirst().getObjectAndCast<Container>();
      const DenseDoubleVectorPtr& costs = p->getSecond().getObjectAndCast<DenseDoubleVector>();
      
      bool zeroIsPositive;
      bool ok = RankingLossFunction::areCostsBipartite(costs->getValues(), zeroIsPositive);
      jassert(ok);
      size_t n = costs->getNumValues();
      size_t positiveIndex = (size_t)-1;
      for (size_t j = 0; j < n; ++j)
        if (RankingLossFunction::isPositiveCost(costs->getValue(j), zeroIsPositive))
        {
          positiveIndex = j;
          break;
        }
      jassert(positiveIndex != (size_t)-1);
      DoubleVectorPtr positiveFeatures = featureVectors->getElement(positiveIndex).getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
      for (size_t j = 0; j < n; ++j)
        if (j != positiveIndex)
        {
          DoubleVectorPtr negativeFeatures = featureVectors->getElement(j).getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
          res->append(new Pair(negativeFeatures, positiveFeatures));
        }
    }
    return res;
  }


  double getScoreDifference(ExecutionContext& context, const BoostingWeakModelPtr& weakModel, const Variable& example) const
  {
    const PairPtr& pair = example.getObjectAndCast<Pair>();
    double negativeScore = weakModel->predict(context, pair->getFirst());
    double positiveScore = weakModel->predict(context, pair->getSecond());
    return positiveScore - negativeScore;
  }

  virtual double computeWeakModelScore(ExecutionContext& context, const BoostingWeakModelPtr& weakModel, const ContainerPtr& examples, const DenseDoubleVectorPtr& weights) const
  {
    size_t n = examples->getNumElements();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += weights->getValue(i) * getScoreDifference(context, weakModel, examples->getElement(i));
    return res;
  }

  virtual double computeWeakModelWeight(double score) const
    {return 0.5 * log((1.0 + score) / (1.0 - score));}

  virtual double computeExampleWeightMultiplier(ExecutionContext& context, const Variable& example, double weakModelWeight, const BoostingWeakModelPtr& weakModel) const
    {return exp(- weakModelWeight * getScoreDifference(context, weakModel, example));}
};

//////////////////
////////////////// GoBoostingSandBox
//////////////////

class GoBoostingSandBox : public WorkUnit
{
public:
  GoBoostingSandBox() : maxCount(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("Loading examples"));
    ContainerPtr examples = loadExamples(context, examplesDirectory, maxCount);
    context.leaveScope(examples ? examples->getNumElements() : 0);
    if (!examples)
      return false;
    
    ContainerPtr rankingExamples = makeRankingExamples(context, examples);

    // make train/test split
    ContainerPtr trainingExamples = rankingExamples->invFold(0, 10);
    ContainerPtr testingExamples = rankingExamples->fold(0, 10);

    // learn model with rankboost
    BoostingStrongModelPtr model = new RankingBoostingStrongModel();
    model->setBatchLearner(new RankBoostLearner(20));
    model->setEvaluator(new GoActionScoringEvaluator());
    model->train(context, trainingExamples, testingExamples, "Boosting", true);
    return true;
  }

private:
  friend class GoBoostingSandBoxClass;

  File examplesDirectory;
  size_t maxCount;

  ContainerPtr loadExamples(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    StreamPtr fileStream = directoryFileStream(context, directory);
    return fileStream->load(maxCount)->apply(context, loadFromFileFunction(pairClass(goStateClass, positiveIntegerPairClass)), Container::parallelApply);
  }

  ContainerPtr makeRankingExamples(ExecutionContext& context, const ContainerPtr& stateActionPairs) const
  {
    GoActionsPerceptionPtr perception = new GoActionsPerception(19);
    if (!perception->initialize(context, goStateClass, containerClass(positiveIntegerPairClass)))
      return false;

    context.enterScope("Make ranking examples");
    size_t n = stateActionPairs->getNumElements();
    ObjectVectorPtr rankingExamples = new ObjectVector(pairClass(doubleVectorClass(), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)), n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr stateActionPair = stateActionPairs->getElement(i).getObjectAndCast<Pair>();
      GoStatePtr state = stateActionPair->getFirst().getObjectAndCast<GoState>();
      Variable correctAction = stateActionPair->getSecond();

      ContainerPtr availableActions = state->getAvailableActions();
      
      ContainerPtr actionFeatures = perception->compute(context, state, availableActions).getObjectAndCast<Container>();
      ContainerPtr rankingCosts = computeRankingCosts(context, availableActions, correctAction);
      
      rankingExamples->set(i, new Pair(actionFeatures, rankingCosts));

      context.progressCallback(new ProgressionState(i+1, n, "Examples"));
    }
    context.leaveScope();
    return rankingExamples;
  }

  ContainerPtr computeRankingCosts(ExecutionContext& context, const ContainerPtr& actions, const Variable& correctAction) const
  {
    if (!correctAction.exists())
    {
      context.errorCallback(T("No supervision action"));
      return ContainerPtr();
    }

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

#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_H_
