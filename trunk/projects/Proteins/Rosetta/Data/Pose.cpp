/*-----------------------------------------.---------------------------------.
| Filename: Pose.cpp                       | Pose source                     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:09 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Data/SymmetricMatrix.h>
#include "Pose.h"

using namespace lbcpp;

#ifdef LBCPP_PROTEIN_ROSETTA

Pose::Pose(const String& sequence)
{
  jassert(sequence.length() != 0);
  pose = new core::pose::Pose();
  core::chemical::make_pose_from_sequence(*pose, (const char*)sequence,
      core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set("fa_standard"));
}

Pose::Pose(const File& pdbFile)
{
  jassert(pdbFile != File::nonexistent);
  pose = new core::pose::Pose((const char*)pdbFile.getFullPathName());
  if (pose() == NULL)
    jassert(false);
}

Pose::Pose(const PosePtr& copy)
{
  pose = new core::pose::Pose();
  *pose = *copy->pose;
}

void Pose::saveToPDB(const File& pdbFile)
{
  if (!pose->dump_pdb((const char*)pdbFile.getFullPathName()))
    jassert(false);
}

Pose& Pose::operator=(const Pose& copy)
{
  *pose = *copy.pose;
  return *this;
}

size_t Pose::getLength()
  {return pose->n_residue();}

double Pose::getPhi(size_t residue)
  {return pose->phi(residue + 1);}

double Pose::getPsi(size_t residue)
  {return pose->psi(residue + 1);}

void Pose::setPhi(size_t residue, double phi)
  {pose->set_phi(residue + 1, phi);}

void Pose::setPsi(size_t residue, double psi)
  {pose->set_psi(residue + 1, psi);}

SymmetricMatrixPtr Pose::getBackboneDistanceMatrix()
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

void Pose::initializeEnergyFunction()
  {score_fct = core::scoring::ScoreFunctionFactory::create_score_function("standard");}

double Pose::getEnergy()
  {return (*score_fct)(*pose);}

double Pose::getCorrectedEnergy()
  {return getEnergy() + getDistanceCorrectionFactor() + getCollisionCorrectionFactor();}

double Pose::getDistanceCorrectionFactor()
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

double Pose::getCollisionCorrectionFactor()
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

# else
Pose::Pose(const String& sequence)
  {jassert(false);}

Pose::Pose(const File& pdbFile)
  {jassert(false);}

Pose::Pose(const PosePtr& copy)
  {jassert(false);}

void Pose::saveToPDB(const File& pdbFile)
  {jassert(false);}

Pose& Pose::operator=(const Pose& copy)
{
  jassert(false);
  return *this;
}

size_t Pose::getLength()
{
  jassert(false);
  return 0;
}

double Pose::getPhi(size_t residue)
{
  jassert(false);
  return 0.0;
}

double Pose::getPsi(size_t residue)
{
  jassert(false);
  return 0.0;
}

void Pose::setPhi(size_t residue, double phi)
  {jassert(false);}

void Pose::setPsi(size_t residue, double psi)
  {jassert(false);}

SymmetricMatrixPtr Pose::getBackboneDistanceMatrix()
{
  jassert(false);
  return SymmetricMatrixPtr();
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

double Pose::getDistanceCorrectionFactor()
{
  jassert(false);
  return 0.0;
}

double Pose::getCollisionCorrectionFactor()
{
  jassert(false);
  return 0.0;
}

#endif //! LBCPP_PROTEIN_ROSETTA
