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

core::pose::PoseOP lbcpp::convertProteinToPose(ExecutionContext& context, const ProteinPtr& protein)
{
  core::pose::PoseOP pose = new core::pose::Pose();
  String proteinPDBString = PDBFileGenerator::producePDBString(context, protein);
  core::io::pdb::pose_from_pdbstring((*pose), (const char*) proteinPDBString);
  return pose;
}

ProteinPtr lbcpp::convertPoseToProtein(ExecutionContext& context, const core::pose::PoseOP& pose)
{
  std::ostringstream oss;
  core::io::pdb::FileData::dump_pdb((*pose), oss);
  oss.flush();
  std::string poseString = oss.str();
  String pdbString(poseString.c_str());

  ProteinPtr prot = Protein::createFromPDB(context, pdbString, true);
  return prot;
  /*
   // Number of residues
   int numres = pose->n_residue();
   ProteinPtr protein = new Protein();

   // Primary structure
   protein->setPrimaryStructure((String) (pose->sequence()).c_str());

   // Tertiary structure
   TertiaryStructurePtr ts = new TertiaryStructure(numres);
   for (int i = 0; i < numres; i++)
   {
   // rosetta : get residue information
   core::conformation::Residue tempResRos = pose->residue(i + 1);
   core::chemical::ResidueType tempResRosType = pose->residue_type(i + 1);
   int nbatoms = tempResRos.natoms();

   // lbcpp : create residue
   ResiduePtr temp = new Residue((AminoAcidType) AminoAcid::fromOneLetterCode(
   (juce::tchar) tempResRosType.name1()).getInteger());

   // fill residue with atoms
   for (size_t j = 0; j < nbatoms; j++)
   {
   // get atoms information
   numeric::xyzVector < core::Real > positionAtomRos = tempResRos.xyz(j + 1);
   std::string atomTypeRos = (tempResRos.atom_type(j + 1)).element();
   std::string atomNameRos = tempResRos.atom_name(j + 1);

   // create atom and set position and occupancy
   impl::Vector3 v3(0.0, 0.0, 0.0);
   Vector3Ptr v3p = new Vector3(v3);
   AtomPtr tempAtom = new Atom((String) (atomNameRos.c_str()),
   (String) (atomTypeRos.c_str()), v3p);

   // Probleme, si pose cree par makePoseFromSequence, energie
   //sensiblement differente... (du a occupancy de toute evidence)
   if ((pose->pdb_info()).get() != NULL)
   tempAtom->setOccupancy((pose->pdb_info())->occupancy(i + 1, j + 1));
   else
   tempAtom->setOccupancy(1.0);

   tempAtom->setX(positionAtomRos.x());
   tempAtom->setY(positionAtomRos.y());
   tempAtom->setZ(positionAtomRos.z());

   // add atom to residue
   temp->addAtom(tempAtom);
   }
   // add residue to tertiary structure
   ts->setResidue(i, temp);
   }

   // add tertiary structure to protein
   protein->setTertiaryStructure(ts);

   return protein;
   */
}

double lbcpp::getTotalEnergy(ExecutionContext& context, const ProteinPtr& prot)
{
  core::pose::PoseOP tempPose = convertProteinToPose(context, prot);
  return getTotalEnergy(tempPose);
}

double lbcpp::getTotalEnergy(const core::pose::PoseOP& pose)
{
  core::scoring::ScoreFunctionOP score_fxn =
      core::scoring::ScoreFunctionFactory::create_score_function("standard");
  return (*score_fxn)(*pose);
}

double lbcpp::getConformationScore(ExecutionContext& context, const ProteinPtr& prot)
{
  core::pose::PoseOP tempPose = convertProteinToPose(context, prot);
  return getConformationScore(tempPose);
}

double formattingCorrectionFactor(double value, double mean, double std)
{
  return std::exp(std::pow((value - mean) / std, 2));
}

double lbcpp::getConformationScore(const core::pose::PoseOP& pose)
{
  core::scoring::ScoreFunctionOP score_fxn =
      core::scoring::ScoreFunctionFactory::create_score_function("standard");

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

  double score = (*score_fxn)(*pose) + juce::jmax(0.0, correctionFactor - (double)3
      * numberResidues + 1);
  return (score >= 1 ? score : std::exp(score - 1));
}

void lbcpp::rosettaInitialization(ExecutionContext& context)
{
  rosettaInitialization(context, true);
}

void lbcpp::rosettaInitialization(ExecutionContext& context, bool verbose)
{
  int argc = 3;
  if (!verbose)
  {
    argc = 5;
  }
  char* argv[3];
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

  core::init(argc, argv);
}

void lbcpp::makePoseFromSequence(core::pose::PoseOP& pose, const String& sequence)
{
  core::chemical::make_pose_from_sequence(*pose, (const char*)sequence,
      core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set("fa_standard"));
}

core::pose::PoseOP lbcpp::initializeProteinStructure(core::pose::PoseOP pose)
{
  core::pose::PoseOP initialized = new core::pose::Pose((*pose));

  for (size_t i = 1; i <= pose->n_residue(); i++)
  {
    if (i == 1)
      initialized->set_phi(i, 0);
    else
      initialized->set_phi(i, -150);
    if (i == pose->n_residue())
      initialized->set_psi(i, 0);
    else
      initialized->set_psi(i, 150);
    initialized->set_omega(i, 45);
  }

  return initialized;
}
