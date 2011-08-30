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

FunctionPtr regressionExtraTree(size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  return new RegressionRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
}

FunctionPtr binaryClassificationExtraTree(size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  return new BinaryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
}

FunctionPtr classificationExtraTree(size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  return new ClassificationRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
}

};
