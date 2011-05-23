/*-----------------------------------------.---------------------------------.
| Filename:  RosettaUtils.h                | RosettaUtils                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 03/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_UTILS_H_
# define LBCPP_PROTEINS_ROSETTA_UTILS_H_
//# define LBCPP_PROTEIN_ROSETTA

# include "precompiled.h"
# include "../Data/AminoAcid.h"
# include "../Data/Protein.h"
# include "../Data/Residue.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "../Geometry/Vector3.h"

# include <time.h>
# include <stdlib.h>
# include <cmath>
# include <iostream>

# ifdef LBCPP_PROTEIN_ROSETTA

#  undef T
#  include <core/chemical/AtomType.hh>
#  include <core/chemical/ChemicalManager.hh>
#  include <core/chemical/util.hh>
#  include <core/chemical/ResidueType.hh>
#  include <core/conformation/Residue.hh>
#  include <core/init.hh>
#  include <core/scoring/ScoreType.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/pose/PDBInfo.hh>
#  include <core/pose/Pose.hh>
#  include <core/scoring/ScoreFunction.hh>
#  include <core/scoring/ScoreFunctionFactory.hh>
#  include <numeric/xyzVector.hh>
#  include <protocols/moves/SwitchResidueTypeSetMover.hh>
#  define T JUCE_T

# else // predeclare rosetta

namespace utility {namespace pointer{
  template< typename T > class owning_ptr;
}; };

namespace core { namespace pose {
  class Pose;
  typedef utility::pointer::owning_ptr< Pose > PoseOP;
}; };

#endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

typedef std::map<std::string, Vector> ResidueCoords;

/**
 * Returns a standardized version of atoms names to match atoms form lbcpp
 * and rosetta. All atoms names begin with letters and can end with a number.
 * @param atomName the name of the atom.
 * @return the standardized name
 */
String standardizedAtomName(const String& atomName);

/**
 * Converts Protein object to Pose object.
 * @param protein pointer to the Protein object to convert
 * @param result pointer
 */
void convertProteinToPose(ExecutionContext& context, const ProteinPtr& protein, core::pose::PoseOP& res);

/**
 * Converts Pose object to Protein object.
 * @param pose a pointer to the Pose object to convert
 * @return a pointer to the Protein object
 */
ProteinPtr convertPoseToProtein(ExecutionContext& context, const core::pose::PoseOP& pose);

double fullAtomEnergy(const core::pose::PoseOP& pose);
double centroidEnergy(const core::pose::PoseOP& pose);

double getNormalizedEnergy(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&));
double getNormalizedScore(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&));

/**
 * Returns the total energy of a Protein object.
 * @param protein a pointer to the Protein object
 * @return the total energy of the protein
 */
double getTotalEnergy(ExecutionContext& context, const ProteinPtr& prot, double(*scoreFunction)(
    const core::pose::PoseOP&));

/**
 * Returns the total energy of a Pose object.
 * @param pose a pointer to the Pose object
 * @return the total energy of the protein
 */
double getTotalEnergy(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&));

/**
 * Returns the score of the current pose. This takes into account
 * the average distance between backbone atoms.
 * @param prot a pointer to the protein object to evaluate.
 * @return the score of the actual conformation.
 */
double getConformationScore(ExecutionContext& context, const ProteinPtr& prot,
    double(*scoreFunction)(const core::pose::PoseOP&));

/**
 * Returns the score of the current pose. This takes into account
 * the average distance between backbone atoms.
 * @param pose a pointer to the pose object to evaluate.
 * @return the score of the actual conformation.
 */
double getConformationScore(const core::pose::PoseOP& pose, double(*scoreFunction)(
    const core::pose::PoseOP&), double* energy = NULL);

/**
 * Initializes a Pose object, in fact its pointer, to the given sequence.
 * @param pose a pointer to the Pose object to initialize
 * @param sequence the sequence of amino acids of the protein
 */
void makePoseFromSequence(core::pose::PoseOP& pose, const String& sequence);

/**
 * Initializes Rosetta. Rosetta database must be placed in the project directory and named
 * rosetta_database.
 * @param verbose sets verbosity level. true or false.
 */
void rosettaInitialization(ExecutionContext& context, bool verbose);

/**
 * Initializes Rosetta with verbosity set to true. Rosetta database must be placed in
 * the project directory and named rosetta_database.
 */
void rosettaInitialization(ExecutionContext& context);

void initializeProteinStructure(const core::pose::PoseOP& pose, core::pose::PoseOP& res);

SymmetricMatrixPtr createCalphaMatrixDistance(const core::pose::PoseOP& pose);

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_UTILS_H_
