
#include <lbcpp/lbcpp.h>

namespace lbcpp {

// Variable -> File
class SaveToFileFunction : public Function
{
public:
  SaveToFileFunction(const File& directory = File::getCurrentWorkingDirectory()) : directory(directory) {}

  virtual TypePtr getInputType() const
    {return anyType;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    if (input.isObject())
    {
      ObjectPtr obj = input.getObject();
      jassert(obj);
      std::cout << "Saving: " << directory.getChildFile(obj->getName() + T(".xml")).getFullPathName() << std::endl;
      obj->saveToFile(directory.getChildFile(obj->getName() + T(".xml")), callback);
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
  PredictFunction(InferencePtr inference,
                  InferenceContextPtr context = singleThreadedInferenceContext())
    : inference(inference), context(context)
    {jassert(inference && context);}
  
  virtual TypePtr getInputType() const
    {return inference->getInputType();}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inference->getOutputType(inputType);}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
    {return context->predict(inference, input);}
  
protected:
  InferencePtr inference;
  InferenceContextPtr context;
};

// Protein -> Protein (with input data only)
class InputProteinFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass;}
  
  virtual TypePtr getOutputType(TypePtr ) const
    {return proteinClass;}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    
    return inputProtein;
  }
};
  
class ProteinInferenceEvaluatorProgram : public WorkUnit
{
public:
  ProteinInferenceEvaluatorProgram() : numThreads(1) {}
  
  virtual String toString() const
    {return T("Take an learned inference and save prediction \
              from an input protein directory to an output directory.");}
  
  virtual bool run(ExecutionContext& context)
  {
    if (!inputDirectory.exists()
        || !outputDirectory.exists()
        || !inferenceFile.exists())
    {
      context.errorCallback(T("ProteinInferenceEvaluatorProgram::run"), getUsageString());
      return false;
    }

    InferencePtr inference = Inference::createFromFile(inferenceFile);
    if (!inference)
    {
      context.errorCallback(T("ProteinInferenceEvaluatorProgram::run"), T("Sorry, the inference file is not correct !"));
      return false;
    }

    ThreadPoolPtr pool = new ThreadPool(numThreads, false);
    std::cout << "Threads      : " << numThreads << std::endl;

    ContainerPtr data = directoryFileStream(inputDirectory, T("*.xml"))
      ->load()
      ->apply(loadFromFileFunction(proteinClass), pool)
      ->apply(FunctionPtr(new InputProteinFunction()));
    std::cout << "Data         : " << data->getNumElements() << std::endl;

    data->apply(FunctionPtr(new PredictFunction(inference, multiThreadedInferenceContext(pool))))
      ->apply(FunctionPtr(new SaveToFileFunction(outputDirectory)), false);

    return true;
  }

protected:
  friend class ProteinInferenceEvaluatorProgramClass;
  
  File inputDirectory;
  File outputDirectory;
  File inferenceFile;
  
  size_t numThreads;
};

};
