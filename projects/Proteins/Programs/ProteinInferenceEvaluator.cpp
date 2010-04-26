/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceEvaluator.cpp  | Evaluator                       |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "InferenceLearner/InferenceLearner.h"
#include "InferenceLearner/InferenceLearnerCallback.h"
#include "InferenceStep/DecoratorInferenceStep.h"
#include "InferenceContext/CancelAfterStepCallback.h"
#include "Protein/Evaluation/ProteinEvaluationCallback.h"
#include "Protein/Inference/ProteinInference.h"
using namespace lbcpp;

extern void declareProteinClasses();

ObjectContainerPtr loadProteins(const File& directory, size_t maxCount = 0)
{
  ObjectStreamPtr proteinsStream = directoryObjectStream(directory, T("*.protein"));
  ObjectContainerPtr res = proteinsStream->load(maxCount)->randomize();
  for (size_t i = 0; i < res->size(); ++i)
    res->getAndCast<Protein>(i)->computeMissingFields();
  return res;
}

InferenceStepPtr addBreakToInference(InferenceStepPtr inference, InferenceStepPtr lastStepBeforeBreak)
  {return new CallbackBasedDecoratorInferenceStep(inference->getName() + T(" breaked"), inference, new CancelAfterStepCallback(lastStepBeforeBreak));}

int main(int argc, char** argv)
{
  declareProteinClasses();

  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " modelDirectory proteinsDirectory" << std::endl;
    return 1;
  }

  File cwd = File::getCurrentWorkingDirectory();
  File modelDirectory = cwd.getChildFile(argv[1]);
  File proteinsDirectory = cwd.getChildFile(argv[2]);

  ObjectContainerPtr proteins = loadProteins(proteinsDirectory);
  if (!proteins)
    return 2;
  std::cout << proteins->size() << " proteins." << std::endl;

  ProteinInferencePtr inference = new ProteinInference();
  inference->loadSubInferencesFromDirectory(modelDirectory);
  if (!inference->getNumSubSteps())
  {
    std::cerr << "Could not find any inference step in directory " << modelDirectory.getFullPathName() << std::endl;
    return 3;
  }
  std::cout << inference->getNumSubSteps() << " inference steps." << std::endl;

  InferenceContextPtr inferenceContext = singleThreadedInferenceContext();
  ProteinEvaluationCallbackPtr evaluationCallback = new ProteinEvaluationCallback();
  inferenceContext->appendCallback(evaluationCallback);

  std::cout << std::endl;
  for (size_t i = 0; i < inference->getNumSubSteps(); ++i)
  {
    InferenceStepPtr decoratedInference;
    if (i < inference->getNumSubSteps() - 1)
    {
      std::cout << "Steps 1.." << (i+1) << std::endl;
      decoratedInference = addBreakToInference(inference, inference->getSubStep(i));
    }
    else
    {
      std::cout << "All Steps" << std::endl;
      decoratedInference = inference;
    }
    
    inferenceContext->runWithSelfSupervisedExamples(decoratedInference, proteins);
    std::cout << evaluationCallback->toString() << std::endl << std::endl;
  }
  return 0;
}
