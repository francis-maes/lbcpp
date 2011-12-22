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
# include <lbcpp/Luape/LuapeCache.h>

namespace lbcpp
{

// FIXME: implement missings vote
class AdaBoostMHWeakObjective : public BoostingWeakObjective
{
public:
  AdaBoostMHWeakObjective(const LuapeClassifierPtr& classifier, const DenseDoubleVectorPtr& supervisions, const DenseDoubleVectorPtr& weights, bool useSymmetricVotes)
    : labels(classifier->getLabels()), doubleVectorClass(classifier->getDoubleVectorClass()), supervisions(supervisions), weights(weights), votesUpToDate(false), useSymmetricVotes(useSymmetricVotes)
  {
    jassert(supervisions->getNumValues() == weights->getNumValues());
    numLabels = labels->getNumElements();
    for (size_t i = 0; i < 3; ++i)
      for (size_t j = 0; j < 2; ++j)
      {
        mu[i][j] = new DenseDoubleVector(doubleVectorClass, numLabels);
        votes[i] = new DenseDoubleVector(doubleVectorClass, numLabels);
      }
    votesUpToDate = false;
  }

  virtual void setPredictions(const LuapeSampleVectorPtr& predictions)
  {
    this->predictions = predictions;
    computeMuAndVoteValues();
  }

  virtual void flipPrediction(size_t index)
  {
    double* weightsPtr = weights->getValuePointer(index * numLabels);
    double* muNegNegPtr = mu[0][0]->getValuePointer(0);
    double* muNegPosPtr = mu[0][1]->getValuePointer(0);
    double* muPosNegPtr = mu[1][0]->getValuePointer(0);
    double* muPosPosPtr = mu[1][1]->getValuePointer(0);
    double* supervisionsPtr = supervisions->getValuePointer(index * numLabels);
    for (size_t i = 0; i < numLabels; ++i)
    {
      double weight = *weightsPtr++;
      
      double& muNegNeg = *muNegNegPtr++;
      double& muNegPos = *muNegPosPtr++;
      double& muPosNeg = *muPosNegPtr++;
      double& muPosPos = *muPosPosPtr++;
      double supervision = *supervisionsPtr++;
      
      if (supervision < 0)
        muNegNeg -= weight, muPosNeg += weight;
      else
        muNegPos -= weight, muPosPos += weight;
    }
    votesUpToDate = false;
  }

  virtual double computeObjective() const
  {
    const_cast<AdaBoostMHWeakObjective* >(this)->ensureVotesAreComputed();
    size_t n = labels->getNumElements();
    double edge = 0.0;

    if (useSymmetricVotes)
    {
      const double* muNegNegPtr = mu[0][0]->getValuePointer(0);
      const double* muPosNegPtr = mu[1][0]->getValuePointer(0);
      const double* muNegPosPtr = mu[0][1]->getValuePointer(0);
      const double* muPosPosPtr = mu[1][1]->getValuePointer(0);
      
      for (size_t j = 0; j < n; ++j)
      {
        double muPositive = (*muNegNegPtr++) + (*muPosPosPtr++);
        double muNegative = (*muPosNegPtr++) + (*muNegPosPtr++);
        double vote = (muPositive > muNegative + 1e-9 ? 1.0 : -1.0);
        edge += vote * (muPositive - muNegative);
      }
    }
    else
    {
      for (size_t i = 0; i < n; ++i)
      {
        edge += votes[0]->getValue(i) * (mu[0][1]->getValue(i) - mu[0][0]->getValue(i))
              + votes[1]->getValue(i) * (mu[1][1]->getValue(i) - mu[1][0]->getValue(i))
              + votes[2]->getValue(i) * (mu[2][1]->getValue(i) - mu[2][0]->getValue(i));
      }
    }
    return edge;
  }
  
  const DenseDoubleVectorPtr* getVotes() const
  {
    const_cast<AdaBoostMHWeakObjective* >(this)->ensureVotesAreComputed();
    return votes;
  }
  
  const DenseDoubleVectorPtr getMu(unsigned char prediction, bool supervision) const
    {return mu[prediction][supervision ? 1 : 0];}

protected:
  EnumerationPtr labels;
  size_t numLabels;
  ClassPtr doubleVectorClass;
  LuapeSampleVectorPtr predictions;
  DenseDoubleVectorPtr supervisions;  // size = numExamples * numLabels
  DenseDoubleVectorPtr weights;   // size = numExamples * numLabels

  DenseDoubleVectorPtr mu[3][2]; // prediction -> supervision -> label -> weight
  DenseDoubleVectorPtr votes[3]; // prediction -> label -> {-1, 1}
  bool votesUpToDate;

  bool useSymmetricVotes;

  void computeMuAndVoteValues()
  {
    for (size_t i = 0; i < 3; ++i)
      for (size_t j = 0; j < 2; ++j)
        mu[i][j]->multiplyByScalar(0.0);

    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      size_t example = it.getIndex();
      unsigned char prediction = it.getRawBoolean();
      double* weightsPtr = weights->getValuePointer(numLabels * example);
      double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
      for (size_t j = 0; j < numLabels; ++j)
      {
        double supervision = *supervisionsPtr++;
        mu[prediction][supervision > 0 ? 1 : 0]->incrementValue(j, *weightsPtr++);
      }
    }
    votesUpToDate = false;
  }
  
  void ensureVotesAreComputed()
  {
    if (!votesUpToDate)
    {
      for (size_t i = 0; i < 3; ++i)
      {
        double* votesPtr = votes[i]->getValuePointer(0);
        double* muNegativesPtr = mu[i][0]->getValuePointer(0);
        double* muPositivesPtr = mu[i][1]->getValuePointer(0);
        for (size_t j = 0; j < numLabels; ++j)
          *votesPtr++ = *muPositivesPtr++ > (*muNegativesPtr++ + 1e-09) ? 1.0 : -1.0;
      }
      votesUpToDate = true;
    }
  }  
};

typedef ReferenceCountedObjectPtr<AdaBoostMHWeakObjective> AdaBoostMHWeakObjectivePtr;

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner, bool useSymmetricVotes)
    : WeightBoostingLearner(weakLearner), useSymmetricVotes(useSymmetricVotes) {}
  AdaBoostMHLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective() const
    {return new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights, useSymmetricVotes);}

//  virtual bool shouldStop(double weakObjectiveValue) const
//    {return weakObjectiveValue == 0.0;}

  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const
  {
    TypePtr supervisionType = examples[0]->getVariable(1).getType();
    EnumerationPtr labels = LuapeClassifier::getLabelsFromSupervision(supervisionType);
    size_t n = examples.size();
    size_t m = labels->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(n * m, 0.0);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = examples[i]->getVariable(1);
      size_t label;
      if (supervision.isInteger())
        label = (size_t)supervision.getInteger();
      else
        label = (size_t)supervision.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
      for (size_t j = 0; j < m; ++j, ++index)
        res->setValue(index, j == label ? 1.0 : -1.0);
    }
    return res;
  }

  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const IndexSetPtr& indices, Variable& successVote, Variable& failureVote, Variable& missingVote) const
  {
    AdaBoostMHWeakObjectivePtr objective = new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights, useSymmetricVotes);
    objective->setPredictions(trainingCache->getSamples(context, weakNode, indices));

    DenseDoubleVectorPtr res[3];
    
    if (useSymmetricVotes)
    {
      const double* muNegNegPtr = objective->getMu(0, false)->getValuePointer(0);
      const double* muPosNegPtr = objective->getMu(1, false)->getValuePointer(0);
      const double* muNegPosPtr = objective->getMu(0, true)->getValuePointer(0);
      const double* muPosPosPtr = objective->getMu(1, true)->getValuePointer(0);
      DenseDoubleVectorPtr votes = new DenseDoubleVector(function.staticCast<LuapeClassifier>()->getDoubleVectorClass());

      double correctWeight = 0.0;
      double errorWeight = 0.0;
      size_t n = votes->getNumValues();
    
      for (size_t j = 0; j < n; ++j)
      {
        double muPositive = (*muNegNegPtr++) + (*muPosPosPtr++);
        double muNegative = (*muPosNegPtr++) + (*muNegPosPtr++);
        double vote = (muPositive > muNegative + 1e-9 ? 1.0 : -1.0);
        votes->setValue(j, vote);
        if (vote > 0)
        {
          correctWeight += muPositive;
          errorWeight += muNegative;
        }
        else
        {
          correctWeight += muNegative;
          errorWeight += muPositive;
        }
      }

      if (errorWeight || correctWeight)
      {
        double alpha = errorWeight ? 0.5 * log(correctWeight / errorWeight) : 1.0; // FIXME: what should we do if everything is correct ?
        jassert(alpha > -1e-9);
        //context.resultCallback(T("alpha"), alpha);
        res[0] = votes->cloneAndCast<DenseDoubleVector>();
        res[0]->multiplyByScalar(-alpha);
        res[1] = votes->cloneAndCast<DenseDoubleVector>();
        res[1]->multiplyByScalar(alpha);
      }
    }
    else
    {
      const DenseDoubleVectorPtr* votes = objective->getVotes();
      for (size_t i = 0; i < 3; ++i)
      {
        const double* votesPtr = votes[i]->getValuePointer(0);
        const double* muNegativesPtr = objective->getMu(i, false)->getValuePointer(0);
        const double* muPositivesPtr = objective->getMu(i, true)->getValuePointer(0);

        double correctWeight = 0.0;
        double errorWeight = 0.0;
        size_t n = votes[i]->getNumValues();
      
        for (size_t j = 0; j < n; ++j)
        {
          if (*votesPtr++ > 0)
          {
            jassert(*muPositivesPtr > *muNegativesPtr + 1e-9);
            correctWeight += *muPositivesPtr++;
            errorWeight += *muNegativesPtr++;
          }
          else
          {
            jassert(*muPositivesPtr <= *muNegativesPtr + 1e-9);
            correctWeight += *muNegativesPtr++;
            errorWeight += *muPositivesPtr++;
          }
        }

        if (errorWeight || correctWeight)
        {
          double alpha = errorWeight ? 0.5 * log(correctWeight / errorWeight) : 1.0; // FIXME: what should we do if everything is correct ?
          res[i] = votes[i]->cloneAndCast<DenseDoubleVector>();
          res[i]->multiplyByScalar(alpha);
          // correctWeight + errorWeight = weight of selected examples (1 if all the examples are selected)
          jassert(alpha > -1e-9);
        }
      }
    }
    
    ClassPtr doubleVectorClass = function.staticCast<LuapeClassifier>()->getDoubleVectorClass();
    failureVote = Variable(res[0], doubleVectorClass);
    successVote = Variable(res[1], doubleVectorClass);
    missingVote = Variable(res[2], doubleVectorClass);
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
        jassert(isNumberValid(weight));
        *weightsPtr++ = weight;
        loss += weight;
      }
    }
    jassert(isNumberValid(loss));
    res->multiplyByScalar(1.0 / loss);
    return res;
  }

#if 0
  virtual bool doLearningIteration(ExecutionContext& context, double& trainingScore, double& validationScore)
  {
    if (!WeightBoostingLearner::doLearningIteration(context, trainingScore, validationScore))
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

protected:
  friend class AdaBoostMHLearnerClass;

  bool useSymmetricVotes;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_
