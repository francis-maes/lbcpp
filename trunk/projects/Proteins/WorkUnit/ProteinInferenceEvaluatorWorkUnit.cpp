/*-----------------------------------------.---------------------------------.
 | Filename: ProteinInferenceEvaluat...cpp  | ProteinInferenceEvaluator       |
 | Author  : Julien Becker                  | WorkUnit                        |
 | Started : 25/11/2010 11:20               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#include "ProteinInferenceEvaluatorWorkUnit.h"
#include "../Data/Protein.h"

using namespace lbcpp;

// Variable -> File
class SaveToFileFunction : public Function
{
public:
  SaveToFileFunction(const File& directory = File::getCurrentWorkingDirectory()) : directory(directory) {}
  
  virtual TypePtr getInputType() const
    {return anyType;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (input.isObject())
    {
      ObjectPtr obj = input.getObject();
      jassert(obj);
      std::cout << "Saving: " << directory.getChildFile(obj->getName() + T(".xml")).getFullPathName() << std::endl;
      obj->saveToFile(context, directory.getChildFile(obj->getName() + T(".xml")));
    }
    else
      jassert(false);
    return input;
  }
  
protected:
  File directory;
};

// input -> predicted output
class PredictFunction : public Function
{
public:
  PredictFunction(InferencePtr inference)
    : inference(inference) {}
  
  virtual TypePtr getInputType() const
    {return inference->getInputType();}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inference->getOutputType(inputType);}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return inference->computeFunction(context, input);}
  
protected:
  InferencePtr inference;
};

// Protein -> Protein (with input data only)
class InputProteinFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass;}
  
  virtual TypePtr getOutputType(TypePtr ) const
    {return proteinClass;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    
    return inputProtein;
  }
};

bool ProteinInferenceEvaluatorWorkUnit::run(ExecutionContext& context)
{
  if (!inputDirectory.exists()
      || !outputDirectory.exists()
      || !inferenceFile.exists())
  {
    context.errorCallback(T("ProteinInferenceEvaluatorWorkUnit::run"), getUsageString());
    return false;
  }

  InferencePtr inference = Inference::createFromFile(context, inferenceFile);
  if (!inference)
  {
    context.errorCallback(T("ProteinInferenceEvaluatorWorkUnit::run"), T("Sorry, the inference file is not correct !"));
    return false;
  }

  ContainerPtr data = directoryFileStream(context, inputDirectory, T("*.xml"))
      ->load()
      ->apply(context, loadFromFileFunction(proteinClass), Container::parallelApply)
      ->apply(context, FunctionPtr(new InputProteinFunction()), Container::parallelApply);
  std::cout << "Data         : " << data->getNumElements() << std::endl;

  data->apply(context, FunctionPtr(new PredictFunction(inference)))
      ->apply(context, FunctionPtr(new SaveToFileFunction(outputDirectory)), Container::parallelApply);

  return true;
}
