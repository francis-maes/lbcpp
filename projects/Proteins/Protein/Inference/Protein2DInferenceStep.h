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
# include "../../InferenceStep/ClassificationInferenceStep.h"
# include "../../InferenceStep/ParallelInferenceStep.h"

namespace lbcpp
{

class Protein2DInferenceStep : public SharedParallelInferenceStep, public ProteinResiduePairRelatedInferenceStepHelper
{
public:
  Protein2DInferenceStep(const String& name, InferenceStepPtr subInference, ProteinResiduePairFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInferenceStep(name, subInference), ProteinResiduePairRelatedInferenceStepHelper(targetName, features, supervisionName) {}
  
  Protein2DInferenceStep() {}
  
  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {ensureSubStepIndicesAreComputed(input); return subStepIndices.size();}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
  {
    ensureSubStepIndicesAreComputed(input);
    jassert(index < subStepIndices.size());
    std::pair<size_t, size_t> ij = subStepIndices[index];
    return features->compute(getProtein(input), ij.first, ij.second);
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    ensureSubStepIndicesAreComputed(supervision);
    jassert(index < subStepIndices.size());
    std::pair<size_t, size_t> ij = subStepIndices[index];
    return getSubSupervision(getSupervision(supervision), ij.first, ij.second);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ProteinResiduePairRelatedInferenceStepHelper::createEmptyOutput(input);}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    jassert(index < subStepIndices.size());
    std::pair<size_t, size_t> ij = subStepIndices[index];
    setSubOutput(output, ij.first, ij.second, subOutput);
  }

protected:
  String currentInputName;
  std::vector< std::pair<size_t, size_t> > subStepIndices;

  void ensureSubStepIndicesAreComputed(ObjectPtr input) const
  {
    jassert(input->getName().isNotEmpty() && input->getName() != T("Unnamed"));
    if (currentInputName != input->getName() || subStepIndices.empty())
    {
      Protein2DInferenceStep* pthis = const_cast<Protein2DInferenceStep* >(this);
      pthis->subStepIndices.clear();
      computeSubStepIndices(getProtein(input), pthis->subStepIndices);
      pthis->currentInputName = input->getName();
    }
  }

  virtual bool load(InputStream& istr)
  {
    return SharedParallelInferenceStep::load(istr) &&
      ProteinResiduePairRelatedInferenceStepHelper::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    SharedParallelInferenceStep::save(ostr);
    ProteinResiduePairRelatedInferenceStepHelper::save(ostr);
  }
};
  
typedef ReferenceCountedObjectPtr<Protein2DInferenceStep> Protein2DInferenceStepPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_2D_STEP_H_
