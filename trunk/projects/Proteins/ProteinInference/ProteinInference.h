/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.h             | Class for protein inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_H_
# define LBCPP_PROTEIN_INFERENCE_H_

# include "../InferenceStep/ParallelInferenceStep.h"
# include "../InferenceStep/SequentialInferenceStep.h"
# include "../InferenceStep/ClassificationInferenceStep.h"
# include "SecondaryStructureDictionary.h"

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
    return new LabelSequence(useDSSPElements ? DSSPSecondaryStructureDictionary::getInstance()
      : SecondaryStructureDictionary::getInstance(), getNumSubInferences(input));
  }

  bool useDSSPElements;
};

typedef ReferenceCountedObjectPtr<SecondaryStructureInferenceStep> SecondaryStructureInferenceStepPtr;

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
