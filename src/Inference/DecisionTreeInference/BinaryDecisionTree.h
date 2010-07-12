/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTree.h           | A class to store a binary       |
| Author  : Francis Maes                   |  decision tree                  |
| Started : 25/06/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_

# include <lbcpp/Data/Object.h>
# include <lbcpp/Data/Variable.h>
# include <lbcpp/Data/Predicate.h>

namespace lbcpp 
{

class BinaryDecisionTree : public Object
{
public:
  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  size_t allocateNodes(size_t count)
    {size_t res = nodes.size(); nodes.resize(res + count); return res;}

  void createLeaf(size_t index, const Variable& value)
    {jassert(index < nodes.size()); nodes[index].setLeaf(value);}

  void createInternalNode(size_t index, size_t splitVariable, const Variable& splitArgument, size_t indexOfLeftChild)
    {jassert(index < nodes.size()); nodes[index].setInternalNode(splitVariable, splitArgument, indexOfLeftChild);}

  Variable makePrediction(const Variable& input, size_t nodeIndex = 0) const;

  size_t getNumNodes() const
    {return nodes.size();}

  static PredicatePtr getSplitPredicate(const Variable& splitArgument);
  
  String toString() const;

protected:
  struct Node
  {
    bool isInternalNode() const
      {return splitVariable >= 0;}

    bool isLeaf() const
      {return splitVariable < 0;}

    Variable getLeafValue() const
      {jassert(isLeaf()); return argument;}

    bool test(const Variable& variable) const;

    size_t getChildNodeIndex(const Variable& variable) const
      {return indexOfLeftChild + (test(variable) ? 1 : 0);}

    void setLeaf(const Variable& value)
      {splitVariable = -1; argument = value; indexOfLeftChild = 0;}

    void setInternalNode(size_t splitVariable, const Variable& splitArgument, size_t indexOfLeftChild)
      {this->splitVariable = (int)splitVariable; argument = splitArgument; this->indexOfLeftChild = indexOfLeftChild;}

    size_t getIndexOfLeftChild() const
      {jassert(isInternalNode()); return indexOfLeftChild;}
    
    size_t getIndexOfRightChild() const
      {jassert(isInternalNode()); return indexOfLeftChild + 1;}
    
    String toString() const;
    
  private:
    int splitVariable; // internal nodes: >= 0, leafs: =-1
    Variable argument; // internal nodes: split argument, leafs: answer value
    size_t indexOfLeftChild; // only for internal nodes
  };
  std::vector<Node> nodes;

private:
  String toStringRecursive(size_t nodeIndex = 0, String indent = String::empty) const;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTree> BinaryDecisionTreePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
