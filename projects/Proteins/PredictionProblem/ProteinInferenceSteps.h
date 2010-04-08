/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceStep.h     | Protein related prediction      |
| Author  : Francis Maes                   |   problems                      |
| Started : 08/04/2010 18:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
# define LBCPP_PROTEIN_PREDICTION_PROBLEM_H_

# include "InferenceStep.h"
# include "ReductionInferenceStep.h"

namespace lbcpp
{


// ObjectContainer => ObjectContainer

/*
class SharedParallelFunction : public ObjectFunction
{
public:
  SharedParallelFunction(SharedParallelInferenceStepPtr problem)
    : problem(problem) {}

  virtual String getInputClassName() const
    {return T("FeatureGenerator");}
  
  virtual String getOutputClassName(const String& ) const
    {return T("ObjectContainer");}

  virtual ObjectPtr function(ObjectPtr object) const
  {
    

  }

protected:
  ClassifierPtr classifier;
};
*/

// Input: FeatureGenerator
// Output: FeatureVector
// Training Data: Input/Output pairs
// Inference: Input -> Output
class MultiClassInferenceStep : public AtomicInferenceStep
{
public:
  MultiClassInferenceStep(const String& name) : AtomicInferenceStep(name) {}

  virtual ObjectFunctionPtr trainPredictor(ObjectContainerPtr trainingData) const
  {
    return ObjectFunctionPtr();
  }
};

// Input: (index -> features) function
// Output: inherited from ObjectContainer 
// Training Data: Input/Output pairs
// Inference: Input -> Output
class SharedParallelReductionProblem : public ReductionInferenceStep
{
public:
  SharedParallelReductionProblem(const String& name)
    : ReductionInferenceStep(name) {}
  
  virtual size_t getNumSubObjects(ObjectPtr object) const = 0;
  virtual ObjectPtr getSubObject(size_t i) const = 0;
};

// Training Data: Proteins
// Inference: Protein -> SecondaryStructureSequence
class SS3InferenceStep : public SharedParallelReductionProblem
{
public:

};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
