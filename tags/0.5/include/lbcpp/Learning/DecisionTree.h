/*-----------------------------------------.---------------------------------.
| Filename: DecisionTree.h                 | Decision Trees Header           |
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_H_
# define LBCPP_DECISION_TREE_H_

# include "../Core/Function.h"

namespace lbcpp
{

extern FunctionPtr regressionExtraTree(size_t numTrees = 100,
                                       size_t numAttributeSamplesPerSplit = 10,
                                       size_t minimumSizeForSplitting = 0,
                                       bool verbose = false,
                                       bool useLowMemory = false);

extern FunctionPtr binaryClassificationExtraTree(size_t numTrees = 100,
                                                 size_t numAttributeSamplesPerSplit = 10,
                                                 size_t minimumSizeForSplitting = 0,
                                                 bool verbose = false,
                                                 bool useLowMemory = false);

extern FunctionPtr classificationExtraTree(size_t numTrees = 100,
                                           size_t numAttributeSamplesPerSplit = 10,
                                           size_t minimumSizeForSplitting = 0,
                                           bool verbose = false,
                                           bool useLowMemory = false);

extern FunctionPtr extraTreeLearningMachine(size_t numTrees = 100,
                                            size_t numAttributeSamplesPerSplit = 10,
                                            size_t minimumSizeForSplitting = 0,
                                            bool verbose = false,
                                            bool useLowMemory = false);

}; /* namespace lbcpp */

#endif //!LBCPP_DECISION_TREE_H_
