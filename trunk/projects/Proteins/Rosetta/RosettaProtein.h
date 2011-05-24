/*-----------------------------------------.---------------------------------.
| Filename: RosettaProtein.h               | RosettaProtein                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 24 mai 2011  07:47:58          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_H_

# include "RosettaUtils.h"
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class RosettaProtein : public Object
{
public:
  RosettaProtein(const core::pose::PoseOP& pose)
    : pose(pose), energy(0), score(0), normalizedScore(0)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    numResidues = (size_t)pose->n_residue();
    update();
# else
    jassert(false);
# endif // LBCPP_PROTEIN_ROSETTA
  }
  RosettaProtein() : numResidues(0), energy(0), score(0), normalizedScore(0) {}

  void getPose(core::pose::PoseOP& returnPose)
    {returnPose = pose;}

  DenseDoubleVectorPtr getHistogram()
    {return histogram;}

  size_t getNumResidues()
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    return (size_t)pose->n_residue();
# else
    return 0;
# endif // LBCPP_PROTEIN_ROSETTA
  }

  void update()
  {
    numResidues = getNumResidues();
    score = getConformationScore(pose, fullAtomEnergy, &energy);
    normalizedScore = sigmoid(0.0005, score);
    updateHistogram();
  }

  void updateHistogram()
  {// TODO
    histogram = new DenseDoubleVector(aminoAcidTypeEnumeration, doubleType);
    histogram->setValue(0, 0.999);
    histogram->setValue(1, 0.99);
    histogram->setValue(2, 0.98);
    std::cout << "size ddv : " << histogram->getNumValues() << std::endl;
    std::cout << "h : " << histogram->toString() << std::endl;

    for (size_t i = 0; i < numResidues; i++)
    {
      // String residueName = featuresEnumeration->getElement(i)->getVariable(1).toString();
    }
    std::cout << "h apres : " << histogram->toString() << std::endl;
  }

  void energies(double* energy, double* score, double* normalizedScore = NULL)
  {
    if (energy != NULL)
      *energy = this->energy;
    if (score != NULL)
      *score = this->score;
    if (normalizedScore != NULL)
      *normalizedScore = this->normalizedScore;
  }
protected:
  friend class RosettaProteinClass;

  core::pose::PoseOP pose;
  size_t numResidues;
  double energy;
  double score;
  double normalizedScore;
  DenseDoubleVectorPtr histogram;
};

extern ClassPtr rosettaProteinClass;
typedef ReferenceCountedObjectPtr<RosettaProtein> RosettaProteinPtr;

class RosettaProteinResidueHistogramFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return rosettaProteinClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return aminoAcidTypeEnumeration;}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    DenseDoubleVectorPtr histogram = (inputs[0].getObjectAndCast<RosettaProtein>())->getHistogram();
    std::cout << "histo values : " << histogram->toString() << std::endl;
    // TODO
    //    for (size_t i = 0; i < histogram->getNumValues(); i++)
    //      callback.sense(i, histogram->getValue(i));
    callback.sense(0, histogram, 1);
  }
};

class RosettaProteinFeatures : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(rosettaProteinClass);
    size_t numResidues = builder.addFunction(getVariableFunction(T("numResidues")), input);
    size_t residueFeatures = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(1, std::log10(100), 10, true), numResidues);

    size_t energy = builder.addFunction(getVariableFunction(T("normalizedScore")), input);
    size_t energyFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(5), energy);

    size_t temp = builder.addFunction(concatenateFeatureGenerator(), residueFeatures, energyFeatures);

    size_t histogram = builder.addFunction(getVariableFunction(T("histogram")), input);
    size_t histogramFeatures = builder.addFunction(new RosettaProteinResidueHistogramFeatureGenerator(), input);

    builder.addFunction(concatenateFeatureGenerator(), temp, histogramFeatures);
  }

protected:
  friend class RosettaProteinFeaturesClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_H_
