/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTree.cpp         | A class to store a binary       |
| Author  : Francis Maes                   |  decision tree                  |
| Started : 28/06/2010 17:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "BinaryDecisionTree.h"
using namespace lbcpp;

Variable BinaryDecisionTree::makePrediction(const Variable& input, size_t nodeIndex) const
{
  jassert(nodeIndex < nodes.size());
  const Node& node = nodes[nodeIndex];
  return node.isLeaf() ? node.getLeafValue() : makePrediction(input, node.getChildNodeIndex(input));
}
