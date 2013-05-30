/*-----------------------------------------.---------------------------------.
| Filename: LowMemoryRTreeFunction.h       | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/08/2012 10:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Learning/DecisionTree.h>
# include <lbcpp/Learning/BatchLearner.h>
# include "RTreeFunction.h"

namespace lbcpp
{

extern BatchLearnerPtr lowMemoryRTreeBatchLearner();

class LowMemoryRTreeFunction : public ExtraTreesFunction
{
public:
  LowMemoryRTreeFunction(size_t numTrees,
                         size_t numAttributeSamplesPerSplit,
                         size_t minimumSizeForSplitting)
  : ExtraTreesFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(lowMemoryRTreeBatchLearner()));}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    jassert(inputs[0].exists());
    
    ContainerPtr input = inputs[0].getObjectAndCast<Container>(context);
    const double hashCode = computeHashCode(input);

    for (PredictedMap::const_iterator it = predictedData.find(hashCode);
        it != predictedData.end(); ++it)
    {
      if (input->compare(it->second.first) == 0)
        return it->second.second;
    }
    
    context.errorCallback(T("LowMemoryRTreeFunction::computeFunction"), T("Data not pre-predicted ! Skipped !"));
    return Variable::missingValue(getOutputType());
  }
  
protected:
  friend class LowMemoryRTreeFunctionClass;
  friend class LowMemoryRTreeBatchLearner;

  typedef std::multimap<double, std::pair<ContainerPtr, Variable> > PredictedMap;
  PredictedMap predictedData;

  LowMemoryRTreeFunction() {}

  virtual RTreeFunctionPtr createExtraTreeImplementation() const = 0;

  virtual Variable createEmptyPrediction() const = 0;
  virtual Variable addPrediction(const Variable& a, const Variable& b) const = 0;
  virtual Variable finalizePrediction(const Variable& value) const = 0;

  static double computeHashCode(const ContainerPtr& input)
  {
    jassert(input);

    double res = 0.f;
    const size_t numAttributes = input->getNumElements();    
    for (size_t j = 0; j < (size_t)numAttributes; ++j)
    {
      Variable objVariable = input->getElement(j);
      TypePtr objType = objVariable.getType();
      if (objType->inheritsFrom(booleanType))
        res += objVariable.getBoolean() ? 1.f : 0.f;
      else if (objType->inheritsFrom(enumValueType))
        res += objVariable.getInteger();
      else if (objType->inheritsFrom(doubleType))
        res += objVariable.getDouble();
      else if (objType->inheritsFrom(integerType))
        res += objVariable.getInteger();
      else
        jassertfalse;
    }
    return res;
  }
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

    std::vector<void*> precomputedCoreTables(numTesting);
    for (size_t i = 0; i < numTesting; ++i)
      precomputedCoreTables[i] = RTreeFunction::computeCoreTableOf(context, validationData[i]->getVariable(0));

    std::vector<Variable> predictions;
    predictions.resize(numTesting);
    for (size_t i = 0; i < numTesting; ++i)
      predictions[i] = rTreeFunction->createEmptyPrediction();

    context.enterScope(T("Low Memory RTree Learning"));
    for (size_t i = 0; i < rTreeFunction->numTrees; ++i)
    {
      RTreeFunctionPtr x3Function = rTreeFunction->createExtraTreeImplementation();
      x3Function->setVerbosity(false);
      if (!x3Function->train(context, trainingData))
      {
        context.errorCallback(T("LowMemoryRTreeFunction"), T("Error during learning"));
        return false;
      }

      for (size_t j = 0; j < numTesting; ++j)
      {
        const Variable result = x3Function->makePredictionFromCoreTable(context, precomputedCoreTables[j]);
        predictions[j] = rTreeFunction->addPrediction(predictions[j], result);
      }

      context.progressCallback(new ProgressionState(i + 1, rTreeFunction->numTrees, T("trees")));
    }

    for (size_t j = 0; j < numTesting; ++j)
      predictions[j] = rTreeFunction->finalizePrediction(predictions[j]);

    for (size_t i = 0; i < numTesting; ++i)
    {
      ContainerPtr input = validationData[i]->getVariable(0).getObjectAndCast<Container>();
      rTreeFunction->predictedData.insert(std::make_pair(rTreeFunction->computeHashCode(input), 
                                                         std::make_pair(input, predictions[i])));
    }

    for (size_t i = 0; i < numTesting; ++i)
      RTreeFunction::deleteCoreTable(precomputedCoreTables[i]);

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
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual RTreeFunctionPtr createExtraTreeImplementation() const
    {return new RegressionRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting);}

  virtual Variable createEmptyPrediction() const
    {return Variable(0.f, doubleType);}

  virtual Variable addPrediction(const Variable& a, const Variable& b) const
    {return Variable(a.getDouble() + b.getDouble(), doubleType);}

  virtual Variable finalizePrediction(const Variable& value) const
    {return Variable(value.getDouble() / (double)numTrees, doubleType);}

protected:
  friend class RegressionLowMemoryRTreeFunctionClass;

  RegressionLowMemoryRTreeFunction() {}
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
  
  virtual TypePtr getSupervisionType() const
    {return sumType(probabilityType, booleanType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
  
  virtual RTreeFunctionPtr createExtraTreeImplementation() const
    {return new BinaryRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting);}

  virtual Variable createEmptyPrediction() const
    {return Variable(0.f, probabilityType);}

  virtual Variable addPrediction(const Variable& a, const Variable& b) const
    {return Variable(a.getDouble() + b.getDouble(), probabilityType);}
  
  virtual Variable finalizePrediction(const Variable& value) const
    {return Variable(value.getDouble() / (double)numTrees, probabilityType);}

protected:
  friend class BinaryLowMemoryRTreeFunctionClass;

  BinaryLowMemoryRTreeFunction() {}
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
  
  virtual TypePtr getSupervisionType() const
    {return sumType(enumValueType, doubleVectorClass(enumValueType, probabilityType));}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (inputVariables[1]->getType()->inheritsFrom(enumValueType))
      return denseDoubleVectorClass(inputVariables[1]->getType(), probabilityType);
    return inputVariables[1]->getType();
  }
  
  virtual RTreeFunctionPtr createExtraTreeImplementation() const
    {return new ClassificationRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting);}

  virtual Variable createEmptyPrediction() const
    {return new DenseDoubleVector(denseDoubleVectorClass(getOutputType()->getTemplateArgument(0), probabilityType));}

  virtual Variable addPrediction(const Variable& a, const Variable& b) const
  {
    b.getObjectAndCast<DenseDoubleVector>()->addTo(a.getObjectAndCast<DenseDoubleVector>());
    return a;
  }
  
  virtual Variable finalizePrediction(const Variable& value) const
  {
    std::vector<double>& values = value.getObjectAndCast<DenseDoubleVector>()->getValues();
    for (size_t i = 0; i < values.size(); ++i)
      values[i] /= numTrees;
    return value;
  }

protected:
  friend class ClassificationLowMemoryRTreeFunctionClass;

  ClassificationLowMemoryRTreeFunction() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
