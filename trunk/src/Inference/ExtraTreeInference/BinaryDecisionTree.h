/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTree.h           | A class to store a binary       |
| Author  : Francis Maes                   |  decision tree                  |
| Started : 25/06/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_

# include <lbcpp/Object/Object.h>
# include <lbcpp/Object/Variable.h>

namespace lbcpp 
{


class BinaryDecisionTree : public Object
{
public:
  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  size_t addNode()
  {
    size_t res = nodes.size();
    nodes.push_back(Node()); // FIXME
    return res;
  }

protected:
  struct Node
  {
    size_t indexOfLeftChild;
    size_t splitVariable;
    VariableValue splitArgument;
  };
  std::vector<Node> nodes;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTree> BinaryDecisionTreePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
