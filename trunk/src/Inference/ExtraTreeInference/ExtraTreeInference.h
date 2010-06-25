/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInference.h           | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_H_

# include <lbcpp/Inference/Inference.h>
# include "BinaryDecisionTree.h"

namespace lbcpp 
{

class ExtraTreeInference : public Inference
{
public:
  ExtraTreeInference(const String& name, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);

  void addTree(BinaryDecisionTreePtr tree)
    {trees.push_back(tree);}

  virtual bool areOutputObjectsEqual(ObjectPtr output1, ObjectPtr output2) const
    {return output1 == output2;}

protected:
  std::vector<BinaryDecisionTreePtr> trees;

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    return ObjectPtr();
  }
};

typedef ReferenceCountedObjectPtr<ExtraTreeInference> ExtraTreeInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_H_
