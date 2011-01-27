

#include <lbcpp/lbcpp.h>
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "LearningParameter.h"
#include "ParameteredProteinInferenceFactory.h"

namespace lbcpp {

class SaveObjectProgram : public WorkUnit
{
  virtual String toString() const
    {return T("SaveObjectProgram is able to serialize a object.");}
  
  virtual Variable run(ExecutionContext& context)
  {
    if (className == String::empty)
    {
      context.warningCallback(T("SaveObjectProgram::run"), T("No class name specified"));
      return false;
    }

    if (outputFile == File::nonexistent)
      outputFile = File::getCurrentWorkingDirectory().getChildFile(className + T(".xml"));
    
    std::cout << "Loading class " << className.quoted() << " ... ";
    std::flush(std::cout);

    TypePtr type = typeManager().getType(context, className);
    if (!type)
    {
      std::cout << "Fail" << std::endl;
      return false;
    }

    ObjectPtr obj = Object::create(type);
    if (!obj)
    {
      std::cout << "Fail" << std::endl;
      return false;
    }

    std::cout << "OK" << std::endl;
    std::cout << "Saving class to " << outputFile.getFileName().quoted() << " ... ";
    std::flush(std::cout);

    obj->saveToFile(context, outputFile);

    std::cout << "OK" << std::endl;
    return true;
  }

protected:
  friend class SaveObjectProgramClass;
  
  String className;
  File outputFile;
};
  
class SnowBox : public WorkUnit
{
public:
  SnowBox() : output(File::getCurrentWorkingDirectory().getChildFile(T("result")))
            , maxProteinsToLoad(0), numberOfFolds(7), currentFold(0)
            , useCrossValidation(false), partAsValidation(0)
            , baseLearner(T("OneAgainstAllLinearSVM")), maxIterations(15)
            , defaultParameter(new NumericalLearningParameter(0.0, 4.0, -10.0))
            , numTrees(100), numAttributesPerSplit(20), numForSplitting(1)
            , target(new ProteinTarget(defaultExecutionContext(), T("(SS3-DR)2")))
            , exportPerceptions(false), optimizeLearningParameter(false)
            , saveIntermediatePredictions(false), targetToOptimize(T("secondaryStructure"))
            , stageToOptimize(0) {}
  
  virtual String toString() const
  {
    return String("SnowBox is ... a snow box ! It's a flexible program allowing"    \
              " you to learn, save, resume models. Just by giving the targets"   \
              " to learn and a training dataset. You can specified the learning" \
              " based-model, the learning rate for each target and each passes," \
              " a specific testing set or a cross-validation protocol.");
  }

  virtual Variable run(ExecutionContext& context);

protected:
  friend class SnowBoxClass;

  File learningDirectory;
  File testingDirectory;
  File validationDirectory;
  File output;
  File inferenceFile;
  File inputDirectory;

  size_t maxProteinsToLoad;

  size_t numberOfFolds;
  size_t currentFold;
  bool useCrossValidation;
  size_t partAsValidation;

  String baseLearner;
  size_t maxIterations;
  std::vector<std::pair<String, std::pair<LearningParameterPtr, LearningParameterPtr> > > learningParameters;
  LearningParameterPtr defaultParameter;
  size_t numTrees;
  size_t numAttributesPerSplit;
  size_t numForSplitting;
  
  ProteinTargetPtr target;
  
  bool exportPerceptions;
  bool optimizeLearningParameter;
  bool saveIntermediatePredictions;
  
  String targetToOptimize;
  size_t stageToOptimize;

private:
  ContainerPtr learningData;
  ContainerPtr testingData;
  ContainerPtr validationData;

  bool loadData(ExecutionContext& context);
  ProteinInferenceFactoryPtr createFactory(ExecutionContext& context) const;
  ProteinSequentialInferencePtr loadOrCreateIfFailInference(ExecutionContext& context, ParameteredProteinInferenceFactoryPtr factory) const;
  void printInformation(ExecutionContext& context) const;
};
  
class LearningParameterObjectiveFunction : public ObjectiveFunction
{
public:
  LearningParameterObjectiveFunction(ParameteredProteinInferenceFactoryPtr factory, ProteinTargetPtr target,
                                     const String& targetName, size_t targetStage,
                                     ContainerPtr learningData, ContainerPtr validationData)
  : factory(factory), target(target), 
  targetName(targetName), targetStage(targetStage),
  learningData(learningData), validationData(validationData) {}
  LearningParameterObjectiveFunction() {}
  
  virtual double compute(ExecutionContext& context, const Variable& input) const;
  
protected:
  friend class LearningParameterObjectiveFunctionClass;
  
  ParameteredProteinInferenceFactoryPtr factory;
  ProteinTargetPtr target;
  String targetName;
  size_t targetStage;
  ContainerPtr learningData;
  ContainerPtr validationData;
};

};
