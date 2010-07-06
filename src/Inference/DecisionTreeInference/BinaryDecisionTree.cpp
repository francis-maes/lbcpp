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

#include <lbcpp/Object/Predicate.h>
#include <lbcpp/Object/Vector.h>
class BelongsToMaskPredicate : public Predicate
{
public:
  BelongsToMaskPredicate(BooleanVectorPtr mask)
    : mask(mask) {}

  virtual String toString() const
    {return T("BelongsToMask(") + mask->toString() + T(")");}

  virtual bool compute(const Variable& value) const
  {
    if (value)
    {
      if (!checkInheritance(value, integerType()))
        return false;
      size_t i = (size_t)value.getInteger();
      jassert(i < mask->size() - 1);
      return mask->get(i);
    }
    else
      return mask->get(mask->size() - 1);
 }

private:
  BooleanVectorPtr mask;
};

PredicatePtr BinaryDecisionTree::getSplitPredicate(const Variable& argument)
{
  if (argument.isDouble())
    return lessThanPredicate(argument.getDouble());

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
  jassert(splitVariable >= 0 && splitVariable < (int)variable.size());
  return getSplitPredicate(argument)->compute(variable[splitVariable]);
}
