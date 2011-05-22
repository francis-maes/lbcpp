/*-----------------------------------------.---------------------------------.
| Filename: ContactMapInference.h          | Contact Map Inference           |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_

# include <lbcpp/Data/Perception.h>
# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/Inference/ParallelInference.h>
# include "../Data/Protein.h"

# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/Evaluator.h>
# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

class ContactMapInference : public SharedParallelInference
{
public:
  ContactMapInference(const String& name, PerceptionPtr perception, InferencePtr contactInference)
    : SharedParallelInference(name, contactInference), perception(perception) {}
  ContactMapInference() {}

  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual TypePtr getSupervisionType() const
    {return symmetricMatrixClass(probabilityType());}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return symmetricMatrixClass(probabilityType());}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ProteinPtr inputProtein = input.getObjectAndCast<Protein>();
    SymmetricMatrixPtr supervisionMap = supervision.getObjectAndCast<SymmetricMatrix>();
    jassert(inputProtein && (!supervision || supervisionMap));

    size_t n = inputProtein->getLength();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve((n * (n - 6)) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
      {
        Variable elementSupervision;
        if (supervisionMap)
          elementSupervision = supervisionMap->getElement(i, j);
        res->addSubInference(subInference, perception->compute(Variable::pair(input, Variable::pair(i, j))), elementSupervision);
      }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinPtr inputProtein = state->getInput().getObjectAndCast<Protein>();    
    size_t n = inputProtein->getLength();

    SymmetricMatrixPtr res = new SymmetricMatrix(probabilityType(), n);
    bool atLeastOnePrediction = false;
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
      {
        Variable result = state->getSubOutput(index++);
        if (result)
        {
          atLeastOnePrediction = true;
          res->setElement(i, j, result);
        }
      }

    return atLeastOnePrediction ? res : Variable::missingValue(res->getClass());
  }

protected:
  PerceptionPtr perception;
};


class AddBiasInference : public StaticDecoratorInference
{
public:
  AddBiasInference(const String& name, InferencePtr numericalInference, double initialBias = 0.0);
  AddBiasInference() {}
/*
  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    if (supervision)
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      res->setSubInference(decorated, input, addConstantScalarFunction(bias)->composeWith(loss));
    }
    else
      res->setSubInference(decorated, input, Variable());
    return res;
  }

  virtual std::pair<Variable, Variable> prepareSubInference(const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      return std::make_pair(input, addConstantScalarFunction(bias)->composeWith(loss));
    }
    return std::make_pair(input, supervision);
  }*/
   
  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
  {
    Variable subOutput = finalState->getSubOutput();
    return subOutput ? Variable((subOutput.getDouble() + bias) / 1000.0) : subOutput;
  }

  double getBias() const
    {return bias;}

  void setBias(double bias)
    {this->bias = bias;}
  
protected:
  double bias;
};

typedef ReferenceCountedObjectPtr<AddBiasInference> AddBiasInferencePtr;

inline InferencePtr addBiasInference(const String& name, InferencePtr numericalInference, double initialBias = 0.0)
  {return new AddBiasInference(name, numericalInference, initialBias);}

class AddBiasInferenceOnlineLearner : public UpdatableInferenceOnlineLearner
{
public:
  AddBiasInferenceOnlineLearner(UpdateFrequency updateFrequency)
    : UpdatableInferenceOnlineLearner(updateFrequency) {}

  virtual void stepFinishedCallback(InferencePtr inf, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    AddBiasInferencePtr inference = inf.staticCast<AddBiasInference>();

    if (prediction)
    {
      ScalarFunctionPtr loss = supervision.getObjectAndCast<ScalarFunction>();
      bool isPositiveExample = loss->compute(1.0) < loss->compute(-1.0);
      double unbiasedScore = (prediction.getDouble() - inference->getBias()) * 1000;
      roc.addPrediction(unbiasedScore, isPositiveExample);
    }
    UpdatableInferenceOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
  }

  virtual bool isLearningStopped() const
    {return false;}

  virtual bool wantsMoreIterations() const
    {return false;}

  virtual double getCurrentLossEstimate() const
    {return 0.0;}

  virtual void update(InferencePtr inf)
  {
    AddBiasInferencePtr inference = inf.staticCast<AddBiasInference>();
    
    if (roc.getSampleCount())
    {
      double bestF1Score, precision, recall;
      std::cout << "Looking for best threshold..." << std::flush;
      double threshold = roc.findThresholdMaximisingF1(bestF1Score, precision, recall);
      std::cout << " " << threshold << " (F1: " << bestF1Score * 100.0 << "%)" << std::endl;
      inference->setBias(-threshold);
      roc.clear();
    }
  }

protected:
  UpdateFrequency updateFrequency;
  ScalarVariableMean bestThreshold;
  ROCAnalyse roc;
};

inline AddBiasInference::AddBiasInference(const String& name, InferencePtr numericalInference, double initialBias)
  : StaticDecoratorInference(name, numericalInference), bias(initialBias)
{
  setOnlineLearner(new AddBiasInferenceOnlineLearner(InferenceOnlineLearner::perPass));
}

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_