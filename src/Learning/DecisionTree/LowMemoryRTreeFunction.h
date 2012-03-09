/*-----------------------------------------.---------------------------------.
| Filename: LowMemoryRTreeFunction.h       | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 09/03/2012 09:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/DecisionTree.h>

namespace lbcpp
{
// Container[Pair[TrainFeature, TrainSupervision]], Container[Pair[TestInput, Any]] -> Container[Prediction]
class LowMemoryRTreeFunction : public Function
{
public:
  LowMemoryRTreeFunction(size_t numTrees,
                         size_t numAttributeSamplesPerSplit,
                         size_t minimumSizeForSplitting)
    : numTrees(numTrees),
      numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting)
    {}

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (TypePtr)containerClass(pairClass(anyType, doubleType));}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return containerClass(probabilityType);}

  virtual String getOutputPostFix() const
    {return T("LowRTree");}  

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ContainerPtr train = inputs[0].getObjectAndCast<Container>();
    ContainerPtr test = inputs[1].getObjectAndCast<Container>();
    jassert(train && test);
    if (train->getNumElements() == 0 && test->getNumElements() == 0)
    {
      context.errorCallback(T("LowMemoryRTreeFunction"), T("No data !"));
      return ContainerPtr();
    }
    const size_t numTest = test->getNumElements();
    DenseDoubleVectorPtr predictions = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, probabilityType, numTest, 0.f);
    context.enterScope(T("Low Memory RTree Learning"));
    for (size_t i = 0; i < numTrees; ++i)
    {
      context.progressCallback(new ProgressionState(i, numTrees, T("tree")));
      FunctionPtr x3Function = extraTreeLearningMachine(1, numAttributeSamplesPerSplit, minimumSizeForSplitting);
      context.enterScope(T("Sub tree ") + String((int)i + 1));
      if (!x3Function->train(context, train))
      {
        context.errorCallback(T("LowMemoryRTreeFunction"), T("Error during learning"));
        return ContainerPtr();
      }
      context.leaveScope();

      for (size_t j = 0; j < numTest; ++j)
        predictions->setValue(j, predictions->getValue(j) + x3Function->compute(context, test->getElement(j).getObjectAndCast<Pair>()->getFirst(), Variable()).getDouble());
    }
    context.progressCallback(new ProgressionState(numTrees, numTrees, T("tree")));

    for (size_t i = 0; i < numTest; ++i)
      predictions->setValue(i, predictions->getValue(i) / numTrees);

    context.leaveScope();
    return predictions;
  }

protected:
  friend class LowMemoryRTreeFunctionClass;
  
  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  LowMemoryRTreeFunction() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LOW_MEMORY_DECISION_TREE_R_TREE_FUNCTION_H_
