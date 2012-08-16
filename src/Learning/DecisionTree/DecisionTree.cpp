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
#include "LowMemoryRTreeFunction.h"

namespace lbcpp
{

FunctionPtr regressionExtraTree(size_t numTrees,
                                size_t numAttributeSamplesPerSplit,
                                size_t minimumSizeForSplitting,
                                bool verbose,
                                ContainerPtr testingData)
{
  if (!testingData)
    return new RegressionRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, verbose);
  return new RegressionLowMemoryRTreeFunction(testingData, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
}

FunctionPtr binaryClassificationExtraTree(size_t numTrees,
                                          size_t numAttributeSamplesPerSplit,
                                          size_t minimumSizeForSplitting,
                                          bool verbose,
                                          ContainerPtr testingData)
{
  if (!testingData)
    return new BinaryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, verbose);
  return new BinaryLowMemoryRTreeFunction(testingData, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
}

FunctionPtr classificationExtraTree(size_t numTrees,
                                    size_t numAttributeSamplesPerSplit,
                                    size_t minimumSizeForSplitting,
                                    bool verbose,
                                    ContainerPtr testingData)
{
  if (!testingData)
    return new ClassificationRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, verbose);
  return new ClassificationLowMemoryRTreeFunction(testingData, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
}

};
