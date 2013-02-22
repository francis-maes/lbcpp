/*-----------------------------------------.---------------------------------.
| Filename: ProteinLuapeSandBox.h          | Protein Luape SandBox           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
# define LBCPP_PROTEINS_LUAPE_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "LuapeProteinPredictorParameters.h"
# include "../Predictor/ProteinPredictor.h"
# include "../Evaluator/ProteinEvaluator.h"
# include "../Evaluator/KolmogorovPerfectMatchingFunction.h"

namespace lbcpp
{
  
  
class ProteinLuapeSandBox : public WorkUnit
{
public:
  ProteinLuapeSandBox() : maxProteinCount(0), treeDepth(1), complexity(5), relativeBudget(10.0), miniBatchRelativeSize(0.0), numIterations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
#if JUCE_DEBUG
//    maxProteinCount = 20;
#endif // !JUCE_DEBUG
    ContainerPtr trainingProteins = loadProteinPairs(context, trainingInputDirectory, trainingSupervisionDirectory, "training");
    ContainerPtr testingProteins = loadProteinPairs(context, testingInputDirectory, testingSupervisionDirectory, "testing");
    if (!trainingProteins || !testingProteins)
      return false;

#if 0
    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(20);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    /*predictor->learningMachineName = T("ExtraTrees");
    predictor->x3Trees = 100;
    predictor->x3Attributes = 0;
    predictor->x3Splits = 1;*/
    predictor->learningMachineName = "kNN";
    predictor->knnNeighbors = 5;
#endif
     
    ProteinPredictorParametersPtr predictor = new LuapeProteinPredictorParameters(treeDepth, complexity, relativeBudget, miniBatchRelativeSize, numIterations, true);

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    DisulfideBondClassifierPtr dsbClassifier = iteration->addTarget(dsbTarget)->getVariable(0).dynamicCast<DisulfideBondClassifier>();
    if (dsbClassifier)
    {
      dsbClassifier->setProteinPairs(context, trainingProteins, true);
      dsbClassifier->setProteinPairs(context, testingProteins, false);
    }

    /*context.enterScope(T("Learn bond classifier"));
    learnClassifier(context, dsbClassifier);
    context.leaveScope();*/
    
//    iteration->addTarget(sa20Target);
//    iteration->addTarget(ss3Target);
    //iteration->addTarget(ss8Target);
    //iteration->addTarget(stalTarget);
    //iteration->addTarget(drTarget);
    
    if (!iteration->train(context, trainingProteins, testingProteins, T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = createEvaluator(true);    
    CompositeScoreObjectPtr scores = iteration->evaluate(context, trainingProteins, evaluator, T("Evaluate on training proteins"));
    double trainScore = evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();

    evaluator = createEvaluator(true);
    scores = iteration->evaluate(context, testingProteins, evaluator, T("Evaluate on test proteins"));
    double testScore = evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
    return new Pair(trainScore, testScore);
  }

  void learnClassifier(ExecutionContext& context, LuapeBinaryClassifierPtr classifier)
  {
    classifier->setRootNode(context, new LuapeScalarSumNode());
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback(T("iteration"), i+1);
      Variable res = learnClassifierIteration(context, classifier);
      context.leaveScope(res);
      context.progressCallback(new ProgressionState(i+1, numIterations, "Iterations"));
      if (res.isBoolean() && !res.getBoolean())
        break;
    }
  }

  LuapeNodePtr sampleNode(ExecutionContext& context, const std::vector<LuapeNodePtr>& nodes) const
  {
    if (nodes.empty())
      return new LuapeConstantNode(1.0);
    else
      return nodes[context.getRandomGenerator()->sampleSize(nodes.size())];
  }

  void sampleCandidates(ExecutionContext& context, LuapeBinaryClassifierPtr classifier, std::vector<LuapeNodePtr>& res) const
  {
    std::vector<LuapeNodePtr> nodes;
    LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder((size_t)relativeBudget, complexity);
    nodeBuilder->buildNodes(context, classifier, 0, nodes);

    std::vector<LuapeNodePtr> firstResidueNodes;
    firstResidueNodes.reserve(nodes.size());
    std::vector<LuapeNodePtr> secondResidueNodes;
    secondResidueNodes.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      LuapeNodePtr node = nodes[i];
      if (!node->getType()->inheritsFrom(doubleType))
        continue; // not supported yet
      String str = node->toShortString();
      if (str.indexOf(T("bond.r2")) >= 0)
        secondResidueNodes.push_back(node);
      else
        firstResidueNodes.push_back(node);
    }

    res.resize((size_t)relativeBudget);
    for (size_t i = 0; i < res.size(); ++i)
    {
      std::vector<LuapeNodePtr> nodes(2);
      nodes[0] = sampleNode(context, firstResidueNodes);
      nodes[1] = sampleNode(context, secondResidueNodes);
      LuapeFunctionPtr function;
      switch (context.getRandomGenerator()->sampleSize(4))
      {
      case 0: function = addDoubleLuapeFunction(); break;
      case 1: function = subDoubleLuapeFunction(); break;
      case 2: function = mulDoubleLuapeFunction(); break;
      case 3: function = divDoubleLuapeFunction(); break;
      }
      res[i] = new LuapeFunctionNode(function, nodes);
      //context.informationCallback(T("Candidate: ") + res[i]->toShortString());
    }
  }

  Variable learnClassifierIteration(ExecutionContext& context, LuapeBinaryClassifierPtr classifier)
  {
    double currentScore = 1.0 - classifier->evaluatePredictions(context, classifier->getTrainingPredictions(), classifier->getTrainingSupervisions());
    context.informationCallback(T("Current score: ") + String(currentScore));

    // play bandits 
    ReferenceCountedObjectPtr<WeakLearningObjective> banditObjective = new WeakLearningObjective(context, classifier);
    BanditPoolPtr pool;
    
    static size_t maxIter = 100;
    size_t iter;
    for (iter = 0; iter < maxIter; ++iter)
    {
      context.enterScope(T("Bandit Iteration ") + String((int)iter+1));

      // sample candidate nodes
      std::vector<LuapeNodePtr> candidates;
      sampleCandidates(context, classifier, candidates);
      
      // create bandits
      pool = new BanditPool(banditObjective, 1.0, true);
      pool->reserveArms(candidates.size());
      for (size_t i = 0; i < candidates.size(); ++i)
        pool->createArm(candidates[i]);

      // play bandits
      pool->play(context, 20 * candidates.size());
      pool->displayInformation(context, 25);

      std::vector< std::pair<size_t, double> > armsOrder;
      pool->getArmsOrder(armsOrder);
      double score = -armsOrder.front().second;
      context.informationCallback(T("Best arm score: ") + String(score));
      context.leaveScope(score);
      if (score > currentScore)
        break;
    }
    if (iter == maxIter)
      return false;

    // extract best node
    std::vector< std::pair<size_t, double> > armsOrder;
    pool->getArmsOrder(armsOrder);
    LuapeNodePtr bestNode = pool->getArmParameter(armsOrder.front().first).getObjectAndCast<LuapeNode>();
    WeakLearningObjective::NodeInfo bestNodeInfo = banditObjective->getNodeInfo(bestNode);
    double bestThreshold = bestNodeInfo.bestThreshold;
    double bestVote = bestNodeInfo.bestVote;
    context.informationCallback(T("Best Node: ") + bestNode->toShortString() + T(" >= ") + String(bestThreshold) + T(" => ") + String(bestVote));

    /*tuneThreshold(context, classifier, bestNode, bestThreshold, bestVote);
    tuneVote(context, classifier, bestNode, bestThreshold, bestVote);
    tuneThreshold(context, classifier, bestNode, bestThreshold, bestVote);
    tuneVote(context, classifier, bestNode, bestThreshold, bestVote);*/

    // fine tune vote and threshold
/*    context.enterScope(bestNode->toShortString());
    double bestThreshold, bestVote;
    double res = makeCurves(context, classifier, bestNode, bestThreshold, bestVote);
    context.leaveScope(res);*/

    // turn into contribution and add to model
    LuapeNodePtr boolNode = bestNode->getType() == booleanType
      ? bestNode
      : LuapeNodePtr(new LuapeFunctionNode(stumpLuapeFunction(bestThreshold), bestNode));
    LuapeNodePtr voteNode = new LuapeFunctionNode(scalarVoteLuapeFunction(bestVote), boolNode);
    context.resultCallback("contribution", voteNode->toShortString());

    LuapeSequenceNodePtr sumNode = classifier->getRootNode().staticCast<LuapeSequenceNode>();
    sumNode->pushNode(context, voteNode, classifier->getSamplesCaches());
      
    // evaluate
    double trainingScore = 1.0 - classifier->evaluatePredictions(context, classifier->getTrainingPredictions(), classifier->getTrainingSupervisions());
    context.resultCallback(T("train Qp"), trainingScore);
    double validationScore = 1.0 - classifier->evaluatePredictions(context, classifier->getValidationPredictions(), classifier->getValidationSupervisions());
    context.resultCallback(T("validation Qp"), validationScore);

    return new Pair(trainingScore, validationScore);
    /*
    for (size_t i = 0; i < candidates.size(); ++i)
    {
      LuapeNodePtr node = candidates[i];
      context.enterScope(node->toShortString());
      double res = makeCurves(context, classifier, node);
      context.leaveScope(res);
    }*/
  }

  class WeakLearningObjective : public BanditPoolObjective
  {
  public:
    WeakLearningObjective(ExecutionContext& context, LuapeBinaryClassifierPtr classifier)
      : classifier(classifier)
    {
      currentPredictions = classifier->getTrainingPredictions().staticCast<DenseDoubleVector>();
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 0.0; best = 1.0;}

    static double computeObjective(ExecutionContext& context, LuapeBinaryClassifierPtr classifier, const DenseDoubleVectorPtr& currentPredictions, const LuapeNodePtr& boolNode, double vote)
    {
      DenseDoubleVectorPtr predictions = currentPredictions->cloneAndCast<DenseDoubleVector>();
      LuapeSampleVectorPtr values = classifier->getTrainingCache()->getSamples(context, boolNode);
      for (LuapeSampleVector::const_iterator it = values->begin(); it != values->end(); ++it)
      {
        unsigned char c = it.getRawBoolean();
        if (c < 2)
          predictions->incrementValue(it.getIndex(), c == 1 ? vote : -vote);
      }
      // compute score
      return 1.0 - classifier->evaluatePredictions(context, predictions, classifier->getTrainingSupervisions());
    }

    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      RandomGeneratorPtr random = context.getRandomGenerator();
      
      LuapeNodePtr node = parameter.getObjectAndCast<LuapeNode>();
      NodeInfo& info = nodeInfos[node];

      double threshold = 0.0;
      size_t thresholdIndex = 0;

      bool useBest = false;
      double stddevMultiplier = 1.0;
      if (instanceIndex >= 4)
      {
        useBest = true;
        stddevMultiplier = 4.0 / instanceIndex;
      }

      if (node->getType() != booleanType)
      {
        // get samples
        if (!info.sortedValues)
          info.sortedValues = classifier->getTrainingCache()->getSortedDoubleValues(context, node);

        size_t n = info.sortedValues->getNumValues();
        if (n < 2 || info.sortedValues->getValue(0).second == info.sortedValues->getValue(n - 1).second)
          return 0.0; // constant value

        // sample threshold and make stump
        thresholdIndex = info.sampleThresholdIndex(random, useBest, stddevMultiplier);
        threshold = info.sortedValues->getValue(thresholdIndex).second;
        node = new LuapeFunctionNode(stumpLuapeFunction(threshold), node);
      }
    
      // sample vote
      double vote = info.sampleVote(random, useBest, stddevMultiplier);
      double score = computeObjective(context, classifier, currentPredictions, node, vote);

      info.observe(score, thresholdIndex, threshold, vote);
      return score;
    }

    struct NodeInfo
    {
      NodeInfo() : bestObjective(0.0), bestThresholdIndex(0), bestThreshold(0.0), bestVote(0.0) {}

      void observe(double objective, size_t thresholdIndex, double threshold, double vote)
      {
        if (objective > bestObjective)
        {
          bestObjective = objective;
          bestThresholdIndex = thresholdIndex;
          bestThreshold = threshold;
          bestVote = vote;
        }
      }

      size_t sampleThresholdIndex(RandomGeneratorPtr random, bool useBest, double stddevMultiplier) const
      {
        jassert(sortedValues);
        size_t n = sortedValues->getNumValues();

        int index = (int)(0.5 + random->sampleDouble(useBest ? (double)bestThresholdIndex : n / 2.0, n * stddevMultiplier / 4.0));
        if (index < 0)
          index = 0;
        else if (index >= (int)n)
          index = (int)n - 1;
        return (size_t)index;
      }

      double sampleVote(RandomGeneratorPtr random, bool useBest, double stddevMultiplier) const
        {return random->sampleDouble(useBest ? bestVote : 0.0, stddevMultiplier);}

      SparseDoubleVectorPtr sortedValues;

      double bestObjective;
      size_t bestThresholdIndex;
      double bestThreshold;
      double bestVote;

    };

    NodeInfo& getNodeInfo(const LuapeNodePtr& node)
      {return nodeInfos[node];}

  protected:
    LuapeBinaryClassifierPtr classifier;
    DenseDoubleVectorPtr currentPredictions;

    std::map<LuapeNodePtr, NodeInfo> nodeInfos;
  };

  void tuneThreshold(ExecutionContext& context, LuapeBinaryClassifierPtr classifier, LuapeNodePtr node, double& threshold, double vote)
  {
    DenseDoubleVectorPtr currentPredictions = classifier->getTrainingPredictions().staticCast<DenseDoubleVector>();

    SparseDoubleVectorPtr sortedDoubleValues = classifier->getTrainingCache()->getSortedDoubleValues(context, node);
    size_t n = sortedDoubleValues->getNumValues();
    double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
    double bestScore = -DBL_MAX;
    double bestThreshold = 0.0;
    context.enterScope(T("Tuning threshold with vote = ") + String(vote));
    for (int i = (int)n - 1; i >= 0; --i)
    {
      //size_t index = sortedDoubleValues->getValue(i).first;
      double threshold = sortedDoubleValues->getValue(i).second;

      jassert(threshold <= previousThreshold);
      if (threshold < previousThreshold)
      {
        context.enterScope(T("Threshold ") + String(threshold));
        context.resultCallback(T("threshold"), threshold);
        context.resultCallback(T("thresholdIndex"), (size_t)i);

        LuapeNodePtr stumpNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), node);

        double trainQp = WeakLearningObjective::computeObjective(context, classifier, currentPredictions, stumpNode, vote);
        if (trainQp > bestScore)
        {
          bestScore = trainQp;
          bestThreshold = threshold;
        }
        context.resultCallback(T("trainQp"), trainQp);
        context.leaveScope();
        previousThreshold = threshold;
      }
    }
    context.informationCallback("Best threshold: " + String(bestThreshold));
    context.leaveScope(bestScore);
    threshold = bestThreshold;
  }

  void tuneVote(ExecutionContext& context, LuapeBinaryClassifierPtr classifier, LuapeNodePtr node, double threshold, double& vote)
  {
    DenseDoubleVectorPtr currentPredictions = classifier->getTrainingPredictions().staticCast<DenseDoubleVector>();
    LuapeNodePtr stumpNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), node);

    double bestScore = -DBL_MAX;
    std::vector<double> bestVotes;
    context.enterScope(T("Tuning vote with threshold = ") + String(threshold));
    for (vote = -1.0; vote <= 1.0; vote += 0.01)
    {
      context.enterScope(T("Vote ") + String(vote));
      context.resultCallback(T("vote"), vote);

      //double bestScoreGivenThreshold = -DBL_MAX;
      double trainQp = WeakLearningObjective::computeObjective(context, classifier, currentPredictions, stumpNode, vote);
      if (trainQp >= bestScore)
      {
        if (trainQp > bestScore)
          bestVotes.clear();
        bestScore = trainQp;
        bestVotes.push_back(vote);
      }
      context.resultCallback(T("trainQp"), trainQp);
      context.leaveScope();
    }
    double bestVote = bestVotes[bestVotes.size() / 2];
    context.informationCallback("Best vote: " + String(bestVote));
    context.leaveScope(bestScore);
    vote = bestVote;
  }

  double makeCurves(ExecutionContext& context, LuapeBinaryClassifierPtr classifier, LuapeNodePtr node, double& bestThreshold, double& bestVote)
  {
    DenseDoubleVectorPtr currentPredictions = classifier->getTrainingPredictions().staticCast<DenseDoubleVector>();

    SparseDoubleVectorPtr sortedDoubleValues = classifier->getTrainingCache()->getSortedDoubleValues(context, node);
    size_t n = sortedDoubleValues->getNumValues();
    double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
    double bestScore = -DBL_MAX;
    for (int i = (int)n - 1; i >= 0; --i)
    {
      //size_t index = sortedDoubleValues->getValue(i).first;
      double threshold = sortedDoubleValues->getValue(i).second;

      jassert(threshold <= previousThreshold);
      if (threshold < previousThreshold)
      {
        context.enterScope(T("Threshold ") + String(threshold));
        context.resultCallback(T("threshold"), threshold);

        LuapeNodePtr stumpNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), node);

        double bestScoreGivenThreshold = -DBL_MAX;
        double incr = classifier->getRootNode()->getNumSubNodes() ? 0.2 : 2.0;
        for (double v = -1.0; v <= 1.0; v += incr)
        {
          double trainQp = WeakLearningObjective::computeObjective(context, classifier, currentPredictions, stumpNode, v);
          
          bestScoreGivenThreshold = juce::jmax(bestScoreGivenThreshold, trainQp);
          if (trainQp > bestScore)
          {
            bestScore = trainQp;
            bestThreshold = threshold;
            bestVote = v;
          }
          context.resultCallback(T("trainQp") + String(v), trainQp);
        }
        context.leaveScope(bestScoreGivenThreshold);
        previousThreshold = threshold;
      }
    }
    return bestScore;
  }

protected:
  friend class ProteinLuapeSandBoxClass;

  File trainingInputDirectory;
  File trainingSupervisionDirectory;
  File testingInputDirectory;
  File testingSupervisionDirectory;
  size_t maxProteinCount;

  size_t treeDepth;
  size_t complexity;
  double relativeBudget;
  double miniBatchRelativeSize;
  size_t numIterations;

  ContainerPtr loadProteinPairs(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, const String& description)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteinCount, T("Loading ") + description + T(" proteins"));
    //context.informationCallback(String(proteins ? (int)proteins->getNumElements() : 0) + T(" ") + description + T(" proteins"));
    return proteins;
  }

  ProteinEvaluatorPtr createEvaluator(bool isFinalEvaluation) const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("DSB QP Perfect"), true);
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("DSB Q2"));
    
    //evaluator->addEvaluator(ss3Target, elementContainerSupervisedEvaluator(classificationEvaluator()), T("Secondary Structure"));
/*    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationCurveEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation), 1), T("Disulfide Bonds (Sens. and Spec)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationCurveEvaluator(binaryClassificationMCCScore, isFinalEvaluation), 1), T("Disulfide Bonds (MCC)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore), 1), T("Disulfide Bonds (Raw)"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0), T("Disulfide Bonds (Greedy L=6)"));    */
    
    return evaluator;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
