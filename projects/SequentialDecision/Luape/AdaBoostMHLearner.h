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
  AdaBoostMHWeakObjective(const LuapeClassifierPtr& classifier, const BooleanVectorPtr& supervisions, const DenseDoubleVectorPtr& weights, const std::vector<size_t>& examples)
    : labels(classifier->getLabels()), doubleVectorClass(classifier->getDoubleVectorClass()), supervisions(supervisions), weights(weights), examples(examples)
  {
    jassert(examples.size());
  }

  virtual void setPredictions(const VectorPtr& predictions)
  {
    this->predictions = predictions;
    computeMuAndVoteValues();
  }

  bool hasExample(size_t index) const
  {
    if (examples.size() == predictions->getNumElements())
      return true;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      if (examples[i] == index)
        return true;
      else if (examples[i] > index)
        return false;
    }
    return false;
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(predictions.isInstanceOf<BooleanVector>());
    bool newPrediction = predictions.staticCast<BooleanVector>()->flip(index);
    if (hasExample(index))
    {
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
  const std::vector<size_t>& examples; // size = numExamples

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
      for (size_t i = 0; i < examples.size(); ++i)
      {
        size_t example = examples[i];
        bool prediction = booleanPredictions->get(example);
        double* weightsPtr = weights->getValuePointer(numLabels * example);
        for (size_t j = 0; j < numLabels; ++j)
        {
          bool isPredictionCorrect = (prediction == supervisions->get(example * numLabels + j));
          (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
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
        for (size_t j = 0; j < numLabels; ++j)
        {
          double sup = supervisions->get(example * numLabels + j) ? 1.0 : -1.0;
          bool isPredictionCorrect = (prediction * sup > 0);
          //double k = fabs(prediction);
          (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
        }
      }
    }
    //jassert(muPositives->l1norm() > 0 || muNegatives->l1norm() > 0);
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

  virtual BoostingWeakObjectivePtr createWeakObjective(const std::vector<size_t>& examples) const
    {return new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights, examples);}

//  virtual bool shouldStop(double weakObjectiveValue) const
//    {return weakObjectiveValue == 0.0;}

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
    DenseDoubleVectorPtr res = new DenseDoubleVector(n * numLabels, 0.0);

    double positiveWeight =  1.0 / (2 * n);
    double negativeWeight = 1.0 / (2 * n * (numLabels - 1));
    loss = 0.0;

    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr activations = predictions->getElement(i).getObjectAndCast<DenseDoubleVector>();
      size_t correctLabel = (size_t)trainingData[i].staticCast<Pair>()->getSecond().getInteger();
      jassert(correctLabel >= 0 && correctLabel < numLabels);
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isCorrectLabel = (j == correctLabel);
        double w0 = isCorrectLabel ? positiveWeight : negativeWeight;
        double sign = isCorrectLabel ? 1.0 : -1.0;
        double weight = w0 * exp(-sign * activations->getValue(j));
        res->setValue(i * numLabels + j, weight);
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
