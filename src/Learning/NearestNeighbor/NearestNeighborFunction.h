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

namespace lbcpp
{

class NearestNeighborBatchLearner;
extern BatchLearnerPtr nearestNeighborBatchLearner();

class NearestNeighborFunction : public Function
{
public:
  NearestNeighborFunction(size_t numNeighbors = 1)
    : numNeighbors(numNeighbors)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(nearestNeighborBatchLearner()));}

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
  std::vector<SparseDoubleVectorPtr> inputData;
  std::vector<Variable> supervisionData;

  virtual Variable computeOuput(ScoresMap& scoredIndices) const = 0;
};

extern ClassPtr nearestNeighborFunctionClass;
typedef ReferenceCountedObjectPtr<NearestNeighborFunction> NearestNeighborFunctionPtr;

class BinaryNearestNeighborFunction : public NearestNeighborFunction
{
public:
  BinaryNearestNeighborFunction(size_t numNeighbors = 1, bool useWeightedScore = false)
    : NearestNeighborFunction(numNeighbors), useWeightedScore(useWeightedScore) {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

protected:
  friend class BinaryNearestNeighborFunctionClass;

  bool useWeightedScore;

  virtual Variable computeOuput(ScoresMap& scoredIndices) const;
};

class NearestNeighborBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return nearestNeighborFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NEAREST_NEIGHBOR_FUNCTION_H_
