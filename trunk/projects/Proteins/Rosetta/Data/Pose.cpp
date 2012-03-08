/*-----------------------------------------.---------------------------------.
| Filename: Pose.cpp                       | Pose source                     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:09 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "Pose.h"
#include "Features/GeneralFeatures.h"

#include "../../Data/AminoAcid.h"

#include <lbcpp/Core/CompositeFunction.h>
#include <lbcpp/Data/SymmetricMatrix.h>

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/chemical/ChemicalManager.hh>
#  include <core/chemical/util.hh>
#  include <core/conformation/Conformation.hh>
#  include <core/conformation/Residue.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <core/scoring/ScoreFunctionFactory.hh>
#  include <numeric/xyzVector.hh>
#  include <protocols/moves/RigidBodyMover.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

using namespace lbcpp;


void Pose::setFeatureGenerator(ExecutionContext& context, GeneralFeaturesPtr& features)
{
  featureGenerator = features;
  hasFeatureGenerator = true;
}

GeneralFeaturesPtr Pose::getFeatureGenerator() const
  {return featureGenerator;}

Variable Pose::getFeatures(ExecutionContext& context)
{
  if (hasFeatureGenerator)
  {
    Variable tmp = (PosePtr)this;
    return featureGenerator->computeFeatures(context, tmp);
  }
  else
  {
    DenseDoubleVectorPtr tmp = new DenseDoubleVector(1, 1.0);
    return tmp;
  }
}

#ifdef LBCPP_PROTEIN_ROSETTA
Pose::Pose() : isEnergyFunctionInitialized(false), hasFeatureGenerator(false) {pose = new core::pose::Pose();}

Pose::Pose(const String& sequence) : isEnergyFunctionInitialized(false), hasFeatureGenerator(false)
{
  jassert(sequence.length() != 0);
  pose = new core::pose::Pose();
  core::chemical::make_pose_from_sequence(*pose, (const char*)sequence,
      core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set("fa_standard"));
}

Pose::Pose(const ProteinPtr& protein)
{
  bool error = false;
  String name = String(protein->getName());
  File tempFile = File::createTempFile(name + String(Time::currentTimeMillis()) + T(".pdb"));

  if (tempFile != File::nonexistent)
  {
    protein->saveToPDBFile(defaultExecutionContext(), tempFile);
    pose = new core::pose::Pose(std::string((const char*)tempFile.getFullPathName()));
    tempFile.deleteFile();
    error = (int)pose->n_residue() != (int)protein->getLength();
  }
  else
    error = true;

  if (error)
    jassert(false);
}

Pose::Pose(const File& pdbFile) : isEnergyFunctionInitialized(false), hasFeatureGenerator(false)
{
  jassert(pdbFile != File::nonexistent);
  pose = new core::pose::Pose((const char*)pdbFile.getFullPathName());
  if (pose() == NULL)
    jassert(false);
}

void Pose::saveToPDB(const File& pdbFile) const
{
  if (!pose->dump_pdb((const char*)pdbFile.getFullPathName()))
    jassert(false);
}

PosePtr Pose::clone() const
{
  // create new pose
  PosePtr tmp = new Pose();

  // copy Rosetta pose
  *(tmp->pose) = *pose;

  // copy energy function
  tmp->isEnergyFunctionInitialized = isEnergyFunctionInitialized;
  if (isEnergyFunctionInitialized)
    tmp->score_fct = score_fct->clone();

  // copy feature generator
  tmp->hasFeatureGenerator = hasFeatureGenerator;
  if (hasFeatureGenerator)
    tmp->featureGenerator = featureGenerator->cloneAndCast<GeneralFeatures> ();

  return tmp;
}

size_t Pose::getLength() const
  {return pose->n_residue();}

DoubleVectorPtr Pose::getCalphaPosition(size_t residue) const
{
  numeric::xyzVector<double> coord = (pose->residue(residue + 1)).xyz("CA");
  DenseDoubleVectorPtr tmp = new DenseDoubleVector(3, 0.0);
  tmp->setValue(0, coord.x());
  tmp->setValue(1, coord.y());
  tmp->setValue(2, coord.z());
  return tmp;
}

void Pose::initializeToHelix()
{
  for (size_t i = 1; i <= pose->n_residue(); i++)
  {
    if (i == 1)
      pose->set_phi(i, 0);
    else
      pose->set_phi(i, -150);
    if (i == pose->n_residue())
      pose->set_psi(i, 0);
    else
      pose->set_psi(i, 150);
    pose->set_omega(i, 45);
  }
}

double Pose::getPhi(size_t residue) const
  {return pose->phi(residue + 1);}

double Pose::getPsi(size_t residue) const
  {return pose->psi(residue + 1);}

void Pose::setPhi(size_t residue, double phi)
  {pose->set_phi(residue + 1, phi);}

void Pose::setPsi(size_t residue, double psi)
  {pose->set_psi(residue + 1, psi);}

void Pose::applyRotation(size_t residueOne, size_t residueTwo, double amplitude)
{
  jassert(getLength() > 2);
  jassert(std::abs((double)residueOne - (double)residueTwo) >= 2.0);
  jassert((residueOne >= 0) && (residueOne < getLength()));
  jassert((residueTwo >= 0) && (residueTwo < getLength()));

  size_t firstResidue = residueOne + 1;
  size_t secondResidue = residueTwo + 1;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int cutpointResidue = (int)std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

  foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  core::kinematics::Jump jumpToModify = pose->jump(1);
  core::kinematics::Stub firstStub = (pose->conformation()).upstream_jump_stub(1);
  core::kinematics::Stub secondStub = (pose->conformation()).downstream_jump_stub(1);

  // Create rotation axis and rotation center
  core::Vector oneEnd = (pose->residue(firstResidue)).xyz("CA");
  core::Vector secondEnd = (pose->residue(secondResidue)).xyz("CA");
  core::Vector rotationAxis = oneEnd - secondEnd;

  // Apply rotation
  jumpToModify.set_rb_center(1, secondStub, secondEnd);
  jumpToModify.rotation_by_axis(firstStub, rotationAxis, secondEnd, (float)amplitude);

  // Set new conformation and clear the jump
  pose->set_jump(1, jumpToModify);
  foldTree = pose->fold_tree();
  foldTree.simple_tree(pose->n_residue());
  pose->fold_tree(foldTree);
}

void Pose::applyTranslation(size_t residueOne, size_t residueTwo, double amplitude)
{
  jassert(getLength() > 2);
  jassert(std::abs((double)residueOne - (double)residueTwo) >= 2.0);
  jassert((residueOne >= 0) && (residueOne < getLength()));
  jassert((residueTwo >= 0) && (residueTwo < getLength()));

  size_t firstResidue = residueOne + 1;
  size_t secondResidue = residueTwo + 1;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int cutpointResidue = (int)std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

  foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  protocols::moves::RigidBodyTransMoverOP mover = new protocols::moves::RigidBodyTransMover((*pose), 1);
  mover->step_size(amplitude);
  mover->apply((*pose));

  // Clear the jump
  foldTree.simple_tree(pose->n_residue());
  pose->fold_tree(foldTree);
}

SymmetricMatrixPtr Pose::getBackboneDistanceMatrix() const
{
  SymmetricMatrixPtr matrix;
  matrix = new DoubleSymmetricMatrix(doubleType, 3 * (int)pose->n_residue(), 0.0);

  numeric::xyzVector<double> coordinatesAtomi;
  numeric::xyzVector<double> coordinatesAtomj;

  for (size_t i = 0; i < matrix->getNumRows(); i++)
  {
    for (size_t j = i; j < matrix->getNumColumns(); j++)
    {
      // Get coordinates
      if (i % 3 == 0)
        coordinatesAtomi = (pose->residue((i / 3) + 1)).xyz("N");
      else if (i % 3 == 1)
        coordinatesAtomi = (pose->residue((i / 3) + 1)).xyz("CA");
      else
        coordinatesAtomi = (pose->residue((i / 3) + 1)).xyz("C");

      if (j % 3 == 0)
        coordinatesAtomj = (pose->residue((j / 3) + 1)).xyz("N");
      else if (j % 3 == 1)
        coordinatesAtomj = (pose->residue((j / 3) + 1)).xyz("CA");
      else
        coordinatesAtomj = (pose->residue((j / 3) + 1)).xyz("C");

      matrix->setElement(i, j, coordinatesAtomi.distance(coordinatesAtomj));
    }
  }
  return matrix;
}

void Pose::computeMeanDistances(size_t cutoff, double* shortRange, double* longRange) const
{
  size_t res = getLength();
  size_t shortCount = 0;
  size_t longCount = 0;
  DenseDoubleVectorPtr first;
  DenseDoubleVectorPtr second;
  double distance;

  for (size_t i = 0; i < res; i++)
  {
    first = getCalphaPosition(i);
    for (size_t j = 0; j < res; j++)
    {
      second = getCalphaPosition(j);
      distance = first->l2norm(second);

      if (std::abs((int)i - (int)j) < cutoff)
      {
        if (shortRange != NULL)
        {
          *shortRange += distance;
          shortCount++;
        }
      }
      else
      {
        if (longRange != NULL)
        {
          *longRange += distance;
          longCount++;
        }
      }
    }
  }

  if (shortRange != NULL)
    *shortRange /= shortCount;
  if (longRange != NULL)
    *longRange /= longCount;
}

double Pose::computeMinimumDistance() const
{
  size_t res = getLength();
  DenseDoubleVectorPtr first;
  DenseDoubleVectorPtr second;
  double distance = 0;
  double optimalDistance = INFINITY;

  for (size_t i = 0; i < res; i++)
  {
    first = getCalphaPosition(i);
    for (size_t j = 0; j < res; j++)
    {
      second = getCalphaPosition(j);
      distance = first->l2norm(second);

      optimalDistance = distance < optimalDistance ? distance : optimalDistance;
    }
  }
  return optimalDistance;
}

double Pose::computeMaximumDistance() const
{
  size_t res = getLength();
  DenseDoubleVectorPtr first;
  DenseDoubleVectorPtr second;
  double distance = 0;
  double optimalDistance = 0;

  for (size_t i = 0; i < res; i++)
  {
    first = getCalphaPosition(i);
    for (size_t j = 0; j < res; j++)
    {
      second = getCalphaPosition(j);
      distance = first->l2norm(second);

      optimalDistance = distance > optimalDistance ? distance : optimalDistance;
    }
  }
  return optimalDistance;
}

void Pose::initializeEnergyFunction()
{
  score_fct = core::scoring::ScoreFunctionFactory::create_score_function("standard");
  isEnergyFunctionInitialized = true;
}

double Pose::getEnergy()
{
  if (!isEnergyFunctionInitialized)
    initializeEnergyFunction();
  return (*score_fct)(*pose);
}

double Pose::getCorrectedEnergy()
  {return getEnergy() + getDistanceCorrectionFactor() + getCollisionCorrectionFactor();}

double Pose::getDistanceCorrectionFactor() const
{
  double meanCN = 1.323;
  double stdCN = 0.1;
  double meanCAN = 1.4646;
  double stdCAN = 0.1;
  double meanCAC = 1.524;
  double stdCAC = 0.1;
  double correctionFactor = 0;
  int numberResidues = pose->n_residue();

  numeric::xyzVector<double> coordinatesC;
  numeric::xyzVector<double> coordinatesCA;
  numeric::xyzVector<double> coordinatesN;
  numeric::xyzVector<double> coordinatesNiplus1;

  for (int i = 1; i < numberResidues; i++)
  {
    // Get coordinates
    coordinatesC = (pose->residue(i)).xyz("C");
    coordinatesCA = (pose->residue(i)).xyz("CA");
    coordinatesN = (pose->residue(i)).xyz("N");
    coordinatesNiplus1 = (pose->residue(i + 1)).xyz("N");

    // Calculate correction factor
    correctionFactor += std::exp(std::pow(
        (coordinatesN.distance(coordinatesCA) - meanCAN) / stdCAN, 2));
    correctionFactor += std::exp(std::pow(
        (coordinatesCA.distance(coordinatesC) - meanCAC) / stdCAC, 2));
    correctionFactor += std::exp(std::pow((coordinatesC.distance(coordinatesNiplus1) - meanCN)
        / stdCN, 2));
  }
  // Last residue is special, no bond between C and Niplus1
  coordinatesC = (pose->residue(numberResidues)).xyz("C");
  coordinatesCA = (pose->residue(numberResidues)).xyz("CA");
  coordinatesN = (pose->residue(numberResidues)).xyz("N");

  // Calculate correction factor
  correctionFactor += std::exp(std::pow((coordinatesN.distance(coordinatesCA) - meanCAN) / stdCAN,
      2));
  correctionFactor += std::exp(std::pow((coordinatesCA.distance(coordinatesC) - meanCAC) / stdCAC,
      2));

  return juce::jmax(0.0, correctionFactor - (double)3 * numberResidues + 1);
}

double Pose::getCollisionCorrectionFactor() const
{
  double correctionFactor = 0.0;
  // determined by inspecting all proteins, computing minimum distance for each
  // of them and then taking mean and variance of these values.
  // true value for average = 2.28, true value for variance = 0.0133
  double averageMinimumDistance = 1.5;
  double varianceMinimumDistance = 0.13;
  double inverseVarianceMinimumDistance = 1.0 / varianceMinimumDistance;

  SymmetricMatrixPtr bbDistances = getBackboneDistanceMatrix();

  for (size_t i = 0; i < bbDistances->getNumRows(); i++)
    for (size_t j = i + 2; j < bbDistances->getNumColumns(); j++)
      correctionFactor += juce::jmax(0.0, std::exp(inverseVarianceMinimumDistance
          * (averageMinimumDistance - bbDistances->getElement(i, j).getDouble())) - 1);

  return juce::jmax(0.0, correctionFactor);
}

DenseDoubleVectorPtr Pose::getHistogram() const
{
  DenseDoubleVectorPtr histogram = new DenseDoubleVector(aminoAcidTypeEnumeration, doubleType);
  double increment = 1.0 / pose->n_residue();
  for (size_t i = 0; i < (size_t)pose->n_residue(); i++)
  {
    char n = pose->residue(i + 1).name1();
    std::string name(&n, 1);
    String resName(name.c_str());

    for (size_t j = 0; j < aminoAcidTypeEnumeration->getNumElements(); j++)
      if (!resName.compare(aminoAcidTypeEnumeration->getElement(j)->getVariable(1).toString()))
      {
        histogram->incrementValue(j, increment);
        break;
      }
  }
  return histogram;
}

# else
Pose::Pose(const String& sequence)
  {jassert(false);}

Pose::Pose(const ProteinPtr& protein)
  {jassert(false);}

Pose::Pose(const File& pdbFile)
  {jassert(false);}

void Pose::saveToPDB(const File& pdbFile) const
  {jassert(false);}

PosePtr Pose::clone() const
{
  jassert(false);
  return PosePtr();
}

size_t Pose::getLength() const
{
  jassert(false);
  return 0;
}

DoubleVectorPtr Pose::getCalphaPosition(size_t residue) const
{
  jassert(false);
  return DenseDoubleVectorPtr();
}

void Pose::initializeToHelix()
  {jassert(false);}

double Pose::getPhi(size_t residue) const
{
  jassert(false);
  return 0.0;
}

double Pose::getPsi(size_t residue) const
{
  jassert(false);
  return 0.0;
}

void Pose::setPhi(size_t residue, double phi)
  {jassert(false);}

void Pose::setPsi(size_t residue, double psi)
  {jassert(false);}

void Pose::applyRotation(size_t residueOne, size_t residueTwo, double amplitude)
  {jassert(false);}

void Pose::applyTranslation(size_t residueOne, size_t residueTwo, double amplitude)
  {jassert(false);}

SymmetricMatrixPtr Pose::getBackboneDistanceMatrix() const
{
  jassert(false);
  return SymmetricMatrixPtr();
}

void Pose::computeMeanDistances(size_t cutoff, double* shortRange, double* longRange) const
  {jassert(false);}

double Pose::computeMinimumDistance() const
{
  jassert(false);
  return 0.0;
}

double Pose::computeMaximumDistance() const
{
  jassert(false);
  return 0.0;
}

void Pose::initializeEnergyFunction()
  {jassert(false);}

double Pose::getEnergy()
{
  jassert(false);
  return 0.0;
}

double Pose::getCorrectedEnergy()
{
  jassert(false);
  return 0.0;
}

double Pose::getDistanceCorrectionFactor() const
{
  jassert(false);
  return 0.0;
}

double Pose::getCollisionCorrectionFactor() const
{
  jassert(false);
  return 0.0;
}

DenseDoubleVectorPtr Pose::getHistogram() const
{
  jassert(false);
  return DenseDoubleVectorPtr();
}

#endif //! LBCPP_PROTEIN_ROSETTA
