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

namespace lbcpp
{

// Input: Protein
// Output: SecondaryStructureSequence
class SecondaryStructureInferenceStep : public SharedParallelInferenceStep
{
public:
  SecondaryStructureInferenceStep(const String& name = T("SecondaryStructure"))
    : SharedParallelInferenceStep(name, InferenceStepPtr(new ClassificationInferenceStep(T("SS3Classification"))), T("SecondaryStructureSequence")) {}

  virtual FeatureGeneratorPtr getInputFeatures(ProteinPtr protein, size_t index) const = 0;

  virtual size_t getNumSubObjects(ObjectPtr input, ObjectPtr output) const
    {return getProtein(input)->getLength();}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getInputFeatures(getProtein(input), index);}

  virtual ObjectPtr getSubOutput(ObjectPtr output, size_t index) const
  {
    SecondaryStructureSequencePtr secondaryStructure = output.dynamicCast<SecondaryStructureSequence>();
    if (!secondaryStructure)
      return ObjectPtr();
    FeatureDictionaryPtr dictionary = FeatureDictionaryManager::getInstance().getFlatVectorDictionary(secondaryStructure->getVariablesDictionary());
    return new Label(dictionary, secondaryStructure->getLabel(index));
  }

protected:
  ProteinPtr getProtein(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return protein;
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
