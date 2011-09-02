/*-----------------------------------------.---------------------------------.
| Filename: NearestNeighborFunction.h      | Nearest Neighbor                |
| Author  : Julien Becker                  |                                 |
| Started : 04/08/2012 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NEAREST_NEIGHBOR_FUNCTION_H_
# define LBCPP_NEAREST_NEIGHBOR_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/NearestNeighbor.h>

namespace lbcpp
{

class NearestNeighborBatchLearner;
extern BatchLearnerPtr nearestNeighborBatchLearner(bool autoNormalizeFeatures = false);

class NearestNeighborFunction : public Function
{
public:
  NearestNeighborFunction(size_t numNeighbors = 1, bool autoNormalizeFeatures = false, bool includeTheNearestNeighbor = true)
    : numNeighbors(numNeighbors)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(nearestNeighborBatchLearner(autoNormalizeFeatures)));}

  virtual TypePtr getSupervisionType() const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass() : getSupervisionType();}

  virtual String getOutputPostFix() const
    {return String((int)numNeighbors) + T("-NN");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class NearestNeighborFunctionClass;
  friend class NearestNeighborBatchLearner;

  typedef std::multimap<double, size_t> ScoresMap;

  size_t numNeighbors;
  bool includeTheNearestNeighbor;

  std::vector<SparseDoubleVectorPtr> inputData;
  std::vector<Variable> supervisionData;
  DenseDoubleVectorPtr normalizationFactors;

  virtual Variable computeOuput(ScoresMap& scoredIndices) const = 0;
};

extern ClassPtr nearestNeighborFunctionClass;
typedef ReferenceCountedObjectPtr<NearestNeighborFunction> NearestNeighborFunctionPtr;

class RegressionNearestNeighbor : public NearestNeighborFunction
{
public:
  RegressionNearestNeighbor(size_t numNeighbors, bool autoNormalizeFeatures, bool includeTheNearestNeighbor)
    : NearestNeighborFunction(numNeighbors, autoNormalizeFeatures, includeTheNearestNeighbor) {}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

protected:
  friend class RegressionNearestNeighborClass;

  RegressionNearestNeighbor() {}

  virtual Variable computeOuput(ScoresMap& scoredIndices) const;
};

class BinaryNearestNeighbor : public NearestNeighborFunction
{
public:
  BinaryNearestNeighbor(size_t numNeighbors, bool autoNormalizeFeatures, bool includeTheNearestNeighbor, bool useWeightedScore)
    : NearestNeighborFunction(numNeighbors, autoNormalizeFeatures, includeTheNearestNeighbor), useWeightedScore(useWeightedScore) {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

protected:
  friend class BinaryNearestNeighborClass;

  bool useWeightedScore;

  BinaryNearestNeighbor() {}

  virtual Variable computeOuput(ScoresMap& scoredIndices) const;
};

class ClassificationNearestNeighbor : public NearestNeighborFunction
{
public:
  ClassificationNearestNeighbor(size_t numNeighbors, bool autoNormalizeFeatures, bool includeTheNearestNeighbor)
    : NearestNeighborFunction(numNeighbors, autoNormalizeFeatures, includeTheNearestNeighbor) {}

  virtual TypePtr getSupervisionType() const
    {return doubleVectorClass(enumValueType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    enumeration = inputVariables[1]->getType()->getTemplateArgument(0).staticCast<Enumeration>();
    return denseDoubleVectorClass(enumeration, probabilityType);
  }

protected:
  friend class ClassificationNearestNeighborClass;

  EnumerationPtr enumeration;

  ClassificationNearestNeighbor() {}

  virtual Variable computeOuput(ScoresMap& scoredIndices) const;
};

class NearestNeighborBatchLearner : public BatchLearner
{
public:
  NearestNeighborBatchLearner(bool autoNormalizeFeatures = false)
    : autoNormalizeFeatures(autoNormalizeFeatures) {}

  virtual TypePtr getRequiredFunctionType() const
    {return nearestNeighborFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class NearestNeighborBatchLearnerClass;

  bool autoNormalizeFeatures;
};

class NearestNeighborLearningMachine : public ProxyFunction
{
public:
  NearestNeighborLearningMachine(size_t numNeighbors, bool autoNormalizeFeatures, bool includeTheNearestNeighbor)
    : numNeighbors(numNeighbors), autoNormalizeFeatures(autoNormalizeFeatures), includeTheNearestNeighbor(includeTheNearestNeighbor) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass() : anyType;}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    const TypePtr supervisionType = inputVariables[1]->getType();

    if (supervisionType == doubleType)
      return regressionNearestNeighbor(numNeighbors, autoNormalizeFeatures, includeTheNearestNeighbor);
    else if (supervisionType == probabilityType || supervisionType == booleanType)
      return binaryNearestNeighbor(numNeighbors, autoNormalizeFeatures, includeTheNearestNeighbor, false);
    else if (supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return classificationNearestNeighbor(numNeighbors, autoNormalizeFeatures, includeTheNearestNeighbor);
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }

protected:
  friend class NearestNeighborLearningMachineClass;

  NearestNeighborLearningMachine() {}

  size_t numNeighbors;
  bool autoNormalizeFeatures;
  bool includeTheNearestNeighbor;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NEAREST_NEIGHBOR_FUNCTION_H_
