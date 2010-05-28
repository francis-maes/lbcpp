/*-----------------------------------------.---------------------------------.
| Filename: Protein2DInferenceStep.h       | Base class for 2D inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_2D_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_2D_STEP_H_

# include "ProteinInferenceStepHelper.h"

namespace lbcpp
{

class Protein2DInferenceStep : public SharedParallelInference, public ProteinResiduePairRelatedInferenceStepHelper
{
public:
  Protein2DInferenceStep(const String& name, InferencePtr subInference, ProteinResiduePairFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInference(name, subInference), ProteinResiduePairRelatedInferenceStepHelper(targetName, features, supervisionName) {}
  
  Protein2DInferenceStep() {}
  
  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const = 0;

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    computeSubStepIndices(protein, subStepIndices);

    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
    jassert(correctProtein || !supervision);
    ObjectPtr targetObject = correctProtein ? correctProtein->getObject(supervisionName) : ObjectPtr();


    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(subStepIndices.size());
    for (size_t i = 0; i < subStepIndices.size(); ++i)
    {
      std::pair<size_t, size_t> ij = subStepIndices[i];
      res->addSubInference(subInference, features->compute(protein, ij.first, ij.second), getSubSupervision(targetObject, ij.first, ij.second));
    }
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinPtr protein = state->getInput().dynamicCast<Protein>();
    jassert(protein);    
    ObjectPtr res = protein->createEmptyObject(targetName);
    jassert(subStepIndices.size() == state->getNumSubInferences());
    for (size_t i = 0; i < subStepIndices.size(); ++i)
    {
      std::pair<size_t, size_t> ij = subStepIndices[i];
      setSubOutput(res, ij.first, ij.second, state->getSubOutput(i));
    }
    return res;
  }

protected:
  std::vector< std::pair<size_t, size_t> > subStepIndices;
    
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
  
typedef ReferenceCountedObjectPtr<Protein2DInferenceStep> Protein2DInferenceStepPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_2D_STEP_H_
