/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInferenceStep.h| Protein Contact Map Inference  |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_

# include "Protein2DInferenceStep.h"

namespace lbcpp
{

class ProteinContactMapInferenceStep : public Protein2DInferenceStep
{
public:
  ProteinContactMapInferenceStep(const String& name, ProteinResiduePairFeaturesPtr features, const String& targetName)
    : Protein2DInferenceStep(name, new ClassificationInferenceStep(name + T("Classification")), features, targetName)
    {}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    ClassificationInferenceStepPtr step = getSharedInferenceStep().dynamicCast<ClassificationInferenceStep>();
    jassert(step);
    step->setLabels(BinaryClassificationDictionary::getInstance());
    return Protein2DInferenceStep::createEmptyOutput(input);
  }

  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const
  {
    size_t n = protein->getLength();
    res.reserve(n * (n - 5) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
        res.push_back(std::make_pair(i, j));
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const
  {
    ScoreSymmetricMatrixPtr contactMap = supervisionObject.dynamicCast<ScoreSymmetricMatrix>();
    jassert(contactMap);
    if (!contactMap->hasScore(firstPosition, secondPosition))
      return ObjectPtr();
    double score = contactMap->getScore(firstPosition, secondPosition);
    return new Label(BinaryClassificationDictionary::getInstance(), score > 0.5 ? 1 : 0);
  }

  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const
  {
    ScoreSymmetricMatrixPtr contactMap = output.dynamicCast<ScoreSymmetricMatrix>();
    jassert(contactMap);
    DenseVectorPtr probabilities = subOutput.dynamicCast<DenseVector>();
    jassert(probabilities && probabilities->getDictionary() == BinaryClassificationDictionary::getInstance());
    contactMap->setScore(firstPosition, secondPosition, probabilities->get(1));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
