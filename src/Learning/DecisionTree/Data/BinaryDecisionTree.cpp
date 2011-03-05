/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTree.cpp         | A class to store a binary       |
| Author  : Francis Maes                   |  decision tree                  |
| Started : 28/06/2010 17:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "BinaryDecisionTree.h"
using namespace lbcpp;

Variable BinaryDecisionTree::makePrediction(ExecutionContext& context, const Variable& input, size_t nodeIndex) const
{
  jassert(nodeIndex < nodes.size());
  const Node& node = nodes[nodeIndex];
  return isLeaf(node) ? getLeafValue(node) : makePrediction(context, input, getChildNodeIndex(context, node, input));
}

PredicatePtr BinaryDecisionTree::getSplitPredicate(const Node& node) const
{
  BinaryDecisionTreeSplitterPtr splitter = getSplitter(node.splitVariable);
  return splitter->getSplitPredicate(node.argument);
}

bool BinaryDecisionTree::test(ExecutionContext& context, const Node& node, const Variable& variable) const
{
  jassert(isInternalNode(node));
  jassert(node.splitVariable >= 0 && node.splitVariable < (int)variable.getObject()->getNumVariables());
  return getSplitPredicate(node)->computePredicate(context, variable.getObject()->getVariable(node.splitVariable));
}

String BinaryDecisionTree::toString() const
{
  jassert(nodes.size());
  return T("BinaryDecisionTree ") + String((int)nodes.size()) + T(" nodes\n") + toStringRecursive(); 
}

String BinaryDecisionTree::toString(const Node& node) const
{
  if (isLeaf(node))
    return T("Value: ") + node.argument.toString();
  else
  {
    PredicatePtr predicate = getSplitPredicate(node);
    jassert(predicate);
    return T("SplitVariable ") + String(node.splitVariable)
      + T(" Predicate = ") + predicate->toString();
  }
}

String BinaryDecisionTree::toStringRecursive(size_t index, String indent) const
{
  jassert(index >= 0);
  const Node currentNode = nodes[index];
  
  String res = indent + String((int)index) + T(" ") + toString(currentNode) + T("\n");
  if (isLeaf(currentNode))
    return res;
  
  indent = indent.replaceCharacter('-', ' ') + T("|- ");
  return res
       + toStringRecursive(getIndexOfLeftChild(currentNode), indent)
       + toStringRecursive(getIndexOfRightChild(currentNode), indent);
} 
