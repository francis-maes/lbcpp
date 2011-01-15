/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInference.h           | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_H_

# include "BinaryDecisionTree.h"
# include "../../Inference/ReductionInference/ParallelVoteInference.h"
# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp 
{

class BinaryDecisionTreeInference : public Inference
{
public:
  BinaryDecisionTreeInference(const String& name, PerceptionPtr perception)
    : Inference(name), perception(perception) {}
  BinaryDecisionTreeInference()
    {}

  virtual TypePtr getInputType() const
    {return perception->getInputType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  BinaryDecisionTreePtr getTree() const
    {return tree;}

  void setTree(BinaryDecisionTreePtr tree)
    {this->tree = tree;}

  PerceptionPtr getPerception() const
    {return perception;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Inference::clone(context, target);
    ReferenceCountedObjectPtr<BinaryDecisionTreeInference> res = target.staticCast<BinaryDecisionTreeInference>();
    if (tree)
      res->tree = tree->cloneAndCast<BinaryDecisionTree>(context);
  }

protected:
  friend class BinaryDecisionTreeInferenceClass;

  PerceptionPtr perception;
  BinaryDecisionTreePtr tree;

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
    {return tree && tree->getNumNodes() ? tree->makePrediction(context, perception->computeFunction(context, input)) : Variable::missingValue(getOutputType(input.getType()));}
};

extern ClassPtr binaryDecisionTreeInferenceClass;
typedef ReferenceCountedObjectPtr<BinaryDecisionTreeInference> BinaryDecisionTreeInferencePtr;

class RegressionBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  RegressionBinaryDecisionTreeInference(const String& name, PerceptionPtr perception)
    : BinaryDecisionTreeInference(name, perception) {}
  RegressionBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}
};

class BinaryClassificationBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  BinaryClassificationBinaryDecisionTreeInference(const String& name, PerceptionPtr perception)
    : BinaryDecisionTreeInference(name, perception) {}
  BinaryClassificationBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return booleanType;}
};

class ClassificationBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  ClassificationBinaryDecisionTreeInference(const String& name, PerceptionPtr perception, EnumerationPtr classes)
    : BinaryDecisionTreeInference(name, perception), classes(classes) {}
  ClassificationBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return classes;}

protected:
  friend class ClassificationBinaryDecisionTreeInferenceClass;
  EnumerationPtr classes;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_H_
