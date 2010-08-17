/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceEvaluator.cpp  | Evaluator                       |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#if 0
#include "Protein/Evaluation/ProteinEvaluationCallback.h"
#include "Protein/Inference/ProteinInference.h"
using namespace lbcpp;

extern void declareProteinClasses();

ObjectContainerPtr loadProteins(const File& fileOrDirectory, size_t maxCount = 0)
{
  if (fileOrDirectory.isDirectory())
  {
    ObjectContainerPtr res = directoryObjectStream(fileOrDirectory, T("*.protein"))
      ->load(maxCount)
      ->randomize()
      ->apply(new ProteinToInputOutputPairFunction());

    ObjectContainerPtr resPair = directoryObjectStream(fileOrDirectory, T("*.proteinPair"))
      ->load(maxCount - res->size())
      ->randomize()
      ->apply(new ComputeMissingFieldsOfProteinPairFunction());

    res = append(res, resPair);

    return res;
  }

  if (fileOrDirectory.getFileExtension() == T(".protein")) 
  {
    ObjectContainerPtr res = directoryObjectStream(fileOrDirectory.getParentDirectory(), fileOrDirectory.getFileName())
    ->load()
    ->apply(new ProteinToInputOutputPairFunction());
    if (res->size())
      return res;
  }
  
  if (fileOrDirectory.getFileExtension() == T(".proteinPair"))
  {
    ObjectContainerPtr res = directoryObjectStream(fileOrDirectory.getParentDirectory(), fileOrDirectory.getFileName())
    ->load()
    ->apply(new ComputeMissingFieldsOfProteinPairFunction());
    if (res->size())
      return res;
  }
  return ObjectContainerPtr();
}

InferencePtr addBreakToInference(InferencePtr inference, InferencePtr lastStepBeforeBreak)
  {return callbackBasedDecoratorInference(inference->getName() + T(" breaked"), inference, cancelAfterStepCallback(lastStepBeforeBreak));}

class SaveOutputInferenceCallback : public InferenceCallback
{
public:
  SaveOutputInferenceCallback(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 2)
    {
      ObjectPtr object = output.getObject();
      File f = directory.getChildFile(object->getName() + T(".") + extension);
      std::cout << "Save " << f.getFileName() << "." << std::endl;
      object->saveToFile(f);
    }
  }

private:
  File directory;
  String extension;
};

class SaveProteinPairInferenceCallback : public InferenceCallback
{
public:
  SaveProteinPairInferenceCallback(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 2)
    {
      ObjectPtr object = output.getObject();
      File f = directory.getChildFile(object->getName() + T(".") + extension);
      std::cout << "Save Pair " << f.getFileName() << "." << std::endl;

      ProteinObjectPtr inputProtein = input.dynamicCast<ProteinObject>()->clone();
      jassert(inputProtein);

      ProteinObjectPtr outputProtein = object.dynamicCast<ProteinObject>();
      jassert(outputProtein);
      

      std::vector<String> keys = outputProtein->getKeys();
      for (size_t i = 0; i < keys.size(); ++i)
        inputProtein->setObject(outputProtein->getObject(keys[i]));

      inputProtein->computeMissingFields();
      
      ProteinObjectPtr supervisionProtein = supervision.dynamicCast<ProteinObject>();
      jassert(supervisionProtein);
      supervisionProtein->computeMissingFields();

      ObjectPairPtr(new ObjectPair(inputProtein, supervision))->saveToFile(f);
    }
  }

private:
  File directory;
  String extension;
};

class PrintDotForEachExampleInferenceCallback : public InferenceCallback
{
public:
  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
      std::cout << "." << std::flush;
  }
};

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  if (argc < 4)
  {
    std::cerr << "Usage: " << argv[0] << " modelDirectory proteinFileOrDirectory mode [param]" << std::endl;
    std::cerr << "Possible values for 'mode': All StepByStep AllSave or AllSavePair" << std::endl;
    std::cerr << "Param is only for mode AllSave and AllSavePair and is the output directory" << std::endl;
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

  File outputDirectory;
  if (mode == T("AllSave") || mode == T("AllSavePair"))
  {
    if (argc > 4)
      outputDirectory = cwd.getChildFile(argv[4]);
    else
    {
      std::cerr << "Missing 'output directory' argument" << std::endl;
      return 1;
    }
    if (!outputDirectory.exists() && !outputDirectory.createDirectory())
    {
      std::cerr << "Could not create output directory " << outputDirectory.getFullPathName() << std::endl;
      return 1;
    }
  }

  std::cout << "Loading data... " << std::flush;
  ObjectContainerPtr proteins = loadProteins(proteinsFileOrDirectory);
  if (!proteins)
    return 2;
  std::cout << proteins->size() << " protein(s)." << std::endl;

  std::cout << "Loading inference... " << std::flush;
  ProteinInferencePtr inference = new ProteinInference();
 jassert(false); // FIXME
  //inference->loadSubInferencesFromDirectory(modelDirectory);
  if (!inference->getNumSubInferences())
  {
    std::cerr << "Could not find any inference step in directory " << modelDirectory.getFullPathName() << std::endl;
    return 3;
  }
  std::cout << inference->getNumSubInferences() << " step(s)." << std::endl;

  InferenceContextPtr inferenceContext = singleThreadedInferenceContext();
  ProteinEvaluationCallbackPtr evaluationCallback = new ProteinEvaluationCallback();
  inferenceContext->appendCallback(evaluationCallback);
  inferenceContext->appendCallback(new PrintDotForEachExampleInferenceCallback());
  InferenceResultCachePtr cache = new InferenceResultCache();

  if (mode == T("All") || mode == T("AllSave") || mode == T("AllSavePair"))
  {
    if (mode == T("AllSave"))
      inferenceContext->appendCallback(new SaveOutputInferenceCallback(outputDirectory, T("protein")));
    if (mode == T("AllSavePair"))
      inferenceContext->appendCallback(new SaveProteinPairInferenceCallback(outputDirectory, T("proteinPair")));
    std::cout << "Making predictions..." << std::endl;

    Inference::ReturnCode returnCode = Inference::finishedReturnCode;
    inferenceContext->runInference(runOnSupervisedExamplesInference(inference), proteins, ObjectPtr(), returnCode);
    std::cout << evaluationCallback->toString() << std::endl << std::endl;
  }
  else if (mode == T("StepByStep"))
  {
    inferenceContext->appendCallback(cacheInferenceCallback(cache, inference));
    std::cout << std::endl;
    for (size_t i = 0; i < inference->getNumSubInferences(); ++i)
    {
      InferencePtr decoratedInference;
      if (i < inference->getNumSubInferences() - 1)
      {
        std::cout << "Making predictions for steps 1.." << (i+1) << std::endl;
        decoratedInference = addBreakToInference(inference, inference->getSubInference(i));
      }
      else
      {
        std::cout << "Making predictions for all steps" << std::endl;
        decoratedInference = inference;
      }
      
      Inference::ReturnCode returnCode = Inference::finishedReturnCode;
      inferenceContext->runInference(runOnSupervisedExamplesInference(decoratedInference), proteins, ObjectPtr(), returnCode);
      std::cout << evaluationCallback->toString() << std::endl << std::endl;
    }
  }
  else
  {
    std::cerr << "Unrecognized mode: " << mode.quoted() << std::endl;
    return 1;
  }
  return 0;
}

#endif // 0
int main() {return 1;}
