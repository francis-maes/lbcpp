/*-----------------------------------------.---------------------------------.
| Filename: DecisionTree.h                 | Decision Trees Header           |
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_H_
# define LBCPP_DECISION_TREE_H_

# include "../Function/Function.h"

namespace lbcpp
{

extern FunctionPtr regressionExtraTree(ExecutionContext& context, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
extern FunctionPtr binaryClassificationExtraTree(ExecutionContext& context, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
extern FunctionPtr classificationExtraTree(ExecutionContext& context, EnumerationPtr classes, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);

}; /* namespace lbcpp */

#endif //!LBCPP_DECISION_TREE_H_
