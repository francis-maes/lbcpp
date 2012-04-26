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
    DisulfideBondClassifierPtr dsbClassifier = iteration->addTarget(dsbTarget)->getVariable(0).getObjectAndCast<DisulfideBondClassifier>();
    dsbClassifier->setProteinPairs(context, trainingProteins, true);
    dsbClassifier->setProteinPairs(context, testingProteins, false);

    context.enterScope(T("Learn bond classifier"));
    learnClassifier(context, dsbClassifier);
    context.leaveScope();
    
//    iteration->addTarget(sa20Target);
//    iteration->addTarget(ss3Target);
    //iteration->addTarget(ss8Target);
    //iteration->addTarget(stalTarget);
    //iteration->addTarget(drTarget);
    
    //if (!iteration->train(context, trainingProteins, testingProteins, T("Training")))
    //  return Variable::missingValue(doubleType);

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
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      learnClassifierIteration(context, classifier);
      context.progressCallback(new ProgressionState(i+1, numIterations, "Iterations"));
      context.leaveScope();
      context.resultCallback(T("iteration"), i+1);
    }
  }

  void learnClassifierIteration(ExecutionContext& context, LuapeBinaryClassifierPtr classifier)
  {
    LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder((size_t)relativeBudget, complexity);
    
    std::vector<LuapeNodePtr> candidates;
    nodeBuilder->buildNodes(context, classifier, 0, candidates);

    for (size_t i = 0; i < candidates.size(); ++i)
    {
      LuapeNodePtr node = candidates[i];
      context.enterScope(node->toShortString());
      double res = makeCurves(context, classifier, node);
      context.leaveScope(res);
    }
  }

  double makeCurves(ExecutionContext& context, LuapeBinaryClassifierPtr classifier, LuapeNodePtr node)
  {
    SparseDoubleVectorPtr sortedDoubleValues = classifier->getTrainingCache()->getSortedDoubleValues(context, node);
    size_t n = sortedDoubleValues->getNumValues();
    double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
    double bestScore = DBL_MAX;
    for (int i = (int)n - 1; i >= 0; --i)
    {
      size_t index = sortedDoubleValues->getValue(i).first;
      double threshold = sortedDoubleValues->getValue(i).second;

      jassert(threshold <= previousThreshold);
      if (threshold < previousThreshold)
      {
        context.enterScope(T("Threshold ") + String(threshold));
        context.resultCallback(T("threshold"), threshold);

        LuapeNodePtr stumpNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), node);

        double bestScoreGivenThreshold = DBL_MAX;
        for (double v = -1.0; v <= 1.0; v += 2.0)
        {
          LuapeNodePtr voteNode = new LuapeFunctionNode(scalarVoteLuapeFunction(v), stumpNode);
          VectorPtr trainPredictions = classifier->getTrainingCache()->getSamples(context, voteNode)->getVector();
          double trainError = classifier->evaluatePredictions(context, trainPredictions, classifier->getTrainingSupervisions());
          bestScore = juce::jmin(bestScore, trainError);
          bestScoreGivenThreshold = juce::jmin(bestScoreGivenThreshold, trainError);
          //VectorPtr validationPredictions = classifier->getValidationCache()->getSamples(context, voteNode)->getVector();
          //double testError = classifier->evaluatePredictions(context, validationPredictions, classifier->getValidationSupervisions());
          context.resultCallback(T("trainError") + String(v), trainError);
          //context.resultCallback(T("testError") + String(v), testError);
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
    
    //evaluator->addEvaluator(ss3Target, containerSupervisedEvaluator(classificationEvaluator()), T("Secondary Structure"));
/*    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation), 1), T("Disulfide Bonds (Sens. and Spec)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore, isFinalEvaluation), 1), T("Disulfide Bonds (MCC)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore), 1), T("Disulfide Bonds (Raw)"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0), T("Disulfide Bonds (Greedy L=6)"));    */
    
    return evaluator;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
