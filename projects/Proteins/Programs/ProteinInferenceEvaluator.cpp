/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceEvaluator.cpp  | Evaluator                       |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
//#include "../../../../src/Inference/InferenceContext/CancelAfterStepCallback.h"
#include "../../../../src/Inference/InferenceCallback/CacheInferenceCallback.h"
#include "Protein/Evaluation/ProteinEvaluationCallback.h"
#include "Protein/Inference/ProteinInference.h"
using namespace lbcpp;

extern void declareProteinClasses();

ObjectContainerPtr loadProteins(const File& fileOrDirectory, size_t maxCount = 0)
{
  if (fileOrDirectory.isDirectory())
  {
    ObjectStreamPtr proteinsStream = directoryObjectStream(fileOrDirectory, T("*.protein"));
    ObjectContainerPtr res = proteinsStream->load(maxCount)->randomize();
    for (size_t i = 0; i < res->size(); ++i)
      res->getAndCast<Protein>(i)->computeMissingFields();
    return res;
  }
  else
  {
    ProteinPtr protein = Protein::createFromFile(fileOrDirectory);
    if (!protein)
      return ObjectContainerPtr();
    protein->computeMissingFields();
    VectorObjectContainerPtr voc = new VectorObjectContainer();
    voc->append(protein);
    return voc;
  }
}

InferenceStepPtr addBreakToInference(InferenceStepPtr inference, InferenceStepPtr lastStepBeforeBreak)
  {return new CallbackBasedDecoratorInferenceStep(inference->getName() + T(" breaked"), inference, cancelAfterStepCallback(lastStepBeforeBreak));}

class SaveOutputInferenceCallback : public InferenceCallback
{
public:
  SaveOutputInferenceCallback(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      File f = directory.getChildFile(output->getName() + T(".") + extension);
      std::cout << "Save " << f.getFileName() << "." << std::endl;
      output->saveToFile(f);
    }
  }

private:
  File directory;
  String extension;
};

class PrintDotForEachExampleInferenceCallback : public InferenceCallback
{
public:
  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
      std::cout << "." << std::flush;
  }
};


int main(int argc, char** argv)
{
  declareProteinClasses();

  if (argc < 4)
  {
    std::cerr << "Usage: " << argv[0] << " modelDirectory proteinFileOrDirectory mode" << std::endl;
    std::cerr << "Possible values for 'mode': All StepByStep or AllSave" << std::endl;
    return 1;
  }

  File cwd = File::getCurrentWorkingDirectory();
  File modelDirectory = cwd.getChildFile(argv[1]);
  File proteinsFileOrDirectory = cwd.getChildFile(argv[2]);
  String mode = argv[3];

  if (!proteinsFileOrDirectory.exists())
  {
    std::cerr << proteinsFileOrDirectory.getFullPathName() << " does not exists." << std::endl;
    return 1;
  }

  std::cout << "Loading data... " << std::flush;
  ObjectContainerPtr proteins = loadProteins(proteinsFileOrDirectory);
  if (!proteins)
    return 2;
  std::cout << proteins->size() << " protein(s)." << std::endl;

  std::cout << "Loading inference... " << std::flush;
  ProteinInferencePtr inference = new ProteinInference();
  inference->loadSubInferencesFromDirectory(modelDirectory);
  if (!inference->getNumSubSteps())
  {
    std::cerr << "Could not find any inference step in directory " << modelDirectory.getFullPathName() << std::endl;
    return 3;
  }
  std::cout << inference->getNumSubSteps() << " step(s)." << std::endl;

  InferenceContextPtr inferenceContext = singleThreadedInferenceContext();
  ProteinEvaluationCallbackPtr evaluationCallback = new ProteinEvaluationCallback();
  inferenceContext->appendCallback(evaluationCallback);
  inferenceContext->appendCallback(new PrintDotForEachExampleInferenceCallback());
  InferenceResultCachePtr cache = new InferenceResultCache();

  if (mode == T("All") || mode == T("AllSave"))
  {
    if (mode == T("AllSave"))
      inferenceContext->appendCallback(new SaveOutputInferenceCallback(cwd.getChildFile(T("proteins")), T("protein")));
    std::cout << "Making predictions..." << std::endl;
    inferenceContext->runWithSelfSupervisedExamples(inference, proteins);
    std::cout << evaluationCallback->toString() << std::endl << std::endl;
  }
  else if (mode == T("StepByStep"))
  {
    inferenceContext->appendCallback(cacheInferenceCallback(cache, inference));
    std::cout << std::endl;
    for (size_t i = 0; i < inference->getNumSubSteps(); ++i)
    {
      InferenceStepPtr decoratedInference;
      if (i < inference->getNumSubSteps() - 1)
      {
        std::cout << "Making predictions for steps 1.." << (i+1) << std::endl;
        decoratedInference = addBreakToInference(inference, inference->getSubStep(i));
      }
      else
      {
        std::cout << "Making predictions for all steps" << std::endl;
        decoratedInference = inference;
      }
      
      inferenceContext->runWithSelfSupervisedExamples(decoratedInference, proteins);
      std::cout << evaluationCallback->toString() << std::endl << std::endl;
    }
  }
  else if (mode == T("AllSave"))
  {
    std::cout << "Making predictions..." << std::endl;
    inferenceContext->runWithSelfSupervisedExamples(inference, proteins);
    std::cout << evaluationCallback->toString() << std::endl << std::endl;
    
  }
  else
  {
    std::cerr << "Unrecognized mode: " << mode.quoted() << std::endl;
    return 1;
  }
  return 0;
}
