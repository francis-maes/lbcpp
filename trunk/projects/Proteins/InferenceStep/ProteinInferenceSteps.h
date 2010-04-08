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

class Label : public FeatureGeneratorDefaultImplementations<Label, FeatureVector>
{
public:
  Label(FeatureDictionaryPtr featureDictionary, size_t index)
    : value(index + 1)
    {setDictionary(featureDictionary);}

  Label(FeatureDictionaryPtr featureDictionary)
    : value(0)
    {setDictionary(featureDictionary);}

  Label(StringDictionaryPtr stringDictionary)
    : value(0)
    {setDictionary(FeatureDictionaryManager::getInstance().getFlatVectorDictionary(stringDictionary));}

  virtual String toString() const
    {return isDecided() ? getDictionary()->getFeature(getIndex()) : T("?");}

  virtual void clear()
    {value = 0;}

  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const
    {visitor.featureSense(getDictionary(), value, 1.0);}

  bool isDecided() const
    {return value > 0;}

  size_t getIndex() const
    {jassert(isDecided()); return (size_t)(value - 1);}

protected:
  // FeatureGenerator
  virtual size_t getNumSubGenerators() const
    {return 0;}
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {jassert(false); return FeatureGeneratorPtr();}
  virtual size_t getSubGeneratorIndex(size_t num) const
    {jassert(false); return (size_t)-1;}
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {jassert(false); return FeatureGeneratorPtr();}

private:
  int value;
};

typedef ReferenceCountedObjectPtr<Label> LabelPtr;

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
    return new Label(FeatureDictionaryPtr(), secondaryStructure->getLabel(index));
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
