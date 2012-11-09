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
# include "../../Data/Protein.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/pose/Pose.hh>
#  include <core/scoring/ScoreFunction.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class Pose;
extern ClassPtr poseClass;
typedef ReferenceCountedObjectPtr<Pose> PosePtr;

class Pose : public Object
{
public:
  /*
   * Constructor
   */
  Pose();
  explicit Pose(const String& sequence);
  explicit Pose(const ProteinPtr& protein);
  explicit Pose(const File& pdbFile);

  /*
   * I/O and copy
   */
  void saveToPDB(const File& pdbFile) const;
  PosePtr clone() const;

  /*
   * Structure - miscellaneous
   */
  size_t getLength() const;
  String getAminoAcidSequence() const;
  DoubleVectorPtr getCalphaPosition(size_t residue) const;
  void initializeToHelix();

  /*
   * Structure - angles
   */
  double getPhi(size_t residue) const;
  double getPsi(size_t residue) const;
  void setPhi(size_t residue, double phi);
  void setPsi(size_t residue, double psi);
  void applyRotation(size_t residueOne, size_t residueTwo, double amplitude);
  void applyTranslation(size_t residueOne, size_t residueTwo, double amplitude);


  /*
   * Structure - distances
   */
  SymmetricMatrixPtr getBackboneDistanceMatrix() const;
  void computeMeanDistances(size_t cutoff, double* shortRange, double* longRange) const;
  double computeMinimumDistance() const;
  double computeMaximumDistance() const;

  /*
   * Energy
   */
  double getEnergy();
  double getCorrectedEnergy();
  double getDistanceCorrectionFactor() const;
  double getCollisionCorrectionFactor() const;

  /*
   * Features
   */
  DenseDoubleVectorPtr getAminoAcidHistogram() const;

protected:
  void initializeEnergyFunction();

  friend class PoseClass;

  bool isEnergyFunctionInitialized;

# ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
  core::scoring::ScoreFunctionOP score_fct;
# endif //! LBCPP_PROTEIN_ROSETTA
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_