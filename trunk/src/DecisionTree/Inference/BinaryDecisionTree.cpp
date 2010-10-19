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

#include <lbcpp/Function/Predicate.h>
#include <lbcpp/Data/Vector.h>
class BelongsToMaskPredicate : public Predicate
{
public:
  BelongsToMaskPredicate(BooleanVectorPtr mask)
    : mask(mask) {}

  virtual String toString() const
    {return T("BelongsToMask(") + mask->toString() + T(")");}

  virtual TypePtr getInputType() const
    {return integerType;}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
  {
    if (value.isMissingValue())
      return mask->get(mask->getNumElements() - 1);
    size_t i = (size_t)value.getInteger();
    jassert(i < mask->getNumElements() - 1);
    return mask->get(i);
 }

private:
  BooleanVectorPtr mask;
};

PredicatePtr BinaryDecisionTree::getSplitPredicate(const Variable& argument)
{
  if (argument.isDouble() || argument.isInteger())
    return lessThanOrEqualToPredicate(argument);

  jassert(argument.isObject());

  PredicatePtr predicate = argument.dynamicCast<Predicate>();
  if (predicate)
    return predicate;
    
  BooleanVectorPtr mask = argument.dynamicCast<BooleanVector>();
  if (mask)
    return new BelongsToMaskPredicate(mask);

  jassert(false);
  return PredicatePtr();
}

bool BinaryDecisionTree::Node::test(const Variable& variable) const
{
  jassert(isInternalNode());
  jassert(splitVariable >= 0 && splitVariable < (int)variable.getObject()->getNumVariables());
  return getSplitPredicate(argument)->computePredicate(variable.getObject()->getVariable(splitVariable));
}

String BinaryDecisionTree::toString() const
{
  jassert(nodes.size());
  return T("BinaryDecisionTree ") + String((int)nodes.size()) + T(" nodes\n") + toStringRecursive(); 
}

String BinaryDecisionTree::Node::toString() const
{
  if (isLeaf())
    return T("Value: ") + argument.toString();
  else
  {
    PredicatePtr predicate = getSplitPredicate(argument);
    jassert(predicate);
    return T("SplitVariable ") + String(splitVariable)
      + T(" Predicate = ") + predicate->toString();
  }
}

String BinaryDecisionTree::toStringRecursive(size_t index, String indent) const
{
  jassert(index >= 0);
  const Node currentNode = nodes[index];
  
  String res = indent + String((int)index) + T(" ") + currentNode.toString() + T("\n");
  if (currentNode.isLeaf())
    return res;
  
  indent = indent.replaceCharacter('-', ' ') + T("|- ");
  return res
       + toStringRecursive(currentNode.getIndexOfLeftChild(), indent)
       + toStringRecursive(currentNode.getIndexOfRightChild(), indent);
} 

