/*-----------------------------------------.---------------------------------.
| Filename: AdaBoostMHLearner.h            | AdaBoost.MH learner             |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_
# define LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_

# include "WeightBoostingLearner.h"
# include <lbcpp/Learning/LossFunction.h> // for SGD

namespace lbcpp
{

class AdaBoostMHWeakObjective : public BoostingWeakObjective
{
public:
  AdaBoostMHWeakObjective(const LuapeClassifierPtr& classifier, const DenseDoubleVectorPtr& supervisions, const DenseDoubleVectorPtr& weights, const std::vector<size_t>& examples)
    : labels(classifier->getLabels()), doubleVectorClass(classifier->getDoubleVectorClass()), supervisions(supervisions), weights(weights), examples(examples)
  {
    numLabels = labels->getNumElements();
    jassert(examples.size());
  }

  virtual void setPredictions(const VectorPtr& predictions)
  {
    this->predictions = predictions;
    computeMuAndVoteValues();
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(predictions.isInstanceOf<BooleanVector>());
    unsigned char& prediction = predictions.staticCast<BooleanVector>()->getData()[index]; // fast unprotected access
    if (prediction < 2)
    {
      prediction = 1 - prediction;

      double* weightsPtr = weights->getValuePointer(index * numLabels);
      double* muNegativesPtr = muNegatives->getValuePointer(0);
      double* muPositivesPtr = muPositives->getValuePointer(0);
      double* votesPtr = votes->getValuePointer(0);
      double* supervisionsPtr = supervisions->getValuePointer(index * numLabels);
      for (size_t i = 0; i < numLabels; ++i)
      {
        double weight = *weightsPtr++;
        double& muNegative = *muNegativesPtr++;
        double& muPositive = *muPositivesPtr++;
        double supervision = *supervisionsPtr++;
        if ((prediction == 1 && supervision > 0) || (prediction == 0 && supervision < 0))
          {muNegative -= weight; muPositive += weight;}
        else
          {muPositive -= weight; muNegative += weight;}
        *votesPtr++ = muPositive > (muNegative + 1e-09) ? 1.0 : -1.0;
      }
    }
  }

  virtual double computeObjective() const
  {
    size_t n = labels->getNumElements();
    double edge = 0.0;
    for (size_t i = 0; i < n; ++i)
      edge += votes->getValue(i) * (muPositives->getValue(i) - muNegatives->getValue(i));
    return edge;
  }

  const DenseDoubleVectorPtr& getVotes() const
    {return votes;}

  const DenseDoubleVectorPtr& getMuNegatives() const
    {return muNegatives;}

  const DenseDoubleVectorPtr& getMuPositives() const
    {return muPositives;}

protected:
  EnumerationPtr labels;
  size_t numLabels;
  ClassPtr doubleVectorClass;
  VectorPtr predictions;   // size = numExamples
  DenseDoubleVectorPtr supervisions;  // size = numExamples * numLabels
  DenseDoubleVectorPtr weights;   // size = numExamples * numLabels
  const std::vector<size_t>& examples; // size = numExamples

  DenseDoubleVectorPtr muNegatives; // size = numLabels
  DenseDoubleVectorPtr muPositives; // size = numLabels
  DenseDoubleVectorPtr votes;       // size = numLabels

  void computeMuAndVoteValues()
  {
    jassert(supervisions->getNumValues() == predictions->getNumElements() * numLabels);

    muNegatives = new DenseDoubleVector(doubleVectorClass, numLabels);
    muPositives = new DenseDoubleVector(doubleVectorClass, numLabels);
    votes = new DenseDoubleVector(doubleVectorClass, numLabels);

    BooleanVectorPtr booleanPredictions = predictions.dynamicCast<BooleanVector>();
    if (booleanPredictions)
    {
      const unsigned char* predictionsPtr = booleanPredictions->getData();
      for (size_t i = 0; i < examples.size(); ++i)
      {
        size_t example = examples[i];
        unsigned char prediction = predictionsPtr[example];
        if (prediction < 2)
        {
          double* weightsPtr = weights->getValuePointer(numLabels * example);
          double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
          for (size_t j = 0; j < numLabels; ++j)
          {
            double supervision = *supervisionsPtr++;
            bool isPredictionCorrect = (prediction == 0 && supervision < 0) || (prediction == 1 && supervision > 0);
            (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
          }
        }
      }
    }
    else
    {
      DenseDoubleVectorPtr scalarPredictions = predictions.dynamicCast<DenseDoubleVector>();
      for (size_t i = 0; i < examples.size(); ++i)
      {
        size_t example = examples[i];
        double prediction = scalarPredictions->getValue(example) * 2 - 1;
        double* weightsPtr = weights->getValuePointer(numLabels * example);
        double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
        for (size_t j = 0; j < numLabels; ++j)
        {
          double supervision = *supervisionsPtr++;
          bool isPredictionCorrect = (prediction * supervision > 0);
          (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
        }
      }
    }

    // compute v_l values
    for (size_t i = 0; i < numLabels; ++i)
      votes->setValue(i, muPositives->getValue(i) > (muNegatives->getValue(i) + 1e-9) ? 1.0 : -1.0);
  }
};

typedef ReferenceCountedObjectPtr<AdaBoostMHWeakObjective> AdaBoostMHWeakObjectivePtr;

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner)
    : WeightBoostingLearner(weakLearner) {}
  AdaBoostMHLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective(const std::vector<size_t>& examples) const
    {return new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights, examples);}

//  virtual bool shouldStop(double weakObjectiveValue) const
//    {return weakObjectiveValue == 0.0;}

  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const
  {
    EnumerationPtr labels = examples[0]->getClass()->getTemplateArgument(1).staticCast<Enumeration>();
    size_t n = examples.size();
    size_t m = labels->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(n * m, 0.0);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      const PairPtr& example = examples[i].staticCast<Pair>();
      size_t label = (size_t)example->getSecond().getInteger();
      for (size_t j = 0; j < m; ++j, ++index)
        res->setValue(index, j == label ? 1.0 : -1.0);
    }
    return res;
  }

  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples, Variable& successVote, Variable& failureVote) const
  {
    AdaBoostMHWeakObjectivePtr objective = new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights, examples);
    objective->setPredictions(trainingSamples->compute(context, weakNode));

    const DenseDoubleVectorPtr& votes = objective->getVotes();
    const DenseDoubleVectorPtr& muNegatives = objective->getMuNegatives();
    const DenseDoubleVectorPtr& muPositives = objective->getMuPositives();

    double correctWeight = 0.0;
    double errorWeight = 0.0;
    size_t n = votes->getNumValues();
    const double* votesPtr = votes->getValuePointer(0);
    const double* muNegativesPtr = muNegatives->getValuePointer(0);
    const double* muPositivesPtr = muPositives->getValuePointer(0);
    for (size_t i = 0; i < n; ++i)
    {
      if (*votesPtr++ > 0)
      {
        correctWeight += *muPositivesPtr++;
        errorWeight += *muNegativesPtr++;
      }
      else
      {
        correctWeight += *muNegativesPtr++;
        errorWeight += *muPositivesPtr++;
      }
    }

    if (!errorWeight && !correctWeight)
      return false;

    double alpha = 0.5 * log(correctWeight / errorWeight);
    context.resultCallback(T("alpha"), alpha);
    // correctWeight + errorWeight = weight of selected examples (1 if all the examples are selected)
    //jassert(fabs(correctWeight + errorWeight - 1.0) < 1e-9);
    jassert(alpha > 0.0);

    // make symmetric votes
    DenseDoubleVectorPtr res = votes->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(alpha);
    successVote = res;

    res = res->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(-1.0);
    failureVote = res;
    return true;
  }

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const VectorPtr& predictions, double& loss) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = trainingData.size();
    jassert(supervisions->getNumElements() == n * numLabels);
    DenseDoubleVectorPtr res = new DenseDoubleVector(supervisions->getNumElements(), 0.0);
    double* weightsPtr = res->getValuePointer(0);
    double* supervisionsPtr = this->supervisions.staticCast<DenseDoubleVector>()->getValuePointer(0);

    double positiveWeight =  1.0 / (2 * n);
    double negativeWeight = 1.0 / (2 * n * (numLabels - 1));
    const ObjectVectorPtr& predictedActivations = predictions.staticCast<ObjectVector>();
    loss = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      const DenseDoubleVectorPtr& activations = predictedActivations->getAndCast<DenseDoubleVector>(i);
      double* activationsPtr = activations->getValuePointer(0);
      for (size_t j = 0; j < numLabels; ++j)
      {
        double supervision = *supervisionsPtr++;
        double w0 = supervision > 0 ? positiveWeight : negativeWeight;
        double weight = w0 * exp(-supervision * (*activationsPtr++));
        *weightsPtr++ = weight;
        loss += weight;
      }
    }
    res->multiplyByScalar(1.0 / loss);
    return res;
  }

#if 0
  virtual bool doLearningIteration(ExecutionContext& context)
  {
    if (!WeightBoostingLearner::doLearningIteration(context))
      return false;
    return true;

    static int counter = 0;
    ++counter;

    if (true)//(counter % 10) == 0)
    {
      static const size_t maxIterations = 100;

      context.enterScope(T("SGD"));

      context.enterScope(T("Before"));
      context.resultCallback(T("iteration"), (size_t)0);
      context.resultCallback(T("loss"), weightsSum);
      context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainingData));
      context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
      context.leaveScope();

      StoppingCriterionPtr stoppingCriterion = maxIterationsWithoutImprovementStoppingCriterion(2);
      
      for (size_t i = 0; i < maxIterations; ++i)
      {
        context.enterScope(T("Iteration ") + String((int)i+1));
        context.resultCallback(T("iteration"), i+1);
        doSGDIteration(context);
        recomputePredictions(context);
        recomputeWeights(context);
        context.resultCallback(T("loss"), weightsSum);
        double trainError = function->evaluatePredictions(context, predictions, trainingData);
        context.resultCallback(T("train error"), trainError);
        context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));

       /* applyRegularizer(context);
        recomputePredictions(context);
        recomputeWeights(context);
        context.resultCallback(T("loss 2"), weightsSum);
        context.resultCallback(T("train error 2"), function->evaluatePredictions(context, predictions, trainingData));
        context.resultCallback(T("validation error 2"), function->evaluatePredictions(context, validationPredictions, validationData));*/

        context.leaveScope();
        if (stoppingCriterion->shouldStop(trainError))
          break;
      }
      context.leaveScope();
    }
    if (false)//counter > 100)
    {
      context.enterScope(T("Pruning"));
      size_t yieldIndex = pruneSmallestVote(context);

      recomputePredictions(context);
      recomputeWeights(context);

      context.resultCallback(T("yield index"), yieldIndex);
      context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainingData));
      context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
      context.resultCallback(T("loss"), weightsSum);
      context.leaveScope();
      context.resultCallback(T("yield index"), yieldIndex);
    }
    return true;
  }

  size_t pruneSmallestVote(ExecutionContext& context)
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();

    ObjectVectorPtr votes = classifier->getVotes().staticCast<ObjectVector>();
    //size_t numVotes = votes->getNumElements();
    
    double smallestVoteNorm = DBL_MAX;
    size_t smallestVoteIndex = (size_t)-1;

    size_t voteIndex = 0;
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      LuapeYieldNodePtr yieldNode = graph->getNode(i).dynamicCast<LuapeYieldNode>();
      if (yieldNode)
      {
        context.enterScope(T("Vote ") + String((int)voteIndex+1));
        DenseDoubleVectorPtr vote = votes->getAndCast<DenseDoubleVector>(voteIndex);
        context.resultCallback(T("index"), voteIndex);
        context.resultCallback(T("l2norm"), vote->l2norm());
        context.resultCallback(T("l1norm"), vote->l1norm());
        context.resultCallback(T("l0norm"), vote->l0norm());

        // FIXME: do not work with continuous weak learners
        double weightSum = 0.0;
        BooleanVectorPtr predictions = graph->updateNodeCache(context, yieldNode->getArgument(), true);
        DenseDoubleVectorPtr weights = makeInitialWeights(classifier, *(const std::vector<PairPtr>* )&trainingData);
        for (size_t j = 0; j < supervisions->getNumElements(); ++j)
        {
          bool isPredictionCorrect = (predictions->get(j / numLabels) == supervisions.staticCast<BooleanVector>()->get(j));
          double v = vote->getValue(j % numLabels);
          weightSum += weights->getValue(j) * exp(-v * (isPredictionCorrect ? 1.0 : -1.0));
        }
        context.resultCallback(T("weight"), weightSum);

        double voteNorm = weightSum;//vote->l2norm();
        if (voteNorm < smallestVoteNorm)
          smallestVoteNorm = voteNorm, smallestVoteIndex = voteIndex;
        ++voteIndex;
        context.leaveScope();
      }
    }

    jassert(smallestVoteIndex != (size_t)-1);
    context.informationCallback(T("Pruning weak predictor # ") + String((int)smallestVoteIndex));
    context.resultCallback(T("pruned yield index"), smallestVoteIndex);
    votes->getObjects().erase(votes->getObjects().begin() + smallestVoteIndex);
    graph->removeYieldNode(context, smallestVoteIndex);

    return smallestVoteIndex;
  }

  void applyRegularizer(ExecutionContext& context)
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    VectorPtr votes = classifier->getVotes();
    size_t numVotes = votes->getNumElements();

    for (size_t i = 0; i < numVotes; ++i)
    {
      DenseDoubleVectorPtr vote = votes->getElement(i).getObjectAndCast<DenseDoubleVector>();
      vote->multiplyByScalar(0.5);
    }
  }

  void doSGDIteration(ExecutionContext& context)
  {
    static const double learningRate = 0.01;// / (1.0 + sqrt((double)graph->getNumYieldNodes()));

    //MultiClassLossFunctionPtr lossFunction = logBinomialMultiClassLossFunction();
    //MultiClassLossFunctionPtr lossFunction = oneAgainstAllMultiClassLossFunction(exponentialDiscriminativeLossFunction());

    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(graph->getNumTrainingSamples(), order);

    DenseDoubleVectorPtr parameters = new DenseDoubleVector();

    ScalarVariableStatistics loss;

    VectorPtr votes = classifier->getVotes();
    size_t numVotes = votes->getNumElements();

    for (size_t i = 0; i < order.size(); ++i)
    {
      const PairPtr& example = trainingData[order[i]].staticCast<Pair>();

      // compute weak predictions
      DenseDoubleVectorPtr weakPredictions = classifier->computeSignedWeakPredictions(context, example->getFirst().getObject());
      jassert(numVotes == weakPredictions->getNumElements());

      // compute activation
      DenseDoubleVectorPtr activations = new DenseDoubleVector(classifier->getDoubleVectorClass());
      for (size_t j = 0; j < numVotes; ++j)
        votes->getElement(j).getObjectAndCast<DoubleVector>()->addWeightedTo(activations, 0, weakPredictions->getValue(j));

      // compute loss value and gradient
      size_t correctClass = (size_t)example->getSecond().getInteger();
      double lossValue = 0.0;
      DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(classifier->getDoubleVectorClass());
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isCorrectClass = (j == correctClass);
        double weight = 1.0 / (2.0 * (isCorrectClass ? 1.0 : (numLabels - 1)));
        double sign = isCorrectClass ? 1.0 : -1.0;
        double e = exp(-sign * activations->getValue(j));
        lossValue += e * weight;
        lossGradient->setValue(j, -sign * e * weight);
      }
      //lossFunction->computeMultiClassLoss(activations, correctClass, numLabels, &lossValue, &lossGradient, 1.0);
      loss.push(lossValue);

      // update
      for (size_t j = 0; j < numVotes; ++j)
      {
        DenseDoubleVectorPtr v = votes->getElement(j).getObjectAndCast<DenseDoubleVector>();
        lossGradient->addWeightedTo(v, 0, -learningRate * weakPredictions->getValue(j));
      }
    }
    
    context.resultCallback(T("meanLoss"), loss.getMean());
  }
#endif // 0
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_
