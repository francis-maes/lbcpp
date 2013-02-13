/*-----------------------------------------.---------------------------------.
| Filename: RTreeFunction.h                | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/02/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_PROXY_FUNCTION_H_
# define LBCPP_DECISION_TREE_PROXY_FUNCTION_H_

# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/DecisionTree.h>

namespace lbcpp
{

class ExtraTreeLearningMachine : public ProxyFunction
{
public:
  ExtraTreeLearningMachine(size_t numTrees,
                           size_t numAttributeSamplesPerSplit,
                           size_t minimumSizeForSplitting,
                           bool useLowMemory,
                           const File& saveTreesToFilePrefix)
  : numTrees(numTrees), 
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting),
    useLowMemory(useLowMemory),
    file(saveTreesToFilePrefix) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual String getOutputPostFix() const
    {return T("x3Proxy");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass() : anyType;}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputsType = inputVariables[0]->getType();
    TypePtr supervisionType = inputVariables[1]->getType();

    if (supervisionType == doubleType)
      return regressionExtraTree(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, useLowMemory, file);
    else if (supervisionType == probabilityType || supervisionType == booleanType)
      return binaryClassificationExtraTree(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, useLowMemory, file);
    else if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return classificationExtraTree(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, useLowMemory, file);
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }

protected:
  friend class ExtraTreeLearningMachineClass;

  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
  bool useLowMemory;
  File file;

  ExtraTreeLearningMachine() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_PROXY_FUNCTION_H_
