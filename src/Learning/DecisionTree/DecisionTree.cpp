/*-----------------------------------------.---------------------------------.
| Filename: DecisionTree.h                 | Decision Trees Header           |
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Learning/DecisionTree.h>
#include "RTreeFunction.h"

using namespace lbcpp;

FunctionPtr regressionExtraTree(ExecutionContext& context, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0)
{
  FunctionPtr res = new RegressionRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  res->initialize(context, doubleType);
  return res;
}

FunctionPtr binaryClassificationExtraTree(ExecutionContext& context, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0)
{
  FunctionPtr res = new BinaryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  res->initialize(context, booleanType);
  return res;
}

FunctionPtr classificationExtraTree(ExecutionContext& context, EnumerationPtr classes, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0)
{
  FunctionPtr res = new ClassificationRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  res->initialize(context, (TypePtr)classes);
  return res;
}
