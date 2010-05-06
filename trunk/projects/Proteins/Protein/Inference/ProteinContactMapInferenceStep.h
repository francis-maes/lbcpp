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
    : Protein2DInferenceStep(name, InferencePtr(), features, targetName)
    {setSharedInferenceStep(linearScalarInference(name + T(" Classification")));}

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
    if (!contactMap || !contactMap->hasScore(firstPosition, secondPosition))
      return ObjectPtr();
    return hingeLoss(contactMap->getScore(firstPosition, secondPosition) > 0.5 ? 1 : 0);
  }

  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const
  {
    ScoreSymmetricMatrixPtr contactMap = output.dynamicCast<ScoreSymmetricMatrix>();
    ScalarPtr score = subOutput.dynamicCast<Scalar>();
    jassert(contactMap && score);
    contactMap->setScore(firstPosition, secondPosition, 1.0 / (1.0 + exp(-score->getValue())));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
