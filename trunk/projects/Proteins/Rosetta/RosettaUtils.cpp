/*-----------------------------------------.---------------------------------.
| Filename:  RosettaUtils.cpp              | RosettaUtils                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 03/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "RosettaUtils.h"
using namespace lbcpp;

String lbcpp::standardizedAtomName(const String& atomName)
{
  String workingCopy = atomName;
  workingCopy = workingCopy.trimEnd();
  workingCopy = workingCopy.trimStart();
  String numbers = workingCopy.initialSectionContainingOnly(String("0123456789"));
  String letters = workingCopy.substring(numbers.length(), workingCopy.length());
  return String(letters + numbers);
}

void lbcpp::convertProteinToPose(ExecutionContext& context, const ProteinPtr& protein,
    core::pose::PoseOP& res)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  res = new core::pose::Pose();
  String proteinPDBString = PDBFileGenerator::producePDBString(context, protein);
  core::io::pdb::pose_from_pdbstring((*res), (const char*)proteinPDBString);

  if ((int)res->n_residue() != (int)protein->getLength())
  {
    context.errorCallback(T("convertProteinToPose"), protein->getName());
    res.reset_to_null();
  }

#endif // LBCPP_PROTEIN_ROSETTA
}

ProteinPtr lbcpp::convertPoseToProtein(ExecutionContext& context, const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA

  RandomGeneratorPtr rg = context.getRandomGenerator();
  bool error = false;
  ProteinPtr prot;
  String name = String(pose->pdb_info()->name().c_str());

  File tempFile = context.getFile(T("tmpConv") +name+ String(Time::currentTimeMillis())
      + String(rg->sampleInt(0, INT_MAX)) + T(".pdb"));

  int i = 0;
  while ((tempFile.exists()) && !error)
  {
    tempFile = context.getFile(T("tmpFileConvPoseToProtein") + String(Time::currentTimeMillis())
        + String(rg->sampleInt(0, INT_MAX)) + T(".pdb"));
    i++;
    if (i >= 100)
      error = true;
  }

  if (!error)
  {
    core::io::pdb::dump_pdb(*pose, (const char*)tempFile.getFullPathName());
    prot = Protein::createFromPDB(context, tempFile, true);
    tempFile.deleteFile();
  }
  else
    return NULL;

  error = error ? true : (int)pose->n_residue() != (int)prot->getLength();

  if (error)
  {
    context.errorCallback(T("convertPoseToProtein"), name);
    return NULL;
  }

  return prot;

  //  std::ostringstream oss;
  //  core::io::pdb::FileData::dump_pdb((*pose), oss);
  //  oss.flush();
  //  std::string poseString = oss.str();
  //  String pdbString(poseString.c_str());
  //
  //  ProteinPtr prot = Protein::createFromPDB(context, pdbString, true);
  //  return prot;
#else
  jassert(false);
  return ProteinPtr();
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::fullAtomEnergy(const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  core::scoring::ScoreFunctionOP score_fxn =
      core::scoring::ScoreFunctionFactory::create_score_function("standard");
  return (*score_fxn)(*pose);
#else
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::centroidEnergy(const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  core::scoring::ScoreFunctionOP score_fxn =
      core::scoring::ScoreFunctionFactory::create_score_function("cen_std");
  protocols::moves::SwitchResidueTypeSetMoverOP switcher =
      new protocols::moves::SwitchResidueTypeSetMover("centroid");
  core::pose::PoseOP energyPose = new core::pose::Pose();
  *energyPose = *pose;
  switcher->apply(*energyPose);
  return (*score_fxn)(*energyPose);
#else
  return 0.0;
#endif
}

double lbcpp::getTotalEnergy(ExecutionContext& context, const ProteinPtr& prot,
    double(*scoreFunction)(const core::pose::PoseOP&))
{
#ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP tempPose;
  convertProteinToPose(context, prot, tempPose);
  return getTotalEnergy(tempPose, scoreFunction);
#else
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::getTotalEnergy(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&))
{
  return (*scoreFunction)(pose);
}

double lbcpp::getConformationScore(ExecutionContext& context, const ProteinPtr& prot,
    double(*scoreFunction)(const core::pose::PoseOP&))
{
#ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP tempPose;
  convertProteinToPose(context, prot, tempPose);
  return getConformationScore(tempPose, scoreFunction);
#else
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double formattingCorrectionFactor(double value, double mean, double std)
{
  return std::exp(std::pow((value - mean) / std, 2));
}

double lbcpp::computeCorrectionFactorForDistances(const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  // Correct the energy function
  double meanCN = 1.323;
  double stdCN = 0.1;
  double meanCAN = 1.4646;
  double stdCAN = 0.1;
  double meanCAC = 1.524;
  double stdCAC = 0.1;
  double correctionFactor = 0;
  int numberResidues = pose->n_residue();

  for (int i = 1; i < numberResidues; i++)
  {
    // Get coordinates
    numeric::xyzVector<double> coordinatesC = (pose->residue(i)).xyz("C");
    numeric::xyzVector<double> coordinatesCA = (pose->residue(i)).xyz("CA");
    numeric::xyzVector<double> coordinatesN = (pose->residue(i)).xyz("N");
    numeric::xyzVector<double> coordinatesNiplus1 = (pose->residue(i + 1)).xyz("N");

    // Calculate correction factor
    correctionFactor += formattingCorrectionFactor(coordinatesN.distance(coordinatesCA), meanCAN,
        stdCAN);
    correctionFactor += formattingCorrectionFactor(coordinatesCA.distance(coordinatesC), meanCAC,
        stdCAC);
    correctionFactor += formattingCorrectionFactor(coordinatesC.distance(coordinatesNiplus1),
        meanCN, stdCN);
  }
  // Last residue is special, no bond between C and Niplus1
  numeric::xyzVector<double> coordinatesC = (pose->residue(numberResidues)).xyz("C");
  numeric::xyzVector<double> coordinatesCA = (pose->residue(numberResidues)).xyz("CA");
  numeric::xyzVector<double> coordinatesN = (pose->residue(numberResidues)).xyz("N");

  // Calculate correction factor
  correctionFactor += formattingCorrectionFactor(coordinatesN.distance(coordinatesCA), meanCAN,
      stdCAN);
  correctionFactor += formattingCorrectionFactor(coordinatesCA.distance(coordinatesC), meanCAC,
      stdCAC);

  return juce::jmax(0.0, correctionFactor - (double)3 * numberResidues + 1);

#else
  jassert(false);
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::computeCorrectionFactorForCollisions(const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  double correctionFactor = 0.0;

  // determined by inspecting all proteins, computing minimum distance for each
  // of them and then taking mean and variance of these values.
  // true value for average = 2.28, true value for variance = 0.0133
  double averageMinimumDistance = 1.5;
  double varianceMinimumDistance = 0.13;

  double inverseVarianceMinimumDistance = 1.0 / varianceMinimumDistance;

  SymmetricMatrixPtr bbDistances = createBackboneMatrixDistance(pose);

  for (size_t i = 0; i < bbDistances->getNumRows(); i++)
    for (size_t j = i + 2; j < bbDistances->getNumColumns(); j++)
      correctionFactor += juce::jmax(0.0, exp(inverseVarianceMinimumDistance
          * (averageMinimumDistance - bbDistances->getElement(i, j).getDouble())) - 1);

  return juce::jmax(0.0, correctionFactor);

#else
  jassert(false);
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::computeCompactness(const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA

  double mean;
  double min;
  double max;
  SymmetricMatrixPtr distMat = createCalphaMatrixDistance(pose, &mean, &min, &max);

  // returns 1 if the structure is a sphere, less than 1 if not.
  return mean / max;

#else
  jassert(false);
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::getConformationScore(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&), double* energy)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  double tempEn = (*scoreFunction)(pose);
  if (energy != NULL)
    *energy = tempEn;
  return tempEn + computeCorrectionFactorForDistances(pose);
#else
  jassert(false);
  return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
}

double lbcpp::sigmoid(double k, double x)
  {return 1 / (1 + std::exp(-k * x));}

double lbcpp::getNormalizedEnergy(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&))
  {return sigmoid(0.0005, getTotalEnergy(pose, scoreFunction));}

double lbcpp::getNormalizedScore(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&))
  {return sigmoid(0.0005, getConformationScore(pose, scoreFunction));}

void lbcpp::rosettaInitialization(ExecutionContext& context)
  {rosettaInitialization(context, true);}

void lbcpp::rosettaInitialization(ExecutionContext& context, bool verbose)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  int argc = 3;
  if (!verbose)
    argc = 5;
  std::vector<char* > argv(argc);
  argv[0] = (char*)"./main";
  argv[1] = (char*)"-database";
  if (!verbose)
  {
    argv[3] = (char*)"-mute";
    argv[4] = (char*)"all";
  }

  juce::File projectDirectory = context.getProjectDirectory();
  String projectDirectoryPath = projectDirectory.getFullPathName();
  String rosettaDatabasePath = projectDirectoryPath + String("/rosetta_database");
  char bufferDatabasePath[300];
  std::string stringDatabasePath = std::string((const char*)rosettaDatabasePath);
  stringDatabasePath += '\0';
  int length = stringDatabasePath.copy(bufferDatabasePath, stringDatabasePath.size() + 1, 0);
  argv[2] = bufferDatabasePath;

  core::init(argc, &argv[0]);
#else
  jassert(false);
#endif // !LBCPP_PROTEIN_ROSETTA
}

void lbcpp::makePoseFromSequence(core::pose::PoseOP& pose, const String& sequence)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  core::chemical::make_pose_from_sequence(*pose, (const char*)sequence,
      core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set("fa_standard"));
#endif // LBCPP_PROTEIN_ROSETTA
}

void lbcpp::initializeProteinStructure(const core::pose::PoseOP& pose, core::pose::PoseOP& res)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  res = new core::pose::Pose((*pose));

  for (size_t i = 1; i <= pose->n_residue(); i++)
  {
    if (i == 1)
      res->set_phi(i, 0);
    else
      res->set_phi(i, -150);
    if (i == pose->n_residue())
      res->set_psi(i, 0);
    else
      res->set_psi(i, 150);
    res->set_omega(i, 45);
  }
#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
}

SymmetricMatrixPtr lbcpp::createCalphaMatrixDistance(const core::pose::PoseOP& pose,
    double* meanDistance, double* minimalDistance, double* maximalDistance)
{
  SymmetricMatrixPtr matrix;

#ifdef LBCPP_PROTEIN_ROSETTA
  matrix = new DoubleSymmetricMatrix(doubleType, (int)pose->n_residue(), 0.0);

  if (meanDistance != NULL)
    *meanDistance = 0.0;
  if (minimalDistance != NULL)
    *minimalDistance = 2 * (double)pose->n_residue();
  if (maximalDistance != NULL)
    *maximalDistance = -1.0;

  double dist;
  int count = 0;

  for (size_t i = 0; i < matrix->getNumRows(); i++)
  {
    for (size_t j = i + 1; j < matrix->getNumColumns(); j++)
    {
      // Get coordinates
      numeric::xyzVector<double> coordinatesCAi = (pose->residue(i + 1)).xyz("CA");
      numeric::xyzVector<double> coordinatesCAj = (pose->residue(j + 1)).xyz("CA");
      dist = coordinatesCAi.distance(coordinatesCAj);

      if (meanDistance != NULL)
      {
        *meanDistance += dist;
        count++;
      }
      if ((minimalDistance != NULL) && (dist < *minimalDistance))
        *minimalDistance = dist;
      else if ((maximalDistance != NULL) && (dist > *maximalDistance))
        *maximalDistance = dist;

      matrix->setElement(i, j, dist);
    }
  }

  if (meanDistance != NULL)
    *meanDistance /= (double)count;

#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  return matrix;
}

SymmetricMatrixPtr lbcpp::createBackboneMatrixDistance(const core::pose::PoseOP& pose)
{
  SymmetricMatrixPtr matrix;
#ifdef LBCPP_PROTEIN_ROSETTA
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
#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  return matrix;
}
