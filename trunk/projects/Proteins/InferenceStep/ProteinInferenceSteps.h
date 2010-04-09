/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceStep.h         | Protein related inferences      |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 18:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
# define LBCPP_PROTEIN_PREDICTION_PROBLEM_H_

# include "InferenceStep.h"
# include "ParallelInferenceStep.h"
# include "ClassificationInferenceStep.h"

namespace lbcpp
{

class FeaturesToContainerElementsSharedParallelInferenceStep : public SharedParallelInferenceStep
{
public:
  FeaturesToContainerElementsSharedParallelInferenceStep(const String& name, InferenceStepPtr subInference)
    : SharedParallelInferenceStep(name, subInference) {}

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
    jassert(container);
    return container->size();
  }

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getInputFeatures(input, index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    if (!supervision)
      return ObjectPtr();
    ObjectContainerPtr s = supervision.dynamicCast<ObjectContainer>();
    jassert(s);
    return s->get(index);
  }

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {output.dynamicCast<ObjectContainer>()->set(index, subOutput);}
};

// Input: Protein
// Output: SecondaryStructureSequence
class SecondaryStructureInferenceStep : public FeaturesToContainerElementsSharedParallelInferenceStep
{
public:
  SecondaryStructureInferenceStep(const String& name, bool useDSSPElements)
    : FeaturesToContainerElementsSharedParallelInferenceStep(name, InferenceStepPtr(new ClassificationInferenceStep(T("SSClassification")))), useDSSPElements(useDSSPElements) {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    SecondaryStructureSequencePtr res = new SecondaryStructureSequence(useDSSPElements);
    res->setLength(getNumSubInferences(input));
    return res;
  }

  bool useDSSPElements;
};

typedef ReferenceCountedObjectPtr<SecondaryStructureInferenceStep> SecondaryStructureInferenceStepPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
