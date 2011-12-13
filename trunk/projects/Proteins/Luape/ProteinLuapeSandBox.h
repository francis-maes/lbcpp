/*-----------------------------------------.---------------------------------.
| Filename: ProteinLuapeSandBox.h          | Protein Luape SandBox           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
# define LBCPP_PROTEINS_LUAPE_SAND_BOX_H_

#include <lbcpp/Core/Function.h>
#include <lbcpp/Luape/LuapeInference.h>
#include "../Predictor/DecoratorProteinPredictorParameters.h"
#include "../Predictor/LargeProteinPredictorParameters.h"
#include "../Evaluator/ProteinEvaluator.h"

namespace lbcpp
{

class LuapeProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    jassert(false);
    return FunctionPtr();
  }

  virtual FunctionPtr binaryClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine = new LuapeBinaryClassifier();
    //learningMachine->setLearner(adaBoostLearner(binaryTreeWeakLearner(singleStumpWeakLearner(), singleStumpWeakLearner()), true), 100);
    return learningMachine;
  }

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine =  new LuapeClassifier();
    learningMachine->setLearner(adaBoostMHLearner(binaryTreeWeakLearner(singleStumpWeakLearner(), singleStumpWeakLearner()), true), 100);
    return learningMachine;
  }

  virtual FunctionPtr regressor(ProteinTarget target) const
  {
    jassert(false);
    return FunctionPtr();
  }
};

class ProteinLuapeSandBox : public WorkUnit
{
public:
  ProteinLuapeSandBox() : maxProteinCount(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr trainingProteins = loadProteinPairs(context, trainingInputDirectory, trainingSupervisionDirectory, "training");
    ContainerPtr testingProteins = loadProteinPairs(context, testingInputDirectory, testingSupervisionDirectory, "testing");
    if (!trainingProteins || !testingProteins)
      return false;

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(20);

#if 0
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    /*predictor->learningMachineName = T("ExtraTrees");
    predictor->x3Trees = 100;
    predictor->x3Attributes = 0;
    predictor->x3Splits = 1;*/
    predictor->learningMachineName = "kNN";
    predictor->knnNeighbors = 5;
#endif
     
    ProteinPredictorParametersPtr predictor = new LuapeProteinPredictorParameters();

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    iteration->addTarget(dsbTarget);

    if (!iteration->train(context, trainingProteins, ContainerPtr(), T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr scores = iteration->evaluate(context, testingProteins, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class ProteinLuapeSandBoxClass;

  File trainingInputDirectory;
  File trainingSupervisionDirectory;
  File testingInputDirectory;
  File testingSupervisionDirectory;
  size_t maxProteinCount;

  ContainerPtr loadProteinPairs(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, const String& description)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteinCount, T("Loading ") + description + T(" proteins"));
    context.informationCallback(String(proteins ? (int)proteins->getNumElements() : 0) + T(" ") + description + T(" proteins"));
    return proteins;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
