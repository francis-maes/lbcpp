/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidueFeatures.h       | A feature function phi :        |
| Author  : Francis Maes                   | protein x position -> features  |
| Started : 22/04/2010 22:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_RESIDUE_FEATURES_H_
# define LBCPP_PROTEIN_INFERENCE_RESIDUE_FEATURES_H_

# include "../Protein.h"

namespace lbcpp
{

class ProteinResidueFeatures : public Object
{
public:
  virtual FeatureGeneratorPtr compute(ProteinPtr protein, size_t position) = 0;
};

typedef ReferenceCountedObjectPtr<ProteinResidueFeatures> ProteinResidueFeaturesPtr;

extern ProteinResidueFeaturesPtr proteinUnitResidueFeature();
extern ProteinResidueFeaturesPtr proteinPositionIndexResidueFeature();

// multiScalePercentageFeatures(position / length)
extern ProteinResidueFeaturesPtr proteinPositionFeatures(size_t numIntervalsInCoarsestScale = 1, size_t intervalRatio = 2, size_t numScales = 3);

// multiScaleNumberFeatures(length)
extern ProteinResidueFeaturesPtr proteinLengthFeatures(size_t numIntervalsPerLog10InCoarestScale = 2, size_t intervalRatio = 2, size_t numScales = 2);

extern ProteinResidueFeaturesPtr proteinSequenceWindowFeatures(const String& sequenceName, size_t numPrevs, size_t numNexts, bool includeCurrent);
extern ProteinResidueFeaturesPtr proteinFrequencyWindowFeatures(const String& sequenceName, size_t numPrevsAndNexts, bool includeCurrent);
extern ProteinResidueFeaturesPtr proteinSegmentConjunctionFeatures(const String& sequenceName, size_t numSegmentsPerSide);

class CompositeProteinResidueFeatures : public ProteinResidueFeatures
{
public:
  void addSubFeatures(ProteinResidueFeaturesPtr subFeatures)
    {subFeatureFunctions.push_back(subFeatures);}

  virtual String toString() const;

  virtual FeatureGeneratorPtr compute(ProteinPtr protein, size_t position);

  size_t getNumSubFeatureFunctions() const
    {return subFeatureFunctions.size();}

  ProteinResidueFeaturesPtr getSubFeatureFunction(size_t i) const
    {jassert(i < subFeatureFunctions.size()); return subFeatureFunctions[i];}

protected:
  std::vector<ProteinResidueFeaturesPtr> subFeatureFunctions;
  
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

typedef ReferenceCountedObjectPtr<CompositeProteinResidueFeatures> CompositeProteinResidueFeaturesPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_RESIDUE_FEATURES_H_
