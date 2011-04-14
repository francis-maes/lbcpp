/**
 * author: Alejandro Marcos Alvarez
 * date: 03/04/2011
 * name: RosettaUtils.cpp
 */

#include "RosettaUtils.h"

namespace lbcpp
{

core::pose::PoseOP convertProtein2Pose(const ProteinPtr protein)
{
	core::pose::PoseOP pose = new core::pose::Pose();
	juce::String proteinPDBString = PDBFileGenerator::producePDBString(protein);
	core::io::pdb::pose_from_pdbstring((*pose), (const char*) proteinPDBString);
	return pose;
}

ProteinPtr convertPose2Protein(ExecutionContext& context, const core::pose::PoseOP pose)
{
	std::ostringstream oss;
	core::io::pdb::FileData::dump_pdb((*pose), oss);
	oss.flush();
	std::string poseString = oss.str();
	juce::String pdbString(poseString.c_str());

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
	 AtomPtr tempAtom = new Atom((juce::String) (atomNameRos.c_str()),
	 (juce::String) (atomTypeRos.c_str()), v3p);

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

double getTotalEnergy(const ProteinPtr prot)
{
	core::pose::PoseOP tempPose = convertProtein2Pose(prot);
	return getTotalEnergy(tempPose);
}

double getTotalEnergy(core::pose::PoseOP pose)
{
	core::scoring::ScoreFunctionOP score_fxn =
			core::scoring::ScoreFunctionFactory::create_score_function("standard");
	return (*score_fxn)(*pose);
}

void rosettaInit()
{
	rosettaInit(true);
}

void rosettaInit(bool verbose)
{
	srand(time(NULL));
	int argc = 3;
	if (!verbose)
	{
		argc = 5;
	}
	char* argv[argc];
	argv[0] = (char*) "./main";
	argv[1] = (char*) "-database";
	if (!verbose)
	{
		argv[3] = (char*) "-mute";
		argv[4] = (char*) "all";
	}
#ifdef JUCE_LINUX
	// path to find rosetta database in monster24
	argv[2] = (char*)"/home/alejandro/soft/rosetta-3.2/rosetta_database";
#elif JUCE_MAC
	// path to find rosetta database in personnal mac
	argv[2] = (char*) "/Users/alex/Documents/Ulg/2M/rosetta3.2_macos/rosetta_database";
#endif
	core::init(argc, argv);
}

void makePoseFromSequence(core::pose::PoseOP pose, const String& sequence)
{
	core::chemical::make_pose_from_sequence(*pose, (const char*) sequence,
			core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set(
					"FA_STANDARD"));
}

double generateRand()
{
	return (double) rand() / (double) RAND_MAX;
}

double generateNormalRand()
{
	double val = 2;
	double x1;
	double x2;
	while (val >= 1)
	{
		x1 = (2 * generateRand()) - 1;
		x2 = (2 * generateRand()) - 1;
		val = std::pow(x1, 2) + std::pow(x2, 2);
	}
	return (x1 * std::sqrt(((double) -2 * std::log(val)) / val));
}
}
; /* namespace lbcpp */
