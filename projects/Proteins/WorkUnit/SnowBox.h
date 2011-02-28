
#include <lbcpp/lbcpp.h>
#include "../Data/Protein.h"
#include "../Data/ProteinFunctions.h"
#include "../Predictor/ProteinPredictor.h"

namespace lbcpp {

class ProteinTargetsArgument : public Object
{
public:
  ProteinTargetsArgument(ExecutionContext& context, const String& targets)
    {loadFromString(context, targets);}

  ProteinTargetsArgument() {}
  
  size_t getNumStages() const
    {return stages.size();}
  
  size_t getNumTasks(size_t stageIndex) const
    {jassert(stageIndex < getNumStages()); return stages[stageIndex].size();}
  
  String getTaskName(size_t stageIndex, size_t taskIndex) const
    {jassert(taskIndex < getNumTasks(stageIndex)); return proteinClass->getMemberVariableName(stages[stageIndex][taskIndex]);}
  
  ProteinTarget getTask(size_t stageIndex, size_t taskIndex) const
    {jassert(taskIndex < getNumTasks(stageIndex)); return (ProteinTarget)stages[stageIndex][taskIndex];}
  
  virtual String toString() const
    {return description;}
  
  virtual bool loadFromString(ExecutionContext& context, const String& str);
  
protected:
  friend class ProteinTargetsArgumentClass;

  std::vector<std::vector<int> > stages;
  String description;
};

typedef ReferenceCountedObjectPtr<ProteinTargetsArgument> ProteinTargetsArgumentPtr;

class SnowBox : public WorkUnit
{
public:
  SnowBox() : output(File::getCurrentWorkingDirectory().getChildFile(T("result")))
            , maxProteinsToLoad(0), numberOfFolds(7), currentFold(0)
            , useCrossValidation(false), partAsValidation(0)
            , baseLearner(T("OneAgainstAllLinearSVM")), maxIterations(15)
            , numTrees(100), numAttributesPerSplit(20), numForSplitting(1)
            , target(new ProteinTargetsArgument(defaultExecutionContext(), T("(SS3-DR)2")))
            , exportPerceptions(false), optimizeLearningParameter(false)
            , saveIntermediatePredictions(false), targetToOptimize(T("secondaryStructure"))
            , stageToOptimize(0) {}
  
  virtual String toString() const
  {
    return String("SnowBox is ... a snow box ! It's a flexible program allowing" \
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
  File inputDirectory;

  size_t maxProteinsToLoad;

  size_t numberOfFolds;
  size_t currentFold;
  bool useCrossValidation;
  size_t partAsValidation;

  String baseLearner;
  size_t maxIterations;
  size_t numTrees;
  size_t numAttributesPerSplit;
  size_t numForSplitting;
  
  ProteinTargetsArgumentPtr target;
  
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
  ProteinPredictorParametersPtr createParameters(ExecutionContext& context) const;
  ProteinSequentialPredictorPtr loadPredictorOrCreateIfFail(ExecutionContext& context) const;
  void printInformation(ExecutionContext& context) const;
};

};
