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

  Variable makePrediction(const Variable& input, size_t nodeIndex = 0) const;

protected:
  ClassPtr inputClass;
  ClassPtr leavesClass;

  struct Node
  {
    bool isInternalNode() const
      {return splitVariable >= 0;}

    bool isLeaf() const
      {return splitVariable < 0;}

    Variable getLeafValue() const
      {jassert(isLeaf()); return argument;}

    bool test(const Variable& variable) const
    {
      jassert(isInternalNode());
      jassert(splitVariable >= 0 && splitVariable < (int)variable.size());
      return variable[splitVariable] == argument; // TMP ! todo: elaborated SplitFunction
    }

    size_t getChildNodeIndex(const Variable& variable) const
      {return indexOfLeftChild + (test(variable) ? 1 : 0);}

  private:
    int splitVariable; // internal nodes: >= 0, leafs: =-1
    Variable argument; // internal nodes: split argument, leafs: answer value
    size_t indexOfLeftChild; // only for internal nodes
  };
  std::vector<Node> nodes;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTree> BinaryDecisionTreePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
