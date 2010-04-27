/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.h             | Class for protein inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_H_
# define LBCPP_PROTEIN_INFERENCE_H_

# include "../../InferenceStep/SequentialInferenceStep.h"

namespace lbcpp
{

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
    jassert(inputProtein->getAminoAcidSequence());
    ScoreVectorSequencePtr pssm = inputProtein->getPositionSpecificScoringMatrix();
    jassert(inputProtein->getPositionSpecificScoringMatrix());
    if (pssm)
      workingProtein->setObject(pssm);
    workingProtein->setObject(inputProtein->getAminoAcidProperty());
    
    // supervision
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();

    // main inference loop
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      InferenceStepPtr inferenceStep = subInferences[i];

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
