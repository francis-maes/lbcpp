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
  AdaBoostMHWeakObjective(const LuapeClassifierPtr& classifier, const BooleanVectorPtr& supervisions, const DenseDoubleVectorPtr& weights)
    : labels(classifier->getLabels()), doubleVectorClass(classifier->getDoubleVectorClass()), supervisions(supervisions), weights(weights)
  {
  }

  virtual void setPredictions(const VectorPtr& predictions)
  {
    this->predictions = predictions;
    computeMuAndVoteValues();
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(predictions.isInstanceOf<BooleanVector>());
    bool newPrediction = predictions.staticCast<BooleanVector>()->flip(index);
    size_t numLabels = labels->getNumElements();
    double* weightsPtr = weights->getValuePointer(index * numLabels);
    double* muNegativesPtr = muNegatives->getValuePointer(0);
    double* muPositivesPtr = muPositives->getValuePointer(0);
    double* votesPtr = votes->getValuePointer(0);
    for (size_t i = 0; i < numLabels; ++i)
    {
      double weight = *weightsPtr++;
      double& muNegative = *muNegativesPtr++;
      double& muPositive = *muPositivesPtr++;
      if (newPrediction == supervisions->get(index * numLabels + i))
        {muNegative -= weight; muPositive += weight;}
      else
        {muPositive -= weight; muNegative += weight;}
      *votesPtr++ = muPositive > muNegative ? 1.0 : -1.0;
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
  ClassPtr doubleVectorClass;
  VectorPtr predictions;   // size = numExamples
  BooleanVectorPtr supervisions;  // size = numExamples * numLabels
  DenseDoubleVectorPtr weights;   // size = numExamples * numLabels
  
  DenseDoubleVectorPtr muNegatives; // size = numLabels
  DenseDoubleVectorPtr muPositives; // size = numLabels
  DenseDoubleVectorPtr votes;       // size = numLabels

  void computeMuAndVoteValues()
  {
    muNegatives = new DenseDoubleVector(doubleVectorClass);
    muPositives = new DenseDoubleVector(doubleVectorClass);
    votes = new DenseDoubleVector(doubleVectorClass);

    size_t numLabels = labels->getNumElements();
    size_t numExamples = predictions->getNumElements();
    jassert(supervisions->getNumElements() == numExamples * numLabels);

    BooleanVectorPtr booleanPredictions = predictions.dynamicCast<BooleanVector>();
    if (booleanPredictions)
    {
      std::vector<bool>::const_iterator itpred = booleanPredictions->getElements().begin();
      double* weightsPtr = weights->getValuePointer(0);
      for (size_t i = 0; i < numExamples; ++i)
      {
        bool prediction = *itpred++;
        for (size_t j = 0; j < numLabels; ++j)
        {
          bool isPredictionCorrect = (prediction == supervisions->get(i * numLabels + j));
          (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
        }
      }
    }
    else
    {
      DenseDoubleVectorPtr scalarPredictions = predictions.dynamicCast<DenseDoubleVector>();
      double* weightsPtr = weights->getValuePointer(0);
      for (size_t i = 0; i < numExamples; ++i)
      {
        double prediction = scalarPredictions->getValue(i) * 2 - 1;
        for (size_t j = 0; j < numLabels; ++j)
        {
          double sup = supervisions->get(i * numLabels + j) ? 1.0 : -1.0;
          bool isPredictionCorrect = (prediction * sup > 0);
          double k = fabs(prediction);
          (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
        }
      }
    }

    // compute v_l values
    for (size_t i = 0; i < numLabels; ++i)
      votes->setValue(i, muPositives->getValue(i) > muNegatives->getValue(i) ? 1.0 : -1.0);
  }
};

typedef ReferenceCountedObjectPtr<AdaBoostMHWeakObjective> AdaBoostMHWeakObjectivePtr;

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner)
    : WeightBoostingLearner(weakLearner) {}
  AdaBoostMHLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective() const
    {return new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights);}

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = examples.size();
    DenseDoubleVectorPtr res(new DenseDoubleVector(n * numLabels, 1.0 / (2 * n * (numLabels - 1))));
    double invZ = 1.0 / (2 * n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t k = (size_t)examples[i]->getSecond().getInteger();
      jassert(k >= 0 && k < numLabels);
      res->setValue(i * numLabels + k, invZ);
    }
    return res;
  }

  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const
  {
    EnumerationPtr labels = examples[0]->getClass()->getTemplateArgument(1).staticCast<Enumeration>();
    size_t n = examples.size();
    size_t m = labels->getNumElements();
    BooleanVectorPtr res = new BooleanVector(n * m);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      const PairPtr& example = examples[i].staticCast<Pair>();
      size_t label = (size_t)example->getSecond().getInteger();
      for (size_t j = 0; j < m; ++j, ++index)
        res->set(index, j == label);
    }
    return res;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0;}

  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const VectorPtr& predictions, const ContainerPtr& supervision, const Variable& vote) const
  {
    size_t numLabels = function.staticCast<LuapeClassifier>()->getLabels()->getNumElements();
    size_t example = index / numLabels;
    size_t k = index % numLabels;
    double alpha = vote.getObjectAndCast<DenseDoubleVector>()->getValue(k);
    double pred = getSignedScalarPrediction(predictions, index);
    double sup = supervision.staticCast<BooleanVector>()->get(index) ? 1.0 : -1.0;
    return currentWeight * exp(-alpha * pred * sup);
  }

  virtual Variable computeVote(BoostingWeakObjectivePtr edgeCalculator) const
  {
    const AdaBoostMHWeakObjectivePtr& objective = edgeCalculator.staticCast<AdaBoostMHWeakObjective>();

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
    double alpha = 0.5 * log(correctWeight / errorWeight);
    jassert(fabs(correctWeight + errorWeight - 1.0) < 1e-9 && alpha > 0.0);

    DenseDoubleVectorPtr res = votes->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(alpha);
    return res;
  }

  virtual bool doLearningIteration(ExecutionContext& context)
  {
    if (!WeightBoostingLearner::doLearningIteration(context))
      return false;
    return true;

    static int counter = 0;
    ++counter;

    if ((counter % 10) == 0)
    {
      static const size_t numIterations = 10;

      context.enterScope(T("SGD"));

      context.enterScope(T("Before"));
      context.resultCallback(T("iteration"), (size_t)0);
      context.resultCallback(T("loss"), weightsSum);
      context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainData));
      context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
      context.leaveScope();

      for (size_t i = 0; i < numIterations; ++i)
      {
        context.enterScope(T("Iteration ") + String((int)i+1));
        context.resultCallback(T("iteration"), i+1);
        doSGDIteration(context);
        recomputePredictions(context);
        recomputeWeights(context);
        context.resultCallback(T("loss"), weightsSum);
        context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainData));
        context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));

       /* applyRegularizer(context);
        recomputePredictions(context);
        recomputeWeights(context);
        context.resultCallback(T("loss 2"), weightsSum);
        context.resultCallback(T("train error 2"), function->evaluatePredictions(context, predictions, trainData));
        context.resultCallback(T("validation error 2"), function->evaluatePredictions(context, validationPredictions, validationData));*/

        context.leaveScope();
      }
      context.leaveScope();
    }
    if (counter > 100)
    {
      context.enterScope(T("Pruning"));
      size_t yieldIndex = pruneSmallestVote(context);

      recomputePredictions(context);
      recomputeWeights(context);

      context.resultCallback(T("yield index"), yieldIndex);
      context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainData));
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

        double weightSum = 0.0;
        BooleanVectorPtr predictions = graph->updateNodeCache(context, yieldNode->getArgument(), true);
        DenseDoubleVectorPtr weights = makeInitialWeights(classifier, *(const std::vector<PairPtr>* )&trainData);
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
    static const double learningRate = 0.001;// / (1.0 + sqrt((double)graph->getNumYieldNodes()));

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
      const PairPtr& example = trainData[order[i]].staticCast<Pair>();

      // compute weak predictions
      BooleanVectorPtr weakPredictions = classifier->computeBooleanWeakPredictions(context, example->getFirst().getObject());
      jassert(numVotes == weakPredictions->getNumElements());

      // compute activation
      DenseDoubleVectorPtr activations = new DenseDoubleVector(classifier->getDoubleVectorClass());
      for (size_t j = 0; j < numVotes; ++j)
      {
        double sign = weakPredictions->get(j) ? 1.0 : -1.0;
        votes->getElement(j).getObjectAndCast<DoubleVector>()->addWeightedTo(activations, 0, sign);
      }

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
        double sign = weakPredictions->get(j) ? 1.0 : -1.0;
        lossGradient->addWeightedTo(v, 0, -learningRate * sign);
      }
    }
    
    context.resultCallback(T("meanLoss"), loss.getMean());
  }

  void recomputeWeights(ExecutionContext& context)
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = trainData.size();
    weights = new DenseDoubleVector(n * numLabels, 0.0);

    double positiveWeight =  1.0 / (2 * n);
    double negativeWeight = 1.0 / (2 * n * (numLabels - 1));
    weightsSum = 0.0;

    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr activations = predictions->getElement(i).getObjectAndCast<DenseDoubleVector>();
      size_t correctLabel = (size_t)trainData[i].staticCast<Pair>()->getSecond().getInteger();
      jassert(correctLabel >= 0 && correctLabel < numLabels);
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isCorrectLabel = (j == correctLabel);
        double w0 = isCorrectLabel ? positiveWeight : negativeWeight;
        double sign = isCorrectLabel ? 1.0 : -1.0;
        double weight = w0 * exp(-sign * activations->getValue(j));
        weights->setValue(i * numLabels + j, weight);
        weightsSum += weight;
      }
    }
    weights->multiplyByScalar(1.0 / weightsSum);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_
