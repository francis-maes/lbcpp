/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceEvaluator.cpp  | Evaluator                       |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>

#include "Inference/ProteinInference.h"
using namespace lbcpp;

extern void declareProteinClasses(ExecutionContext& context);

ContainerPtr loadProteins(ExecutionContext& context, const File& fileOrDirectory, size_t maxCount = 0)
{
  if (fileOrDirectory.isDirectory())
  {
    ContainerPtr res = directoryFileStream(fileOrDirectory, T("*.xml"))
      ->load(context, maxCount)
      ->randomize()
      ->apply(context, proteinToInputOutputPairFunction());

    return res;
  }

  if (fileOrDirectory.getFileExtension() == T(".xml")) 
  {
    ContainerPtr res = directoryFileStream(fileOrDirectory.getParentDirectory(), fileOrDirectory.getFileName())
    ->load(context)
    ->apply(context, proteinToInputOutputPairFunction());
    if (res->getNumElements())
      return res;
  }

  return ContainerPtr();
}

InferencePtr addBreakToInference(InferencePtr inference, InferencePtr lastStepBeforeBreak)
  {return callbackBasedDecoratorInference(inference->getName() + T(" breaked"), inference, cancelAfterStepCallback(lastStepBeforeBreak));}

class SaveOutputInferenceCallback : public InferenceCallback
{
public:
  SaveOutputInferenceCallback(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}

  virtual void postInferenceCallback(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 2)
    {
      ObjectPtr object = output.getObject();
      File f = directory.getChildFile(object->getName() + T(".") + extension);
      std::cout << "Save " << f.getFileName() << "." << std::endl;
      object->saveToFile(context, f);
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

  virtual void postInferenceCallback(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 2)
    {
      ObjectPtr object = output.getObject();
      File f = directory.getChildFile(object->getName() + T(".") + extension);
      context.informationCallback(T("Save Pair ") + f.getFileName() + T("."));

      ProteinPtr inputProtein = input.clone(context).getObjectAndCast<Protein>();
      jassert(inputProtein);

      const ProteinPtr& outputProtein = output.getObjectAndCast<Protein>();
      jassert(outputProtein);
      
      for (size_t i = 0; i < outputProtein->getNumVariables(); ++i)
        inputProtein->setVariable(context, i, outputProtein->getVariable(i));

      Variable::pair(inputProtein, supervision).saveToFile(context, f);
    }
  }

private:
  File directory;
  String extension;
};

class PrintDotForEachExampleInferenceCallback : public InferenceCallback
{
public:
  virtual void postInferenceCallback(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
      std::cout << "." << std::flush;
  }
};

int main(int argc, char** argv)
{
  lbcpp::initialize();
  InferenceContextPtr context = singleThreadedInferenceContext();
  declareProteinClasses(*context);

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
  ContainerPtr proteins = loadProteins(*context, proteinsFileOrDirectory);
  if (!proteins)
    return 2;
  std::cout << proteins->getNumElements() << " protein(s)." << std::endl;

  std::cout << "Loading inference... " << std::flush;
  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  jassert(false); // FIXME
  //inference->loadSubInferencesFromDirectory(modelDirectory);
  if (!inference->getNumSubInferences())
  {
    std::cerr << "Could not find any inference step in directory " << modelDirectory.getFullPathName() << std::endl;
    return 3;
  }
  std::cout << inference->getNumSubInferences() << " step(s)." << std::endl;

  // FIXME
  //ProteinEvaluationCallbackPtr evaluationCallback = new ProteinEvaluationCallback();
  //inferenceContext->appendCallback(evaluationCallback);
  context->appendCallback(new PrintDotForEachExampleInferenceCallback());
  InferenceResultCachePtr cache = new InferenceResultCache();

  if (mode == T("All") || mode == T("AllSave") || mode == T("AllSavePair"))
  {
    if (mode == T("AllSave"))
      context->appendCallback(new SaveOutputInferenceCallback(outputDirectory, T("protein")));
    if (mode == T("AllSavePair"))
      context->appendCallback(new SaveProteinPairInferenceCallback(outputDirectory, T("proteinPair")));
    std::cout << "Making predictions..." << std::endl;

    Inference::ReturnCode returnCode = Inference::finishedReturnCode;
    context->run(runOnSupervisedExamplesInference(inference, false), proteins, ObjectPtr(), returnCode);
    // FIXME
    //std::cout << evaluationCallback->toString() << std::endl << std::endl;
  }
  else if (mode == T("StepByStep"))
  {
    context->appendCallback(cacheInferenceCallback(cache, inference));
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
      context->run(runOnSupervisedExamplesInference(decoratedInference, false), proteins, ObjectPtr(), returnCode);
      // FIXME
      //std::cout << evaluationCallback->toString() << std::endl << std::endl;
    }
  }
  else
  {
    std::cerr << "Unrecognized mode: " << mode.quoted() << std::endl;
    return 1;
  }
  return 0;
}
