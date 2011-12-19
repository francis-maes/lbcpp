/*-----------------------------------------.---------------------------------.
| Filename: Pose.h                         | Pose                            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:54:45 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_
# define LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_

# include "Rosetta.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/chemical/ChemicalManager.hh>
#  include <core/chemical/util.hh>
#  include <core/conformation/Residue.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/pose/Pose.hh>
#  include <core/scoring/ScoreFunction.hh>
#  include <core/scoring/ScoreFunctionFactory.hh>
#  include <numeric/xyzVector.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class Pose;
typedef ReferenceCountedObjectPtr<Pose> PosePtr;

class Pose : public Object
{
public:
  Pose() {}
  Pose(const String& sequence);
  Pose(const File& pdbFile);
  Pose(const PosePtr& copy);

  void saveToPDB(const File& pdbFile);

  Pose& operator=(const Pose& copy);

  size_t getLength();
  double getPhi(size_t residue);
  double getPsi(size_t residue);
  void setPhi(size_t residue, double phi);
  void setPsi(size_t residue, double psi);
  SymmetricMatrixPtr getBackboneDistanceMatrix();

  double getEnergy();
  double getCorrectedEnergy();
  double getDistanceCorrectionFactor();
  double getCollisionCorrectionFactor();

protected:
  friend class PoseClass;

# ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
# endif //! LBCPP_PROTEIN_ROSETTA
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_
