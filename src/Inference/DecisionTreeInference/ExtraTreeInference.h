/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInference.h           | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_H_

# include "BinaryDecisionTree.h"
# include "../ReductionInference/ParallelVoteInference.h"

namespace lbcpp 
{

class SingleExtraTreeInference : public Inference
{
public:
  SingleExtraTreeInference(const String& name)
    : Inference(name) {}
  SingleExtraTreeInference()
    {}

protected:
  BinaryDecisionTreePtr tree;

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    return Variable();
  }
};

class ExtraTreeInference : public ParallelVoteInference
{
public:
  ExtraTreeInference(const String& name, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
};

typedef ReferenceCountedObjectPtr<ExtraTreeInference> ExtraTreeInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_H_
