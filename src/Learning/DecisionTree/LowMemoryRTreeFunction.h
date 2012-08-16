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
  LowMemoryRTreeFunction(size_t numTrees,
                         size_t numAttributeSamplesPerSplit,
                         size_t minimumSizeForSplitting)
    : numTrees(numTrees),
      numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting),
      predictionIndex((size_t)-1)
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
    jassert(predictionIndex < predictions.size());
    return predictions[const_cast<LowMemoryRTreeFunction*>(this)->predictionIndex++];
  }
  
protected:
  friend class LowMemoryRTreeFunctionClass;
  friend class LowMemoryRTreeBatchLearner;

  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  size_t predictionIndex;
  std::vector<Variable> predictions;

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

    const size_t numTesting = validationData.size();

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
        predictions[j][i] = x3Function->compute(context, validationData[j]->getVariable(0), Variable());

      context.progressCallback(new ProgressionState(i + 1, rTreeFunction->numTrees, T("trees")));
    }

    rTreeFunction->predictions.resize(numTesting);
    rTreeFunction->predictionIndex = 0;
    for (size_t i = 0; i < numTesting; ++i)
      rTreeFunction->predictions[i] = rTreeFunction->computePrediction(predictions[i]);
    
    context.leaveScope();
    return true;
  }
};

class RegressionLowMemoryRTreeFunction : public LowMemoryRTreeFunction
{
public:
  RegressionLowMemoryRTreeFunction(size_t numTrees,
                                   size_t numAttributeSamplesPerSplit,
                                   size_t minimumSizeForSplitting)
    : LowMemoryRTreeFunction(numTrees,
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
  BinaryLowMemoryRTreeFunction(size_t numTrees,
                                   size_t numAttributeSamplesPerSplit,
                                   size_t minimumSizeForSplitting)
    : LowMemoryRTreeFunction(numTrees,
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
  ClassificationLowMemoryRTreeFunction(size_t numTrees,
                                       size_t numAttributeSamplesPerSplit,
                                       size_t minimumSizeForSplitting)
    : LowMemoryRTreeFunction(numTrees,
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
    if (n == 0)
      return Variable::missingValue(getOutputType());
    DenseDoubleVectorPtr res = new DenseDoubleVector(subPredicitons[0].getType());
    for (size_t i = 0; i < n; ++i)
      subPredicitons[i].getObjectAndCast<DenseDoubleVector>()->addTo(res);

    std::vector<double>& values = res->getValues();
    for (size_t i = 0; i < values.size(); ++i)
      values[i] /= n;

    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
