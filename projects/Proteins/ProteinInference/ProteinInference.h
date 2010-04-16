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

  virtual String getTargetName() const = 0;
  virtual FeatureDictionaryPtr getTargetDictionary() const = 0;

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new LabelSequence(getTargetDictionary(), getNumSubInferences(input));}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }
};

typedef ReferenceCountedObjectPtr<ProteinSequenceLabelingInferenceStep> ProteinSequenceLabelingInferenceStepPtr;

class ProteinInference : public SequentialInferenceStep
{
public:
  ProteinInference() : SequentialInferenceStep(T("Protein")) {}

  // child inference are all of the form
  // Protein -> Protein sub object
  void appendStep(InferenceStepPtr inference, const String& targetName)
    {inferenceSteps.push_back(std::make_pair(inference, targetName));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    // input and working proteins
    ProteinPtr inputProtein = input.dynamicCast<Protein>();
    jassert(inputProtein);
    ProteinPtr workingProtein = new Protein(inputProtein->getName());
    workingProtein->setAminoAcidSequence(inputProtein->getAminoAcidSequence());
    workingProtein->setPositionSpecificScoringMatrix(inputProtein->getPositionSpecificScoringMatrix());
    
    // supervision
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();

    // main inference loop
    for (size_t i = 0; i < inferenceSteps.size(); ++i)
    {
      InferenceStepPtr inferenceStep = inferenceSteps[i].first;
      String objectName = inferenceSteps[i].second;

      ObjectPtr supervision;
      if (correctProtein)
      {
        supervision = correctProtein->getObject(objectName);
        jassert(supervision);
      }

      ObjectPtr inferenceOutput = context->runInference(inferenceStep, workingProtein, supervision, returnCode);
      jassert(inferenceOutput);
      workingProtein = workingProtein->clone();
      workingProtein->setObject(objectName, inferenceOutput);
      if (returnCode != finishedReturnCode)
        break;
    }

    // return the last version of the working protein
    return workingProtein;
  }

  virtual size_t getNumSubSteps() const
    {return inferenceSteps.size();}

  virtual InferenceStepPtr getSubStep(size_t index) const
    {jassert(index < inferenceSteps.size()); return inferenceSteps[index].first;}

protected:
  std::vector< std::pair<InferenceStepPtr, String> > inferenceSteps;
};

typedef ReferenceCountedObjectPtr<ProteinInference> ProteinInferencePtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
