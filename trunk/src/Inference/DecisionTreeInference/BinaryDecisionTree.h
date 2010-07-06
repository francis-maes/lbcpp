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

  void createLeaf(size_t index, const Variable& value)
  {
    if (index >= nodes.size())
      nodes.resize(index + 1);
    nodes[index].setLeaf(value);
  }

  Variable makePrediction(const Variable& input, size_t nodeIndex = 0) const;

  size_t getNumNodes() const
    {return nodes.size();}

protected:
  TypePtr inputClass;
  TypePtr leavesClass;

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

    void setLeaf(const Variable& value)
      {splitVariable = -1; argument = value; indexOfLeftChild = 0;}

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
