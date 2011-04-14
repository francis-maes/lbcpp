/**
 * author: Alejandro Marcos Alvarez
 * date: 03/04/2011
 * name: RosettaUtils.h
 */

#ifndef LBCPP_PROTEINS_ROSETTAUTILS_H_
#define LBCPP_PROTEINS_ROSETTAUTILS_H_

#include "../Data/AminoAcid.h"
#include "../Data/Protein.h"
#include "../Data/Residue.h"
#include "../Data/Formats/PDBFileGenerator.h"
#include "../Geometry/Vector3.h"

#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>

#undef T
#include <core/chemical/AtomType.hh>
#include <core/chemical/ChemicalManager.hh>
#include <core/chemical/util.hh>
#include <core/chemical/ResidueType.hh>
#include <core/conformation/Residue.hh>
#include <core/init.hh>
#include <core/io/pdb/file_data.hh>
#include <core/io/pdb/pose_io.hh>
#include <core/pose/PDBInfo.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <numeric/xyzVector.hh>
#define T JUCE_T

namespace lbcpp
{

typedef std::map<std::string, Vector> ResidueCoords;

/**
 * Converts Protein object to Pose object.
 * @param protein pointer to the Protein object to convert
 * @return a pointer to the Pose object
 */
core::pose::PoseOP convertProtein2Pose(const ProteinPtr protein);

/**
 * Converts Pose object to Protein object.
 * @param pose a pointer to the Pose object to convert
 * @return a pointer to the Protein object
 */
//ProteinPtr convertPose2Protein(const core::pose::PoseOP pose);
ProteinPtr convertPose2Protein(ExecutionContext& context, const core::pose::PoseOP pose);

/**
 * Returns the total energy of a Protein object.
 * @param protein a pointer to the Protein object
 * @return the total energy of the protein
 */
double getTotalEnergy(const ProteinPtr prot);

/**
 * Returns the total energy of a Pose object.
 * @param pose a pointer to the Pose object
 * @return the total energy of the protein
 */
double getTotalEnergy(const core::pose::PoseOP pose);

/**
 * Initializes a Pose object, in fact its pointer, to the given sequence.
 * @param pose a pointer to the Pose object to initialize
 * @param sequence the sequence of amino acids of the protein
 */
void makePoseFromSequence(core::pose::PoseOP pose, const String& sequence);

/**
 * Initializes Rosetta.
 * @param verbose sets verbosity level. true or false.
 */
void rosettaInit(bool verbose);

/**
 * Initializes Rosetta with verbosity set to true.
 */
void rosettaInit();

/**
 * Generates random number uniformly distributed between 0 and 1.
 * @return a random double uniformly distributed between 0 and 1.
 */
double generateRand();

/**
 * Generates random numbers following normal distribution with mean 0
 * and standard deviation of 1.
 * @return a normally distributed random double.
 */
double generateNormalRand();
}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTAUTILS_H_
