
#ifndef LBCPP_DECISION_TREE_R_TREE_INFERENCE_H_
# define LBCPP_DECISION_TREE_R_TREE_INFERENCE_H_

# include "predeclarations.h"
# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class RTreeInference : public Inference
{
public:
  RTreeInference(const String& name, PerceptionPtr perception, size_t numTrees,
                 size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting);
  RTreeInference() {}
  
  virtual TypePtr getInputType() const
    {return perception->getInputType();}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;} // Depend of the implemetation
  
  void setTrees(RTreePtr trees)
    {this->trees = trees;}
  
  PerceptionPtr getPerception() const
    {return perception;}
  
  size_t getNumTrees() const
    {return numTrees;}
  
  size_t getNumAttributeSamplesPerSplit() const
    {return numAttributeSamplesPerSplit;}
  
  size_t getMinimumSizeForSplitting() const
    {return minimumSizeForSplitting;}

protected:
  friend class RTreeInferenceClass;
  
  RTreePtr trees;
  PerceptionPtr perception;
  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
  
  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;
};

class ClassificationRTreeInference : public RTreeInference
{
public:
  ClassificationRTreeInference(const String& name, PerceptionPtr perception, EnumerationPtr classes, size_t numTrees,
                size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : RTreeInference(name, perception, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting), enumeration(classes) {}
  ClassificationRTreeInference() {}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return enumeration;}
  
  virtual TypePtr getSupervisionType() const
    {return enumeration;}
  
protected:
  friend class ClassificationRTreeInferenceClass;
  
  EnumerationPtr enumeration;
};
  
class RegressionRTreeInference : public RTreeInference
{
public:
  RegressionRTreeInference(const String& name, PerceptionPtr perception, size_t numTrees,
                 size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : RTreeInference(name, perception, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting) {}
  RegressionRTreeInference() {}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
};

class BinaryRTreeInference : public RTreeInference
{
public:
  BinaryRTreeInference(const String& name, PerceptionPtr perception, size_t numTrees,
                           size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : RTreeInference(name, perception, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting) {}
  BinaryRTreeInference() {}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return probabilityType;}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
};

extern ClassPtr rTreeInferenceClass;

extern InferencePtr rTreeInferenceLearner();

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_INFERENCE_H_
