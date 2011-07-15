/*-----------------------------------------.---------------------------------.
| Filename: DecisionTree.h                 | Decision Trees Header           |
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Learning/DecisionTree.h>
#include "RTreeFunction.h"

namespace lbcpp
{

FunctionPtr regressionExtraTree(ExecutionContext& context, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  FunctionPtr res = new RegressionRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  res->initialize(context, (TypePtr)containerClass(anyType), doubleType);
  return res;
}

FunctionPtr binaryClassificationExtraTree(ExecutionContext& context, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  FunctionPtr res = new BinaryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  res->initialize(context, (TypePtr)containerClass(anyType), booleanType);
  return res;
}

FunctionPtr classificationExtraTree(ExecutionContext& context, EnumerationPtr classes, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  FunctionPtr res = new ClassificationRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  res->initialize(context, (TypePtr)containerClass(anyType), (TypePtr)classes);
  return res;
}

};
