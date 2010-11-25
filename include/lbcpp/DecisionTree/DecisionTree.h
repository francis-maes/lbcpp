/*-----------------------------------------.---------------------------------.
| Filename: DecisionTree.h                 | Decision Trees Header           |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 15:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_H_
# define LBCPP_DECISION_TREE_H_

# include "../Inference/Inference.h"

namespace lbcpp
{

extern InferencePtr regressionExtraTreeInference(ExecutionContext& context, const String& name, PerceptionPtr perception, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
extern InferencePtr binaryClassificationExtraTreeInference(ExecutionContext& context, const String& name, PerceptionPtr perception, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
extern InferencePtr classificationExtraTreeInference(ExecutionContext& context, const String& name, PerceptionPtr perception, EnumerationPtr classes, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);

}; /* namespace lbcpp */

#endif //!LBCPP_DECISION_TREE_H_
