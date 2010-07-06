/*-----------------------------------------.---------------------------------.
| Filename: ProteinSequenceLabelingInfe...h| ProteinObject Sequence Labeling |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 21:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_

# include "ProteinTargetInferenceHelper.h"

namespace lbcpp
{

// Inference:
//   Input, Supervision: ProteinObject
//   Output: a class inherited from Sequence
//
// SubInference:
//   Input: Features
//   Output, Supervision: Sequence elements
class Protein1DTargetInference : public SharedParallelInference, public ProteinResidueRelatedInferenceStepHelper
{
public:
  Protein1DTargetInference(const String& name, InferencePtr subInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInference(name, subInference), ProteinResidueRelatedInferenceStepHelper(targetName, features, supervisionName)
  {
    setBatchLearner(simulationInferenceLearner());
  }
  
  Protein1DTargetInference() {}
  
  virtual size_t getNumSubInferences(ProteinObjectPtr protein) const
    {return protein->getLength();}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ProteinObjectPtr protein = input.dynamicCast<ProteinObject>();
    jassert(protein);
    ProteinObjectPtr correctProtein = supervision.dynamicCast<ProteinObject>();
    jassert(correctProtein || !supervision);
    ObjectContainerPtr objects;
    if (correctProtein)
      objects = correctProtein->getObject(supervisionName).dynamicCast<ObjectContainer>();
    
    size_t n = getNumSubInferences(protein);
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
      res->addSubInference(subInference, features->compute(protein, i), objects ? objects->get(i) : ObjectPtr());
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinObjectPtr protein = state->getInput().dynamicCast<ProteinObject>();
    jassert(protein);    
    ObjectContainerPtr res = protein->createEmptyObject(targetName).dynamicCast<ObjectContainer>();
    jassert(res);
    bool hasAtLeastOnePrediction = false;
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      Variable subOutput = state->getSubOutput(i);
      if (subOutput)
      {
        if (subOutput.isObject())
          res->set(i, subOutput);
        else if (subOutput.isDouble())
          res->set(i, new Scalar(subOutput.getDouble())); // tmp: data structures must be updated
        else if (subOutput.isInteger())
        {
          // tmp: data structures must be updated
          LabelSequencePtr labelSequence = res.dynamicCast<LabelSequence>();
          jassert(labelSequence);
          res->set(i, new Label(labelSequence->getDictionary(), subOutput.getInteger()));
        }
        else
          jassert(false);
        hasAtLeastOnePrediction = true;
      }
    }
    return hasAtLeastOnePrediction ? Variable(res) : Variable();
  }

protected:
  virtual bool load(InputStream& istr)
  {
    return SharedParallelInference::load(istr) &&
      ProteinResidueRelatedInferenceStepHelper::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    SharedParallelInference::save(ostr);
    ProteinResidueRelatedInferenceStepHelper::save(ostr);
  }
};
  
typedef ReferenceCountedObjectPtr<Protein1DTargetInference> Protein1DTargetInferencePtr;

class ProteinSequenceLabelingInferenceStep : public Protein1DTargetInference
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name, InferencePtr classificationInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : Protein1DTargetInference(name, classificationInference, features, targetName, supervisionName)
  {
    classificationInference->setName(getName() + T(" Classif"));
  }

  ProteinSequenceLabelingInferenceStep() {}

};

//// PSSM predition
/*
class ParallelSharedMultiRegressionInference : public SharedParallelInference
{
public:
  ParallelSharedMultiRegressionInference(const String& name, FeatureDictionaryPtr outputDictionary)
    : SharedParallelInference(name, new RegressionInferenceStep(name + T("Regression"))), outputDictionary(outputDictionary) {}

  virtual size_t getNumSubInferences() const = 0;
  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
    jassert(container);
    DenseVectorPtr vector = supervision.dynamicCast<DenseVector>();
    jassert(!supervision || vector)

    size_t n = getNumSubInferences();
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
      res->addSubInference(subInference, getInputFeatures(input, i), new Scalar(vector->get(i)));
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    size_t n = state->getNumSubInferences();
    DenseVectorPtr res = new DenseVector(outputDictionary, n);
    for (size_t i = 0; i < n; ++i)
    {
      ScalarPtr scalar = state->getSubOutput(i).dynamicCast<Scalar>();
      if (scalar)
        res->set(i, scalar->getValue());
    }
    return res;
  }

protected:
  FeatureDictionaryPtr outputDictionary;
};

class PSSMRowPredictionInferenceStep : public ParallelSharedMultiRegressionInference
{
public:
  PSSMRowPredictionInferenceStep()
    : ParallelSharedMultiRegressionInference(T("PSSMRow"), AminoAcidDictionary::getInstance()) {}

  virtual size_t getNumSubInferences() const
    {return AminoAcidDictionary::numAminoAcids;}
 
  FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t scoreIndex) const;
};

class PSSMPredictionInferenceStep : public Protein1DTargetInference
{
public:
  PSSMPredictionInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DTargetInference(name, new PSSMRowPredictionInferenceStep(), features, T("PositionSpecificScoringMatrix")) {}
  PSSMPredictionInferenceStep() {}
};
*/
/////////////

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
