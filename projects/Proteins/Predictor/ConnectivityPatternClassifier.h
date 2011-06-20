/*-----------------------------------------.---------------------------------.
| Filename: ConnectivityPatternClassifier.h| Connectivity Pattern Classifier |
| Author  : Julien Becker                   |                                 |
| Started : 16/06/2011 13:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONNECTIVITY_PATTERN_CLASSIFIER_H_
# define LBCPP_CONNECTIVITY_PATTERN_CLASSIFIER_H_

# include "../../src/Learning/Numerical/LinearLearningMachine.h"
# include "../../src/Function/Evaluator/Utilities.h"
# include "../Evaluator/DisulfideBondEvaluator.h"
# include "../Data/ProteinFunctions.h"

namespace lbcpp
{

extern ClassPtr addConnectivityPatternBiasLearnableFunctionClass;

class AddConnectivityPatternBiasLearnableFunction : public Function
{
public:
  AddConnectivityPatternBiasLearnableFunction()
    {thisClass = addConnectivityPatternBiasLearnableFunctionClass;}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)symmetricMatrixClass(doubleType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const SymmetricMatrixPtr& prediction = inputs[0].getObjectAndCast<SymmetricMatrix>(context);
    jassert(prediction);
    
    const size_t n = prediction->getNumElements();
    for (size_t i = 0; i < n; ++i)
      prediction->setElement(i, Variable(prediction->getElement(i).getDouble() + bias, doubleType));

    return prediction;
  }

  void setBias(double bias)
    {this->bias = bias;}

protected:
  friend class AddConnectivityPatternBiasLearnableFunctionClass;

  double bias;
};

class AddConnectivityPatternBiasBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return addConnectivityPatternBiasLearnableFunctionClass;}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const ReferenceCountedObjectPtr<AddConnectivityPatternBiasLearnableFunction>& function = f.staticCast<AddConnectivityPatternBiasLearnableFunction>();
    
    ROCScoreObject roc;
    insertToROCScoreObject(context, trainingData, roc);
    insertToROCScoreObject(context, validationData, roc);
    
    if (!roc.getSampleCount())
    {
      context.errorCallback(T("No training examples"));
      return false;
    }
    // Get threshold
    std::vector<double> thresholds;
    roc.getAllThresholds(thresholds);
    
    double bestScore = DBL_MAX;
    double bestThreshold = 0.0;
    // Find the best threshold the 0/1 Loss score
    for (size_t i = 0; i < thresholds.size(); ++i)
    {
      SupervisedEvaluatorPtr evaluator = new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, thresholds[i], doubleType), thresholds[i]);
      ScoreObjectPtr scoreObject = evaluator->createEmptyScoreObject(context, FunctionPtr());

      insertToEvaluatorScoreObject(context, trainingData, evaluator, scoreObject);
      insertToEvaluatorScoreObject(context, validationData, evaluator, scoreObject);

      evaluator->finalizeScoreObject(scoreObject, FunctionPtr());
      double score = scoreObject->getScoreToMinimize();

      if (score < bestScore)
      {
        bestScore = score;
        bestThreshold = thresholds[i];
      }
    }

    context.resultCallback(T("Best threshold"), bestThreshold);
    context.resultCallback(T("Best threshold score"), bestScore);
    function->setBias(-bestThreshold);
    return true;
  }
  
protected:  
  void insertToROCScoreObject(ExecutionContext& context, const std::vector<ObjectPtr>& data, ROCScoreObject& roc) const
  {
    for (size_t i = 0; i < data.size(); ++i)
    {
      const ObjectPtr& example = data[i];

      SymmetricMatrixPtr supervision = example->getVariable(1).getObjectAndCast<SymmetricMatrix>(context);
      SymmetricMatrixPtr prediction = example->getVariable(0).getObjectAndCast<SymmetricMatrix>(context);
      jassert(supervision && prediction);

      const size_t n = supervision->getNumElements();
      for (size_t j = 0; j < n; ++j)
      {
        bool isPositive;
        if (convertSupervisionVariableToBoolean(supervision->getElement(j), isPositive))
          roc.addPrediction(context, prediction->getElement(j).getDouble(), isPositive);
      }
    }
  }
  
  void insertToEvaluatorScoreObject(ExecutionContext& context, const std::vector<ObjectPtr>& data, const SupervisedEvaluatorPtr& evaluator, const ScoreObjectPtr& scoreObject) const
  {
    for (size_t i = 0; i < data.size(); ++i)
    {
      const ObjectPtr& example = data[i];
      
      SymmetricMatrixPtr supervision = example->getVariable(1).getObjectAndCast<SymmetricMatrix>(context);
      SymmetricMatrixPtr prediction = example->getVariable(0).getObjectAndCast<SymmetricMatrix>(context);
      jassert(supervision && prediction);
      
      evaluator->addPrediction(context, prediction, supervision, scoreObject);
    }
  }
};

class NoPostProcessingLinearBinaryClassifier : public SupervisedNumericalFunction
{
public:
  NoPostProcessingLinearBinaryClassifier(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters) {}

  NoPostProcessingLinearBinaryClassifier() {}

  virtual TypePtr getInputType() const
    {return doubleVectorClass();}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual NumericalLearnableFunctionPtr createLearnableFunction() const
  {
    NumericalLearnableFunctionPtr res = linearLearnableFunction();
    res->setEvaluator(binaryClassificationEvaluator()); // todo: connect with scoreToOptimize
    return res;
  }
};

extern ClassPtr connectivityPatternClassifierClass;

class ConnectivityPatternClassifier : public CompositeFunction
{
public:
  ConnectivityPatternClassifier(LearnerParametersPtr learningParameters)
    : learningParameters(learningParameters)
    {thisClass = connectivityPatternClassifierClass;}

  ConnectivityPatternClassifier() {}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? symmetricMatrixClass(probabilityType) : symmetricMatrixClass(doubleVectorClass());}
  
  virtual String getOutputPostFix() const
    {return T("PatternPrediction");}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(symmetricMatrixClass(doubleVectorClass()));
    size_t supervision = builder.addInput(symmetricMatrixClass(probabilityType));

    FunctionPtr classifier = new NoPostProcessingLinearBinaryClassifier(learningParameters);
    classifier->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
    size_t prediction = builder.addFunction(mapNSymmetricMatrixFunction(classifier, 1), input, supervision);
    prediction = builder.addFunction(new AddConnectivityPatternBiasLearnableFunction(), prediction, supervision);
    builder.addFunction(mapNSymmetricMatrixFunction(signedScalarToProbabilityFunction(), 1), prediction);
  }

protected:
  friend class ConnectivityPatternClassifierClass;

  LearnerParametersPtr learningParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONNECTIVITY_PATTERN_CLASSIFIER_H_