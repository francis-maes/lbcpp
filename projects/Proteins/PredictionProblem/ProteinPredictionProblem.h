/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictionProblem.h     | Protein related prediction      |
| Author  : Francis Maes                   |   problems                      |
| Started : 08/04/2010 18:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
# define LBCPP_PROTEIN_PREDICTION_PROBLEM_H_

# include "PredictionProblem.h"
# include "ReductionPredictionProblem.h"

namespace lbcpp
{

// FeatureGenerator => FeatureVector
class MultiClassPredictionProblem : public AtomicPredictionProblem
{
public:
  MultiClassPredictionProblem(const String& name) : AtomicPredictionProblem(name) {}

  virtual ObjectFunctionPtr trainPredictor(ObjectContainerPtr trainingData) const
  {
    return ObjectFunctionPtr();
  }
};

// ObjectContainer => ObjectContainer

/*
class SharedParallelFunction : public ObjectFunction
{
public:
  SharedParallelFunction(SharedParallelPredictionProblemPtr problem)
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

class SharedParallelPredictionProblem : public ReductionPredictionProblem
{
public:
  SharedParallelPredictionProblem(const String& name)
    : ReductionPredictionProblem(name) {}
  
  virtual size_t getNumSubObjects(ObjectPtr object) const = 0;
  virtual ObjectPtr getSubObject(size_t i) const = 0;
};

class SS3PredictionProblem : public ..
*/

// the aim is to learn a "incomplete Protein => complete Protein" function
class ProteinPredictionProblem : public ReductionPredictionProblem
{
public:
//  ProteinPredictionProblem() : ReductionPredictionProblem(T("Protein Predictions"), new SS3PredictionProblem()) {}

  virtual ObjectPtr transformInputObject(ObjectPtr object) const
    {return object;}

  virtual ObjectPtr transformOutputObject(ObjectPtr object) const
    {return object;}
};

typedef ReferenceCountedObjectPtr<ProteinPredictionProblem> ProteinPredictionProblemPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
