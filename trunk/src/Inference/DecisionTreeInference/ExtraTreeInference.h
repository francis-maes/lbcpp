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
# include "../ReductionInference/ParallelVoteInference.h"
# include <lbcpp/Data/ProbabilityDistribution.h>

namespace lbcpp 
{

class BinaryDecisionTreeInference : public Inference
{
public:
  BinaryDecisionTreeInference(const String& name, TypePtr inputType)
    : Inference(name), inputType(inputType) {}
  BinaryDecisionTreeInference()
    {}

  virtual TypePtr getInputType() const
    {return inputType;}

  BinaryDecisionTreePtr getTree() const
    {return tree;}

  void setTree(BinaryDecisionTreePtr tree)
    {this->tree = tree;}

  virtual void clone(ObjectPtr target) const
  {
    Inference::clone(target);
    ReferenceCountedObjectPtr<BinaryDecisionTreeInference> res = target.staticCast<BinaryDecisionTreeInference>();
    res->inputType = inputType;
    if (tree)
      res->tree = tree->clone();
  }

protected:
  friend class BinaryDecisionTreeInferenceClass;

  BinaryDecisionTreePtr tree;
  TypePtr inputType;

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return tree && tree->getNumNodes() ? tree->makePrediction(input) : Variable();}
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTreeInference> BinaryDecisionTreeInferencePtr;

class RegressionBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  RegressionBinaryDecisionTreeInference(const String& name, TypePtr inputType)
    : BinaryDecisionTreeInference(name, inputType) {}
  RegressionBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return doubleType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType();}
};

class BinaryClassificationBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  BinaryClassificationBinaryDecisionTreeInference(const String& name, TypePtr inputType)
    : BinaryDecisionTreeInference(name, inputType) {}
  BinaryClassificationBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return booleanType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return booleanType();}
};

class ClassificationBinaryDecisionTreeInference : public BinaryDecisionTreeInference
{
public:
  ClassificationBinaryDecisionTreeInference(const String& name, TypePtr inputType, EnumerationPtr classes)
    : BinaryDecisionTreeInference(name, inputType), classes(classes) {}
  ClassificationBinaryDecisionTreeInference() {}

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return discreteProbabilityDistributionClass(classes);}

protected:
  EnumerationPtr classes;
};

class ExtraTreeInference : public ParallelVoteInference
{
public:
  ExtraTreeInference(const String& name, BinaryDecisionTreeInferencePtr decisionTreeModel, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
  ExtraTreeInference() {}
};

typedef ReferenceCountedObjectPtr<ExtraTreeInference> ExtraTreeInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_H_
