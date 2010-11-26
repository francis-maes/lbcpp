/*-----------------------------------------.---------------------------------.
 | Filename: BinaryDecisionTreeSplitter.h  | BinaryDecisionTreeSplitter      |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 13:19              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_INFERENCE_SPLITTER_H_
# define LBCPP_DECISION_TREE_INFERENCE_SPLITTER_H_

# include <lbcpp/lbcpp.h>
# include "SplitScoringFunction.h"

namespace lbcpp
{

class BinaryDecisionTreeSplitter : public Object
{
public:
  BinaryDecisionTreeSplitter(SplitScoringFunctionPtr scoringFunction = SplitScoringFunctionPtr(), RandomGeneratorPtr random = RandomGeneratorPtr(), size_t variableIndex = 0)
    : scoringFunction(scoringFunction), random(random), variableIndex(variableIndex) {}
  
  virtual Variable findBestSplit(ContainerPtr data) const = 0;
  
  virtual Variable sampleSplit(ContainerPtr data) const = 0;
  
  virtual PredicatePtr getSplitPredicate(const Variable& splitArgument) const = 0;

  double computeSplitScore(ExecutionContext& context,
                           ContainerPtr data, ContainerPtr& negativeExamples, ContainerPtr& positiveExamples,
                           PredicatePtr predicate) const;

protected:
  friend class BinaryDecisionTreeSplitterClass;
  
  SplitScoringFunctionPtr scoringFunction;
  RandomGeneratorPtr random;
  size_t variableIndex;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTreeSplitter> BinaryDecisionTreeSplitterPtr;

class BooleanBinaryDecisionTreeSplitter : public BinaryDecisionTreeSplitter
{
public:
  BooleanBinaryDecisionTreeSplitter(SplitScoringFunctionPtr scoringFunction, RandomGeneratorPtr random = RandomGeneratorPtr(), size_t variableIndex = 0)
  : BinaryDecisionTreeSplitter(scoringFunction, random, variableIndex) {}

  virtual Variable findBestSplit(ContainerPtr data) const
    {jassertfalse; return Variable();}
  
  virtual Variable sampleSplit(ContainerPtr data) const
    {return Variable(random->sampleBool(), booleanType);}

  virtual PredicatePtr getSplitPredicate(const Variable& splitArgument) const
    {return equalToPredicate(splitArgument);}
};

class DoubleBinaryDecisionTreeSplitter : public BinaryDecisionTreeSplitter
{
public:
  DoubleBinaryDecisionTreeSplitter(SplitScoringFunctionPtr scoringFunction, RandomGeneratorPtr random = RandomGeneratorPtr(), size_t variableIndex = 0)
  : BinaryDecisionTreeSplitter(scoringFunction, random, variableIndex) {}

  virtual Variable findBestSplit(ContainerPtr data) const
    {jassertfalse; return Variable();}
  
  virtual Variable sampleSplit(ContainerPtr data) const;
  
  virtual PredicatePtr getSplitPredicate(const Variable& splitArgument) const
    {return lessThanOrEqualToPredicate(splitArgument);}
};

class IntegereBinaryDecisionTreeSplitter : public BinaryDecisionTreeSplitter
{
public:
  IntegereBinaryDecisionTreeSplitter(SplitScoringFunctionPtr scoringFunction, RandomGeneratorPtr random = RandomGeneratorPtr(), size_t variableIndex = 0)
  : BinaryDecisionTreeSplitter(scoringFunction, random, variableIndex) {}
  
  virtual Variable findBestSplit(ContainerPtr data) const
  {jassertfalse; return Variable();}
  
  virtual Variable sampleSplit(ContainerPtr data) const;
  
  virtual PredicatePtr getSplitPredicate(const Variable& splitArgument) const
    {return lessThanOrEqualToPredicate(splitArgument);}
};

class EnumerationBinaryDecisionTreeSplitter : public BinaryDecisionTreeSplitter
{
public:
  EnumerationBinaryDecisionTreeSplitter(SplitScoringFunctionPtr scoringFunction, RandomGeneratorPtr random = RandomGeneratorPtr(), size_t variableIndex = 0)
  : BinaryDecisionTreeSplitter(scoringFunction, random, variableIndex) {}

  virtual Variable findBestSplit(ContainerPtr data) const
    {jassertfalse; return Variable();}
  
  virtual Variable sampleSplit(ContainerPtr data) const;
  
  virtual PredicatePtr getSplitPredicate(const Variable& splitArgument) const;
};

};

#endif //!LBCPP_DECISION_TREE_INFERENCE_SPLITTER_H_
