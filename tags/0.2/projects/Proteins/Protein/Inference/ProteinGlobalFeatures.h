/*-----------------------------------------.---------------------------------.
| Filename: ProteinGlobalFeatures.h        | A feature function phi :        |
| Author  : Francis Maes                   |    protein -> features          |
| Started : 05/05/2010 16:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_GLOBAL_FEATURES_H_
# define LBCPP_PROTEIN_INFERENCE_GLOBAL_FEATURES_H_

# include "../Protein.h"

namespace lbcpp
{

class ProteinGlobalFeatures : public Object
{
public:
  virtual String getName() const
    {return getClassName();}

  virtual FeatureGeneratorPtr compute(ProteinPtr protein) = 0;
};

typedef ReferenceCountedObjectPtr<ProteinGlobalFeatures> ProteinGlobalFeaturesPtr;

// numberLogFeatures(length)
extern ProteinGlobalFeaturesPtr proteinUnitResidueGlobalFeature();
extern ProteinGlobalFeaturesPtr proteinLengthFeatures(size_t numIntervalsPerLog10 = 3);
extern ProteinGlobalFeaturesPtr proteinGlobalCompositionFeatures(const String& sequenceName);

class CompositeProteinGlobalFeatures : public ProteinGlobalFeatures
{
public:
  void addSubFeatures(ProteinGlobalFeaturesPtr subFeatures)
    {subFeatureFunctions.push_back(subFeatures);}

  virtual String toString() const;

  virtual FeatureGeneratorPtr compute(ProteinPtr protein);

  size_t getNumSubFeatureFunctions() const
    {return subFeatureFunctions.size();}

  ProteinGlobalFeaturesPtr getSubFeatureFunction(size_t i) const
    {jassert(i < subFeatureFunctions.size()); return subFeatureFunctions[i];}

protected:
  std::vector<ProteinGlobalFeaturesPtr> subFeatureFunctions;
  
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

typedef ReferenceCountedObjectPtr<CompositeProteinGlobalFeatures> CompositeProteinGlobalFeaturesPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_GLOBAL_FEATURES_H_
