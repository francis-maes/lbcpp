/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.h             | Class for protein inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_H_
# define LBCPP_PROTEIN_INFERENCE_H_

# include "../InferenceStep/ParallelSequenceLabelingInferenceStep.h"
# include "../InferenceStep/SequentialInferenceStep.h"
# include "SecondaryStructureDictionary.h"

namespace lbcpp
{

class ProteinSequenceLabelingInferenceStep : public ParallelSequenceLabelingInferenceStep
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name)
    : ParallelSequenceLabelingInferenceStep(name) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual String getTargetName() const = 0;
  virtual FeatureDictionaryPtr getTargetDictionary() const = 0;

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    ProteinPtr protein = supervision.dynamicCast<Protein>();
    jassert(protein);
    ObjectPtr proteinObject = protein->getObject(getTargetName());
    return ParallelSequenceLabelingInferenceStep::getSubSupervision(proteinObject, index);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new LabelSequence(getTargetName(), getTargetDictionary(), getNumSubInferences(input));}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }
};

typedef ReferenceCountedObjectPtr<ProteinSequenceLabelingInferenceStep> ProteinSequenceLabelingInferenceStepPtr;

class ProteinInference : public VectorSequentialInferenceStep
{
public:
  ProteinInference() : VectorSequentialInferenceStep(T("Protein")) {}

  // child inference are all of the form
  // Protein -> Protein sub object
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    // input and working proteins
    ProteinPtr inputProtein = input.dynamicCast<Protein>();
    jassert(inputProtein);
    ProteinPtr workingProtein = new Protein(inputProtein->getName());
    workingProtein->setObject(inputProtein->getAminoAcidSequence());
    workingProtein->setObject(inputProtein->getPositionSpecificScoringMatrix());
    
    // supervision
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();

    // main inference loop
    for (size_t i = 0; i < inferenceSteps.size(); ++i)
    {
      InferenceStepPtr inferenceStep = inferenceSteps[i];

      ObjectPtr inferenceOutput = context->runInference(inferenceStep, workingProtein, correctProtein, returnCode);
      jassert(inferenceOutput);
      workingProtein = workingProtein->clone();
      workingProtein->setObject(inferenceOutput);
      if (returnCode != finishedReturnCode)
        break;
    }

    // return the last version of the working protein
    return workingProtein;
  }
};

typedef ReferenceCountedObjectPtr<ProteinInference> ProteinInferencePtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
