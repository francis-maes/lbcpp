#include "../Evaluator/KolmogorovPerfectMatchingFunction.h"

namespace lbcpp
{

class ProteinLearnerFunction : public Function
{
public:
  ProteinLearnerFunction(const String& inputDirectory,
                         const String& supervisionDirectory,
                         const String& machineLearning,
                         ProteinTarget target,
                         bool isValidation)
    : inputDirectory(inputDirectory),
      supervisionDirectory(supervisionDirectory),
      machineLearning(machineLearning),
      target(target),
      isValidation(isValidation) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return largeProteinParametersClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scalarVariableMeanAndVarianceClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("train/")), context.getFile(supervisionDirectory).getChildFile(T("train/")), 0, T("Loading proteins"));
    
    LargeProteinParametersPtr parameter = input.getObjectAndCast<LargeProteinParameters>(context);
    context.resultCallback(T("Parameter"), parameter);

    ProteinPredictorParametersPtr predictor = createAutoTunedPredictor(context, trainingProteins, machineLearning, parameter);

    ScalarVariableMeanAndVariancePtr res = new ScalarVariableMeanAndVariance();
    if (isValidation)
    {
      ContainerPtr testingProteins = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("test/")), context.getFile(supervisionDirectory).getChildFile(T("test/")), 0, T("Loading proteins"));
      res->push(computeFold(context, predictor, trainingProteins, testingProteins));
    }
    else
    {
      for (size_t i = 0; i < 10; ++i)
        res->push(computeFold(context, predictor, trainingProteins->invFold(i, 10), trainingProteins->fold(i, 10)));
    }

    return res;
  }

  double computeFold(ExecutionContext& context, const ProteinPredictorParametersPtr& predictor,
                     const ContainerPtr& trainingProteins, const ContainerPtr& testingProteins) const
  {
    ProteinPredictorParametersPtr clone = predictor->cloneAndCast<ProteinPredictorParameters>(context);
    ProteinPredictorPtr iteration = new ProteinPredictor(clone);
    iteration->addTarget(target);

    if (!iteration->train(context, trainingProteins, testingProteins, T("Training")))
      return 101.f;

    ProteinEvaluatorPtr evaluator = createProteinEvaluator();
    CompositeScoreObjectPtr scores = iteration->evaluate(context, testingProteins, evaluator, T("EvaluateTest"));
    return evaluator->getScoreToMinimize(scores);
  }

  ProteinPredictorParametersPtr createAutoTunedPredictor(ExecutionContext& context,
                                                         const ContainerPtr& proteins,
                                                         const String& machineLearning,
                                                         const LargeProteinParametersPtr& parameter) const
  {
    if (machineLearning == T("ExtraTrees"))
    {
      LargeProteinPredictorParametersPtr res = new LargeProteinPredictorParameters(parameter);
      res->learningMachineName = T("ExtraTrees");
      res->x3Trees = 1000;
      res->x3Attributes = 0;
      res->x3Splits = 1;
      res->x3LowMemory = true;
      return res;
    }

    if (machineLearning == T("SVM"))
    {
      double bestC = DBL_MAX;
      double bestGamma = DBL_MAX;
      double bestError = DBL_MAX;

      std::vector<double> valuesC;
      valuesC.push_back(0);
      valuesC.push_back(5);
      valuesC.push_back(10);
      valuesC.push_back(15);

      std::vector<double> valuesGamma;
      valuesGamma.push_back(-14);
      valuesGamma.push_back(-7);
      valuesGamma.push_back(0);
      valuesGamma.push_back(7);
      valuesGamma.push_back(14);

      for (size_t i = 0; i < valuesC.size(); ++i)
      {
        for (size_t j = 0; j < valuesGamma.size(); ++j)
        {
          LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
          predictor->learningMachineName = T("LibSVM");
          predictor->svmC = valuesC[i];
          predictor->svmGamma = valuesGamma[j];
          double result = computeFold(context, predictor, proteins->invFold(0,5), proteins->fold(0,5));
          
          if (result < bestError)
          {
            bestC = valuesC[i];
            bestGamma = valuesGamma[j];
            bestError = result;
          }
        }
      }

      LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
      predictor->learningMachineName = T("LibSVM");
      predictor->svmC = bestC;
      predictor->svmGamma = bestGamma;
      return predictor;
    }
    
    context.errorCallback(T("Undefined machine learning !"));
    return ProteinPredictorParametersPtr();
  }

protected:
  friend class ProteinLearnerFunctionClass;

  String inputDirectory;
  String supervisionDirectory;
  String machineLearning;
  ProteinTarget target;
  bool isValidation;

  ProteinLearnerFunction() {}

  ProteinEvaluatorPtr createProteinEvaluator() const
  {    
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    
    if (target == ss3Target || target == ss8Target || target == stalTarget)
      evaluator->addEvaluator(target, elementContainerSupervisedEvaluator(classificationEvaluator()), T("SS3-SS8-StAl"), true);
    else if (target == sa20Target || target == cbsTarget)
      evaluator->addEvaluator(target, elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("SA20"), true);
    else if (target == drTarget)
      evaluator->addEvaluator(target, elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationMCCScore)), T("DR"), true);
    else if (target == dsbTarget)
      evaluator->addEvaluator(target, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("DSB QP Perfect"), true);
    else if (target == cbpTarget)
      evaluator->addEvaluator(target, binaryClassificationEvaluator(binaryClassificationAccuracyScore), T("CBP"), true);
    
    return evaluator;
  }
};

class ProteinFeatureFunctionSelectionWorkUnit : public WorkUnit
{
public:
  ProteinFeatureFunctionSelectionWorkUnit()
    : target(noTarget), memoryResourceInGb(0) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    if (projectName == String::empty)
    {
      context.errorCallback(T("Please provide a projet name"));
      return false;
    }
    
    if (gridName == String::empty)
    {
      context.errorCallback(T("Please provide the grid identifier (user@grid)"));
      return false;
    }

    if (!memoryResourceInGb)
    {
      context.errorCallback(T("Please provide the amout of expected memory (in Gb)"));
      return false;
    }

    if (target == noTarget)
    {
      context.errorCallback(T("Please provide a valid protein task"));
      return false;
    }
    
    if (machineLearning == String::empty)
    {
      context.errorCallback(T("Please provide the machine learning name"));
      return false;
    }

    ExecutionContextPtr remoteContext = distributedExecutionContext(context,
                                            T("m24.big.ulg.ac.be"), 1664,
                                            projectName, T("jbecker@screen"), gridName,
                                            fixedResourceEstimator(1, memoryResourceInGb * 1024, 300), false);

    LargeProteinParametersPtr initialParameters = new LargeProteinParameters();

    OptimizationProblemPtr problem = new OptimizationProblem(
        new ProteinLearnerFunction(inputDirectory, supervisionDirectory, machineLearning, target, false),
        initialParameters, SamplerPtr(),
        new ProteinLearnerFunction(inputDirectory, supervisionDirectory, machineLearning, target, true));

    std::vector<StreamPtr> streams;
    if (target == cbpTarget)
      streams = LargeProteinParameters::createStreams(T("useRelativePosition"));
    else if (target == cbsTarget)
      streams = LargeProteinParameters::createStreams(T("usePositionDifference"));
    else if (target == dsbTarget)
      streams = LargeProteinParameters::createStreams(T("cbsAbsoluteSize"));
    else
    {
      context.errorCallback(T("No predetermined streams !"));
      return false;
    }

    
    OptimizerPtr optimizer = bestFirstSearchOptimizer(streams, optimizerStateFile);

    return optimizer->compute(*remoteContext.get(), problem);
  }

protected:
  friend class ProteinFeatureFunctionSelectionWorkUnitClass;

  String inputDirectory;
  String supervisionDirectory;
  File optimizerStateFile;
  ProteinTarget target;

  String machineLearning;
  
  String projectName;
  String gridName;
  size_t memoryResourceInGb;
};

};