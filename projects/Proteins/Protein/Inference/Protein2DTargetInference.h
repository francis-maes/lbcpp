/*-----------------------------------------.---------------------------------.
| Filename: Protein2DTargetInference.h       | Base class for 2D inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_2D_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_2D_STEP_H_

# include "ProteinTargetInferenceHelper.h"

namespace lbcpp
{

class Protein2DTargetInference : public SharedParallelInference, public ProteinResiduePairRelatedInferenceStepHelper
{
public:
  Protein2DTargetInference(const String& name, InferencePtr subInference, ProteinResiduePairFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInference(name, subInference), ProteinResiduePairRelatedInferenceStepHelper(targetName, features, supervisionName)
  {
    setBatchLearner(simulationInferenceLearner());
  }
  
  Protein2DTargetInference() {}
  
  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const = 0;

  struct State : public ParallelInferenceState
  {
    State(ProteinPtr input, ProteinPtr supervision)
      : ParallelInferenceState(input, supervision) {}

    std::vector< std::pair<size_t, size_t> > subInferenceIndices;
  };

  typedef ReferenceCountedObjectPtr<State> StatePtr;

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
    jassert(correctProtein || !supervision);

    StatePtr state = new State(protein, correctProtein);
    computeSubStepIndices(protein, state->subInferenceIndices);

    ObjectPtr targetObject = correctProtein ? correctProtein->getObject(supervisionName) : ObjectPtr();
    state->reserve(state->subInferenceIndices.size());
    for (size_t i = 0; i < state->subInferenceIndices.size(); ++i)
    {
      std::pair<size_t, size_t> ij = state->subInferenceIndices[i];
      state->addSubInference(subInference,
        features->compute(protein, ij.first, ij.second),
        getSubSupervision(targetObject, ij.first, ij.second));
    }
    return state;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr s, ReturnCode& returnCode)
  {
    StatePtr state = s.dynamicCast<State>();
    jassert(state);
    ProteinPtr protein = state->getInput().dynamicCast<Protein>();
    jassert(protein);    
    ObjectPtr res = protein->createEmptyObject(targetName);
    bool hasAtLeastOnePrediction = false;
    for (size_t i = 0; i < state->subInferenceIndices.size(); ++i)
    {
      std::pair<size_t, size_t> ij = state->subInferenceIndices[i];
      ObjectPtr subOutput = state->getSubOutput(i);
      if (subOutput)
      {
        setSubOutput(res, ij.first, ij.second, subOutput);
        hasAtLeastOnePrediction = true;
      }
    }
    return hasAtLeastOnePrediction ? Variable(res) : Variable();
  }

protected:   
  virtual bool load(InputStream& istr)
  {
    return SharedParallelInference::load(istr) &&
      ProteinResiduePairRelatedInferenceStepHelper::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    SharedParallelInference::save(ostr);
    ProteinResiduePairRelatedInferenceStepHelper::save(ostr);
  }
};
  
typedef ReferenceCountedObjectPtr<Protein2DTargetInference> Protein2DTargetInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_2D_STEP_H_
