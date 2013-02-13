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
#include "SavableRTreeFunction.h"

namespace lbcpp
{

ExtraTreesFunctionPtr regressionExtraTree(size_t numTrees,
                                          size_t numAttributeSamplesPerSplit,
                                          size_t minimumSizeForSplitting,
                                          bool useLowMemory,
                                          const File& saveTreesToFilePrefix)
{
  if (!useLowMemory)
    return new RegressionRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  if (saveTreesToFilePrefix == File::nonexistent)
    return new RegressionLowMemoryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  return new RegressionSavableRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, saveTreesToFilePrefix);
}

ExtraTreesFunctionPtr binaryClassificationExtraTree(size_t numTrees,
                                          size_t numAttributeSamplesPerSplit,
                                          size_t minimumSizeForSplitting,
                                          bool useLowMemory,
                                          const File& saveTreesToFilePrefix)
{
  if (!useLowMemory)
    return new BinaryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  if (saveTreesToFilePrefix == File::nonexistent)
    return new BinaryLowMemoryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  return new BinarySavableRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, saveTreesToFilePrefix);
}

ExtraTreesFunctionPtr classificationExtraTree(size_t numTrees,
                                    size_t numAttributeSamplesPerSplit,
                                    size_t minimumSizeForSplitting,
                                    bool useLowMemory,
                                    const File& saveTreesToFilePrefix)
{
  if (!useLowMemory)
    return new ClassificationRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  if (saveTreesToFilePrefix == File::nonexistent)
    return new ClassificationLowMemoryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
  return new ClassificationSavableRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting, saveTreesToFilePrefix);
}

};
