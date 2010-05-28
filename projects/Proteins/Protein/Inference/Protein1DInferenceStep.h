/*-----------------------------------------.---------------------------------.
| Filename: ProteinSequenceLabelingInfe...h| Protein Sequence Labeling       |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 21:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_

# include "ProteinInferenceStepHelper.h"

namespace lbcpp
{

// Inference:
//   Input, Supervision: Protein
//   Output: a class inherited from Sequence
//
// SubInference:
//   Input: Features
//   Output, Supervision: Sequence elements
class Protein1DInferenceStep : public SharedParallelInference, public ProteinResidueRelatedInferenceStepHelper
{
public:
  Protein1DInferenceStep(const String& name, InferencePtr subInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInference(name, subInference), ProteinResidueRelatedInferenceStepHelper(targetName, features, supervisionName) {}
  
  Protein1DInferenceStep() {}
  
  virtual size_t getNumSubInferences(ProteinPtr protein) const
    {return protein->getLength();}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
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

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinPtr protein = state->getInput().dynamicCast<Protein>();
    jassert(protein);    
    ObjectContainerPtr res = protein->createEmptyObject(targetName).dynamicCast<ObjectContainer>();
    jassert(res);
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
      res->set(i, state->getSubOutput(i));
    return res;
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
  
typedef ReferenceCountedObjectPtr<Protein1DInferenceStep> Protein1DInferenceStepPtr;

class ProteinSequenceLabelingInferenceStep : public Protein1DInferenceStep
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name, InferencePtr classificationInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : Protein1DInferenceStep(name, classificationInference, features, targetName, supervisionName)
  {
    classificationInference->setName(getName() + T(" Classif"));
  }

  ProteinSequenceLabelingInferenceStep(const String& name, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : Protein1DInferenceStep(name, new ClassificationInferenceStep(name + T(" Classif")), features, targetName, supervisionName) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectContainerPtr res = SharedParallelInference::run(context, input, supervision, returnCode).dynamicCast<ObjectContainer>();
    for (size_t i = 0; i < res->size(); ++i)
      if (res->get(i))
        return res;
    return ObjectPtr(); // only missing values, return an empty object
  }
};

//// PSSM predition

class ParallelSharedMultiRegressionInference : public SharedParallelInference
{
public:
  ParallelSharedMultiRegressionInference(const String& name, FeatureDictionaryPtr outputDictionary)
    : SharedParallelInference(name, new RegressionInferenceStep(name + T("Regression"))), outputDictionary(outputDictionary) {}

  virtual size_t getNumSubInferences() const = 0;
  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
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

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
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

class PSSMPredictionInferenceStep : public Protein1DInferenceStep
{
public:
  PSSMPredictionInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DInferenceStep(name, new PSSMRowPredictionInferenceStep(), features, T("PositionSpecificScoringMatrix")) {}
  PSSMPredictionInferenceStep() {}
};

/////////////

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
