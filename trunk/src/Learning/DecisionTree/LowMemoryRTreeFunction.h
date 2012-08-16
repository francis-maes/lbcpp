/*-----------------------------------------.---------------------------------.
| Filename: LowMemoryRTreeFunction.h       | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/08/2012 10:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Learning/DecisionTree.h>
# include <lbcpp/Learning/BatchLearner.h>
# include "RTreeFunction.h"

namespace lbcpp
{

extern BatchLearnerPtr lowMemoryRTreeBatchLearner();

class LowMemoryRTreeFunction : public Function
{
public:
  LowMemoryRTreeFunction(const ContainerPtr& testingData,
                         size_t numTrees,
                         size_t numAttributeSamplesPerSplit,
                         size_t minimumSizeForSplitting)
    : testingData(testingData),
      numTrees(numTrees),
      numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(lowMemoryRTreeBatchLearner()));}

  /* LowMemoryRTreeFunction */
  size_t getNumTrees() const
    {return numTrees;}
  
  size_t getNumAttributeSamplesPerSplit() const
    {return numAttributeSamplesPerSplit;}
  
  size_t getMinimumSizeForSplitting() const
    {return minimumSizeForSplitting;}
  
  virtual TypePtr getSupervisionType() const = 0;
  
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass(anyType) : getSupervisionType();}
  
  virtual String getOutputPostFix() const
    {return T("LowMemRTree");}  
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ContainerPtr inputData = inputs[0].getObjectAndCast<Container>();
    jassert(predictions.count(inputData) == 1);
    return predictions.find(inputData)->second;
  }
  
protected:
  friend class LowMemoryRTreeFunctionClass;
  friend class LowMemoryRTreeBatchLearner;
  
  ContainerPtr testingData;

  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  std::map<ObjectPtr, Variable> predictions;

  LowMemoryRTreeFunction()
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(lowMemoryRTreeBatchLearner()));}
  
  virtual FunctionPtr createExtraTreeImplementation() const = 0;
  virtual Variable computePrediction(const std::vector<Variable>& subPredicitons) const = 0;
};

extern ClassPtr lowMemoryRTreeFunctionClass;
typedef ReferenceCountedObjectPtr<LowMemoryRTreeFunction> LowMemoryRTreeFunctionPtr;


class LowMemoryRTreeBatchLearner : public BatchLearner
{
public:  
  virtual TypePtr getRequiredFunctionType() const
    {return lowMemoryRTreeFunctionClass;}

  virtual bool train(ExecutionContext& context,
                     const FunctionPtr& function,
                     const std::vector<ObjectPtr>& trainingData,
                     const std::vector<ObjectPtr>& validationData) const
  {
    const LowMemoryRTreeFunctionPtr& rTreeFunction = function.staticCast<LowMemoryRTreeFunction>();
    if (!checkHasAtLeastOneExemples(trainingData))
    {
      context.errorCallback(T("No training examples"));
      return false;
    }

    const size_t numTesting = rTreeFunction->testingData->getNumElements();

    std::vector<std::vector<Variable> > predictions(numTesting);
    for (size_t i = 0; i < numTesting; ++i)
      predictions[i] = std::vector<Variable>(rTreeFunction->numTrees);

    context.enterScope(T("Low Memory RTree Learning"));
    for (size_t i = 0; i < rTreeFunction->numTrees; ++i)
    {
      FunctionPtr x3Function = rTreeFunction->createExtraTreeImplementation();
      if (!x3Function->train(context, trainingData))
      {
        context.errorCallback(T("LowMemoryRTreeFunction"), T("Error during learning"));
        return false;
      }
      
      for (size_t j = 0; j < numTesting; ++j)
        predictions[j][i] = x3Function->compute(context, rTreeFunction->testingData->getElement(j).getObjectAndCast<Pair>()->getFirst(), Variable());

      context.progressCallback(new ProgressionState(i + 1, rTreeFunction->numTrees, T("trees")));
    }

    for (size_t i = 0; i < numTesting; ++i)
    {
      ObjectPtr key = rTreeFunction->testingData->getElement(i).getObjectAndCast<Pair>()->getFirst().getObject();
      Variable value = rTreeFunction->computePrediction(predictions[i]);
      rTreeFunction->predictions[key] = value;
    }
    
    context.leaveScope();
    return true;
  }
};

class RegressionLowMemoryRTreeFunction : public LowMemoryRTreeFunction
{
public:
  RegressionLowMemoryRTreeFunction(const ContainerPtr& testingData,
                                   size_t numTrees,
                                   size_t numAttributeSamplesPerSplit,
                                   size_t minimumSizeForSplitting)
    : LowMemoryRTreeFunction(testingData,
                             numTrees,
                             numAttributeSamplesPerSplit,
                             minimumSizeForSplitting) {}
  RegressionLowMemoryRTreeFunction() {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual FunctionPtr createExtraTreeImplementation() const
  {
    return new RegressionRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting, false);
  }

  virtual Variable computePrediction(const std::vector<Variable>& subPredicitons) const
  {
    const size_t n = subPredicitons.size();
    double sum = 0.f;
    for (size_t i = 0; i < n; ++i)
      sum += subPredicitons[i].getDouble();
    return Variable(sum / (double)n, doubleType);
  }
};

class BinaryLowMemoryRTreeFunction : public LowMemoryRTreeFunction
{
public:
  BinaryLowMemoryRTreeFunction(const ContainerPtr& testingData,
                                   size_t numTrees,
                                   size_t numAttributeSamplesPerSplit,
                                   size_t minimumSizeForSplitting)
    : LowMemoryRTreeFunction(testingData,
                             numTrees,
                             numAttributeSamplesPerSplit,
                             minimumSizeForSplitting) {}
  BinaryLowMemoryRTreeFunction() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(probabilityType, booleanType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
  
  virtual FunctionPtr createExtraTreeImplementation() const
  {
    return new BinaryRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting, false);
  }

  virtual Variable computePrediction(const std::vector<Variable>& subPredicitons) const
  {
    const size_t n = subPredicitons.size();
    double sum = 0.f;
    for (size_t i = 0; i < n; ++i)
      sum += subPredicitons[i].getDouble();
    return Variable(sum / (double)n, probabilityType);
  }
};

class ClassificationLowMemoryRTreeFunction : public LowMemoryRTreeFunction
{
public:
  ClassificationLowMemoryRTreeFunction(const ContainerPtr& testingData,
                                   size_t numTrees,
                                   size_t numAttributeSamplesPerSplit,
                                   size_t minimumSizeForSplitting)
    : LowMemoryRTreeFunction(testingData,
                             numTrees,
                             numAttributeSamplesPerSplit,
                             minimumSizeForSplitting) {}
  ClassificationLowMemoryRTreeFunction() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(enumValueType, doubleVectorClass(enumValueType, probabilityType));}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (inputVariables[1]->getType()->inheritsFrom(enumValueType))
      return denseDoubleVectorClass(inputVariables[1]->getType(), probabilityType);
    return inputVariables[1]->getType();
  }
  
  virtual FunctionPtr createExtraTreeImplementation() const
  {
    return new ClassificationRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting, false);
  }

  virtual Variable computePrediction(const std::vector<Variable>& subPredicitons) const
  {
    const size_t n = subPredicitons.size();
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    for (size_t i = 0; i < n; ++i)
      subPredicitons[i].getObjectAndCast<DenseDoubleVector>()->addTo(res);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
