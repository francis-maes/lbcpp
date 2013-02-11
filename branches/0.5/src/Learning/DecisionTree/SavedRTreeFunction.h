/*-----------------------------------------.---------------------------------.
| Filename: SavedRTreeFunction.h           | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 11/02/2013 09:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAVED_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_SAVED_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Learning/DecisionTree.h>
# include <lbcpp/Learning/BatchLearner.h>
# include "RTreeFunction.h"

namespace lbcpp
{

extern BatchLearnerPtr savedRTreeBatchLearner();

class SavedRTreeFunction : public Function
{
public:
  SavedRTreeFunction(size_t numTrees,
                     size_t numAttributeSamplesPerSplit,
                     size_t minimumSizeForSplitting,
                     const File& file)
    : numTrees(numTrees),
      numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting),
      file(file)
    {
      // Tree files are not assumed to exist when using this constructor
      setBatchLearner(filterUnsupervisedExamplesBatchLearner(savedRTreeBatchLearner()));
    }

  /* SavedRTreeFunction */
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
    {return T("SavedRTree");}  
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    void* coreTable = RTreeFunction::computeCoreTableOf(context, inputs[0]);
    Variable prediction = createEmptyPrediction();

    for (size_t i = 0; i < numTrees; ++i)
    {
      RTreeFunctionPtr x3Function = createExtraTreeImplementation();
      x3Function->initialize(context, inputVariables);
      if (!x3Function->loadTreesFromBinaryFile(context, getTreesFile(i)))
      {
        context.errorCallback(T("SavedRTreeFunction::computeFunction"), T("Error while loading trees from file: ") + getTreesFile(i).getFileName());
        return Variable();
      }
      const Variable result = x3Function->makePredictionFromCoreTable(context, coreTable);
      prediction = addPrediction(prediction, result);
    }

    return finalizePrediction(prediction);
  }
  
protected:
  friend class SavedRTreeFunctionClass;
  friend class SavedRTreeBatchLearner;

  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  File file;

  SavedRTreeFunction()
  {
    // Tree files must exist when using this constructor.
    setBatchLearner(BatchLearnerPtr());
  }
  
  File getTreesFile(size_t index) const
  {
    String identifier = String((int)index);
    const int length = String((int)numTrees - 1).length();
    while (identifier.length() < length)
      identifier = T("0") + identifier;
    return File(file.getFullPathName() + T(".") + identifier + T(".x3"));
  }
  
  virtual RTreeFunctionPtr createExtraTreeImplementation() const = 0;

  virtual Variable createEmptyPrediction() const = 0;
  virtual Variable addPrediction(const Variable& a, const Variable& b) const = 0;
  virtual Variable finalizePrediction(const Variable& value) const = 0;
};

extern ClassPtr savedRTreeFunctionClass;
typedef ReferenceCountedObjectPtr<SavedRTreeFunction> SavedRTreeFunctionPtr;

class SavedRTreeBatchLearner : public BatchLearner
{
public:  
  virtual TypePtr getRequiredFunctionType() const
    {return savedRTreeFunctionClass;}

  virtual bool train(ExecutionContext& context,
                     const FunctionPtr& function,
                     const std::vector<ObjectPtr>& trainingData,
                     const std::vector<ObjectPtr>& validationData) const
  {
    const SavedRTreeFunctionPtr& rTreeFunction = function.staticCast<SavedRTreeFunction>();
    if (!checkHasAtLeastOneExemples(trainingData))
    {
      context.errorCallback(T("No training examples"));
      return false;
    }

    context.enterScope(T("SavedRTree Learning"));
    for (size_t i = 0; i < rTreeFunction->numTrees; ++i)
    {
      RTreeFunctionPtr x3Function = rTreeFunction->createExtraTreeImplementation();
      x3Function->setBatchLearner(rTreeBatchLearner());
      if (!x3Function->train(context, trainingData))
      {
        context.errorCallback(T("SavedRTreeFunction"), T("Error during learning"));
        return false;
      }
      x3Function->saveTreesToBinaryFile(context, rTreeFunction->getTreesFile(i));
      context.progressCallback(new ProgressionState(i + 1, rTreeFunction->numTrees, T("trees")));
    }

    context.leaveScope();
    return true;
  }
};

class RegressionSavedRTreeFunction : public SavedRTreeFunction
{
public:
  RegressionSavedRTreeFunction(size_t numTrees,
                               size_t numAttributeSamplesPerSplit,
                               size_t minimumSizeForSplitting,
                               const File& file)
    : SavedRTreeFunction(numTrees,
                             numAttributeSamplesPerSplit,
                             minimumSizeForSplitting,
                             file) {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual RTreeFunctionPtr createExtraTreeImplementation() const
  {
    return new RegressionRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting, false);
  }

  virtual Variable createEmptyPrediction() const
  {
    return Variable(0.f, doubleType);
  }

  virtual Variable addPrediction(const Variable& a, const Variable& b) const
  {
    return Variable(a.getDouble() + b.getDouble(), doubleType);
  }

  virtual Variable finalizePrediction(const Variable& value) const
  {
    return Variable(value.getDouble() / (double)numTrees, doubleType);
  }
  
protected:
  friend class RegressionSavedRTreeFunctionClass;
  RegressionSavedRTreeFunction() {}
};

class BinarySavedRTreeFunction : public SavedRTreeFunction
{
public:
  BinarySavedRTreeFunction(size_t numTrees,
                           size_t numAttributeSamplesPerSplit,
                           size_t minimumSizeForSplitting,
                           const File& file)
    : SavedRTreeFunction(numTrees,
                         numAttributeSamplesPerSplit,
                         minimumSizeForSplitting,
                         file) {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(probabilityType, booleanType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
  
  virtual RTreeFunctionPtr createExtraTreeImplementation() const
  {
    return new BinaryRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting, false);
  }

  virtual Variable createEmptyPrediction() const
  {
    return Variable(0.f, probabilityType);
  }

  virtual Variable addPrediction(const Variable& a, const Variable& b) const
  {
    return Variable(a.getDouble() + b.getDouble(), probabilityType);
  }
  
  virtual Variable finalizePrediction(const Variable& value) const
  {
    return Variable(value.getDouble() / (double)numTrees, probabilityType);
  }

protected:
  friend class BinarySavedRTreeFunctionClass;

  BinarySavedRTreeFunction() {}
};

class ClassificationSavedRTreeFunction : public SavedRTreeFunction
{
public:
  ClassificationSavedRTreeFunction(size_t numTrees,
                                   size_t numAttributeSamplesPerSplit,
                                   size_t minimumSizeForSplitting,
                                   const File& file)
    : SavedRTreeFunction(numTrees,
                         numAttributeSamplesPerSplit,
                         minimumSizeForSplitting,
                         file) {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(enumValueType, doubleVectorClass(enumValueType, probabilityType));}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (inputVariables[1]->getType()->inheritsFrom(enumValueType))
      return denseDoubleVectorClass(inputVariables[1]->getType(), probabilityType);
    return inputVariables[1]->getType();
  }
  
  virtual RTreeFunctionPtr createExtraTreeImplementation() const
  {
    return new ClassificationRTreeFunction(1, numAttributeSamplesPerSplit, minimumSizeForSplitting, false);
  }

  virtual Variable createEmptyPrediction() const
  {
    return new DenseDoubleVector(denseDoubleVectorClass(getOutputType()->getTemplateArgument(0), probabilityType));
  }

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
  friend class ClassificationSavedRTreeFunctionClass;

  ClassificationSavedRTreeFunction() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAVED_DECISION_TREE_R_TREE_FUNCTION_H_
