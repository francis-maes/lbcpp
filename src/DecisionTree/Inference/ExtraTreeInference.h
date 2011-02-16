/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInference.h           | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_H_

# include "../../Learning/DecisionTree/Data/BinaryDecisionTree.h"
# include "../../Inference/ReductionInference/ParallelVoteInference.h"
# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp 
{

class BinaryDecisionTreeInference : public Inference
{
public:
  BinaryDecisionTreeInference(const String& name)
  : Inference(name) {}
  BinaryDecisionTreeInference()
    {}

  virtual TypePtr getInputType() const
    {return anyType;} // FIXME

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  BinaryDecisionTreePtr getTree() const
    {return tree;}

  void setTree(BinaryDecisionTreePtr tree)
    {this->tree = tree;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Inference::clone(context, target);
    ReferenceCountedObjectPtr<BinaryDecisionTreeInference> res = target.staticCast<BinaryDecisionTreeInference>();
    if (tree)
      res->tree = tree->cloneAndCast<BinaryDecisionTree>(context);
  }

protected:
  friend class BinaryDecisionTreeInferenceClass;

  BinaryDecisionTreePtr tree;

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
    {return tree && tree->getNumNodes() ? tree->makePrediction(context, input) : Variable::missingValue(getOutputType(input.getType()));}
};

extern ClassPtr binaryDecisionTreeInferenceClass;
typedef ReferenceCountedObjectPtr<BinaryDecisionTreeInference> BinaryDecisionTreeInferencePtr;

class RegressionBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  RegressionBinaryDecisionTreeInference(const String& name)
    : BinaryDecisionTreeInference(name) {}
  RegressionBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}
};

class BinaryClassificationBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  BinaryClassificationBinaryDecisionTreeInference(const String& name)
    : BinaryDecisionTreeInference(name) {}
  BinaryClassificationBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return probabilityType;}
};

class ClassificationBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  ClassificationBinaryDecisionTreeInference(const String& name, EnumerationPtr classes)
    : BinaryDecisionTreeInference(name), classes(classes) {}
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
