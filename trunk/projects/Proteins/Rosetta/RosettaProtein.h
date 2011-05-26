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
# include "Sampler/GeneralProteinMoverSampler.h"

namespace lbcpp
{

class RosettaProtein : public Object
{
public:
  RosettaProtein(core::pose::PoseOP& pose, size_t computeResidues, size_t computeEnergy, size_t computeHistogram, size_t computeDistances)
    : energy(0), score(0), normalizedScore(0), shortDistance(0), longDistance(0),
      computeResidues(computeResidues), computeEnergy(computeEnergy), computeHistogram(computeHistogram),
      computeDistances(computeDistances)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    this->pose = pose;
    numResidues = (size_t)pose->n_residue();
    update();
# else
    jassert(false);
# endif // LBCPP_PROTEIN_ROSETTA
  }

  RosettaProtein()
    : numResidues(0), energy(0), score(0), normalizedScore(0), shortDistance(0), longDistance(0),
      computeResidues(1), computeEnergy(1), computeHistogram(1), computeDistances(1) {}

  void getPose(core::pose::PoseOP& returnPose)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    returnPose = pose;
#else
    jassert(false);
# endif // LBCPP_PROTEIN_ROSETTA
  }

  void setPose(const core::pose::PoseOP& pose)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    this->pose = pose;
    numResidues = pose->n_residue();
#else
    jassert(false);
#endif
  }

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
    if (computeEnergy)
    {
# ifdef LBCPP_PROTEIN_ROSETTA
      energies(NULL, NULL, NULL);
# else
      jassert(false);
# endif
    }
    if (computeHistogram)
      updateHistogram();
    if (computeDistances)
      updateDistances();
  }

  void updateHistogram()
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    histogram = new DenseDoubleVector(aminoAcidTypeEnumeration, doubleType);
    double increment = 1.0 / numResidues;
    for (size_t i = 0; i < numResidues; i++)
    {
      char n = pose->residue(i+1).name1();
      std::string name(&n, 1);
      String resName(name.c_str());

      for (size_t j = 0; j < aminoAcidTypeEnumeration->getNumElements(); j++)
        if (!resName.compare(aminoAcidTypeEnumeration->getElement(j)->getVariable(1).toString()))
        {
          histogram->incrementValue(j, increment);
          break;
        }
    }
# else
    jassert(false);
# endif // LBCPP_PROTEIN_ROSETTA
  }

  void energies(double* energy, double* score, double* normalizedScore = NULL)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    // energies updated
    this->score = getConformationScore(pose, fullAtomEnergy, &(this->energy));
    this->normalizedScore = sigmoid(0.0005, this->score);

    // return
    if (energy != NULL)
      *energy = this->energy;
    if (score != NULL)
      *score = this->score;
    if (normalizedScore != NULL)
      *normalizedScore = this->normalizedScore;
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

  void updateDistances()
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    SymmetricMatrixPtr distances = createCalphaMatrixDistance(pose);

    int minDist = juce::jmin(20, (int)(numResidues * 0.5));

    double maxShortDist = 5.4 * minDist / 3.6;
    double maxLongDist = 5.4 * numResidues / 3.6;

    double tempShortDist = 0;
    double tempLongDist = 0;
    size_t countShort = 0;
    size_t countLong = 0;
    for (size_t i = 0; i < distances->getNumRows(); i++)
      for (size_t j = 0; j < distances->getNumColumns(); j++)
        if (std::abs((int)i - (int)j) < minDist)
        {
          tempShortDist += distances->getElement(i, j).getDouble();
          countShort++;
        }
        else
        {
          tempLongDist += distances->getElement(i, j).getDouble();
          countLong++;
        }
    shortDistance = tempShortDist / (double)countShort;
    shortDistance /= maxShortDist;
    longDistance = tempLongDist / (double)countLong;
    longDistance /= maxLongDist;
# else
    jassert(false);
# endif
  }

protected:
  friend class RosettaProteinClass;
  
# ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
# endif // LBCPP_PROTEIN_ROSETTA

  size_t numResidues;
  double energy;
  double score;
  double normalizedScore;
  DenseDoubleVectorPtr histogram;
  double shortDistance;
  double longDistance;
  size_t computeResidues;
  size_t computeEnergy;
  size_t computeHistogram;
  size_t computeDistances;
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
    DenseDoubleVectorPtr histogram =
        (inputs[0].getObjectAndCast<RosettaProtein> ())->getHistogram();
    for (size_t i = 0; i < histogram->getNumValues(); i++)
      callback.sense(i, histogram->getValue(i));
  }

protected:
  friend class RosettaProteinResidueHistogramFeatureGeneratorClass;
};

class RosettaProteinFeatures : public CompositeFunction
{
public:
  RosettaProteinFeatures(size_t residues, size_t energy, size_t histogram, size_t distances)
    : selectResiduesFeatures(residues), selectEnergyFeatures(energy),
      selectHistogramFeatures(histogram), selectDistanceFeatures(distances) {}

  RosettaProteinFeatures()
    : selectResiduesFeatures(1), selectEnergyFeatures(1),
      selectHistogramFeatures(1), selectDistanceFeatures(1) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    // inputs and variables
    size_t input = builder.addInput(rosettaProteinClass);
    size_t numResidues = builder.addFunction(getVariableFunction(T("numResidues")), input);
    size_t energy = builder.addFunction(getVariableFunction(T("normalizedScore")), input);
    size_t histogram = builder.addFunction(getVariableFunction(T("histogram")), input);
    size_t shortDistance = builder.addFunction(getVariableFunction(T("shortDistance")), input);
    size_t longDistance = builder.addFunction(getVariableFunction(T("longDistance")), input);

    // select features
    builder.startSelection();
    size_t residueFeatures;
    size_t energyFeatures;
    size_t histogramFeatures;
    size_t shortDistancesFeatures;
    size_t longDistancesFeatures;
    if (selectResiduesFeatures)
      residueFeatures = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(1, std::log10(100.0), 10, true), numResidues);
    if (selectEnergyFeatures)
      energyFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), energy);
    if (selectHistogramFeatures)
      histogramFeatures = builder.addFunction(new RosettaProteinResidueHistogramFeatureGenerator(), input);
    if (selectDistanceFeatures)
    {
      shortDistancesFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), shortDistance);
      longDistancesFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), longDistance);
    }

    // end selection
    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
  }

protected:
  friend class RosettaProteinFeaturesClass;

  size_t selectResiduesFeatures;
  size_t selectEnergyFeatures;
  size_t selectHistogramFeatures;
  size_t selectDistanceFeatures;
};

typedef ReferenceCountedObjectPtr<RosettaProteinFeatures> RosettaProteinFeaturesPtr;
extern CompositeFunctionPtr rosettaProteinFeatures(size_t residues, size_t energy, size_t histogram, size_t distances);

class RosettaWorker;
typedef ReferenceCountedObjectPtr<RosettaWorker> RosettaWorkerPtr;

class RosettaWorker : public Object
{
public:
  RosettaWorker(core::pose::PoseOP& pose, size_t learningPolicy,
                size_t residues, size_t energy, size_t histogram, size_t distances)
    : learningPolicy(learningPolicy)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    protein = new RosettaProtein(pose, residues, energy, histogram, distances);
    features = rosettaProteinFeatures(residues, energy, histogram, distances);
    if (learningPolicy > 1)
      features->initialize(defaultExecutionContext(), rosettaProteinClass);
    sampler = new GeneralProteinMoverSampler(pose->n_residue(), learningPolicy);
# else
    jassert(false);
# endif
  }
  RosettaWorker() {}

  void getPose(core::pose::PoseOP& returnPose)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    protein->getPose(returnPose);
# else
    jassert(false);
# endif
  }

  void setPose(const core::pose::PoseOP& pose)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    protein->setPose(pose);
# else
    jassert(false);
# endif
  }

  size_t getNumResidues()
    {return protein->getNumResidues();}

  void update()
    {protein->update();}

  Variable getFeatures(ExecutionContext& context)
  {
    if (learningPolicy <= 1)
      return Variable();
    protein->update();
    return features->compute(context, protein);
  }

  Variable getFeatures(ExecutionContext& context, const RosettaProteinPtr& input)
  {
    input->update();
    return features->compute(context, input);
  }

  void energies(double* energy, double* score = NULL, double* normalizedScore = NULL)
    {protein->energies(energy, score, normalizedScore);}

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random)
  {
    Variable input;
    if (learningPolicy > 1)
      input = getFeatures(context);
    context.resultCallback(T("toto"), input);
    return sampler->sample(context, random, &input);
  }

  void learn(ExecutionContext& context, const ContainerPtr& inputProteins,
      const ContainerPtr& inputMovers)
  {
    //TypePtr doubleVectorType = features->getOutputType();
    //EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(doubleVectorType);
    VectorPtr inputs;
    if (learningPolicy > 1)
    {
      inputs = vector(features->getOutputType());
      for (size_t i = 0; i < inputProteins->getNumElements(); i++)
      {
        RosettaProteinPtr tempProtein = inputProteins->getElement(i).getObjectAndCast<
            RosettaProtein> ();
        DoubleVectorPtr tempFeatures;
        if (learningPolicy > 1)
          tempFeatures = getFeatures(context, tempProtein).getObjectAndCast<DoubleVector> ();
        inputs->append(tempFeatures);
      }
    }
    sampler->learn(context, inputs, inputMovers, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());
  }

protected:
  friend class RosettaWorkerClass;

  RosettaProteinPtr protein;
  RosettaProteinFeaturesPtr features;
  GeneralProteinMoverSamplerPtr sampler;
  size_t learningPolicy;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_H_
