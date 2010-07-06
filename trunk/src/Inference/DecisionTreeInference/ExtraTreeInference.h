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

class BinaryDecisionTreeInference : public Inference
{
public:
  BinaryDecisionTreeInference(const String& name)
    : Inference(name) {}
  BinaryDecisionTreeInference()
    {}

  BinaryDecisionTreePtr getTree() const
    {return tree;}

  void setTree(BinaryDecisionTreePtr tree)
    {this->tree = tree;}

protected:
  BinaryDecisionTreePtr tree;

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return tree && tree->getNumNodes() ? tree->makePrediction(input) : Variable();}
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTreeInference> BinaryDecisionTreeInferencePtr;

class ExtraTreeInference : public ParallelVoteInference
{
public:
  ExtraTreeInference(const String& name, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
  ExtraTreeInference() {}
};

typedef ReferenceCountedObjectPtr<ExtraTreeInference> ExtraTreeInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_H_
