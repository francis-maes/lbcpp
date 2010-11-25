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
# include <lbcpp/Function/Predicate.h>
# include "BinaryDecisionTreeSplitter.h"

namespace lbcpp 
{

class BinaryDecisionTree : public Object
{
public:
  BinaryDecisionTree(size_t numVariables = 0) : splitters(numVariables) {}
  
  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  size_t allocateNodes(size_t count)
    {size_t res = nodes.size(); nodes.resize(res + count); return res;}

  void createLeaf(size_t index, const Variable& value)
    {jassert(index < nodes.size()); setLeaf(nodes[index], value);}

  void createInternalNode(size_t index, size_t splitVariable, const Variable& splitArgument, size_t indexOfLeftChild)
    {jassert(index < nodes.size()); setInternalNode(nodes[index], splitVariable, splitArgument, indexOfLeftChild);}

  Variable makePrediction(ExecutionContext& context, const Variable& input, size_t nodeIndex = 0) const;

  size_t getNumNodes() const
    {return nodes.size();}
  
  String toString() const;
  
  BinaryDecisionTreeSplitterPtr getSplitter(size_t variableIndex) const
    {jassert(variableIndex < splitters.size()); return splitters[variableIndex];}
  
  void setSplitter(size_t variableIndex, BinaryDecisionTreeSplitterPtr splitter)
    {jassert(variableIndex < splitters.size()); splitters[variableIndex] = splitter;}

protected:
  std::vector<BinaryDecisionTreeSplitterPtr> splitters;
  
  struct Node
  {
    int splitVariable; // internal nodes: >= 0, leafs: =-1
    Variable argument; // internal nodes: split argument, leafs: answer value
    size_t indexOfLeftChild; // only for internal nodes
  };
  std::vector<Node> nodes;
  
  /* Node's functions */
  PredicatePtr getSplitPredicate(const Node& node) const;
  
  bool isInternalNode(const Node& node) const
    {return node.splitVariable >= 0;}
  
  bool isLeaf(const Node& node) const
    {return node.splitVariable < 0;}
  
  Variable getLeafValue(const Node& node) const
    {jassert(isLeaf(node)); return node.argument;}
  
  bool test(ExecutionContext& context, const Node& node, const Variable& variable) const;
  
  size_t getChildNodeIndex(ExecutionContext& context, const Node& node, const Variable& variable) const
    {return node.indexOfLeftChild + (test(context, node, variable) ? 1 : 0);}
  
  void setLeaf(Node& node, const Variable& value)
    {node.splitVariable = -1; node.argument = value; node.indexOfLeftChild = 0;}
  
  void setInternalNode(Node& node, size_t splitVariable, const Variable& splitArgument, size_t indexOfLeftChild)
    {node.splitVariable = (int)splitVariable; node.argument = splitArgument; node.indexOfLeftChild = indexOfLeftChild;}
  
  size_t getIndexOfLeftChild(const Node& node) const
    {jassert(isInternalNode(node)); return node.indexOfLeftChild;}
  
  size_t getIndexOfRightChild(const Node& node) const
    {jassert(isInternalNode(node)); return node.indexOfLeftChild + 1;}
  
  String toString(const Node& node) const;

private:
  String toStringRecursive(size_t nodeIndex = 0, String indent = String::empty) const;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTree> BinaryDecisionTreePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
