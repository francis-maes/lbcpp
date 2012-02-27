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
#  include <core/pose/Pose.hh>
#  include <core/scoring/ScoreFunction.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class PoseFeatures;
typedef ReferenceCountedObjectPtr<PoseFeatures> PoseFeaturesPtr;

class Pose;
extern ClassPtr poseClass;
typedef ReferenceCountedObjectPtr<Pose> PosePtr;

class Pose : public Object
{
public:
  /*
   * Constructor
   */
  Pose() {}
  explicit Pose(const String& sequence);
  explicit Pose(const File& pdbFile);

  /*
   * I/O and copy
   */
  void saveToPDB(const File& pdbFile) const;
  PosePtr clone();

  /*
   * Structure
   */
  size_t getLength() const;
  double getPhi(size_t residue) const;
  double getPsi(size_t residue) const;
  void setPhi(size_t residue, double phi);
  void setPsi(size_t residue, double psi);
  SymmetricMatrixPtr getBackboneDistanceMatrix() const;

  /*
   * Energy
   */
  double getEnergy() const;
  double getCorrectedEnergy() const;
  double getDistanceCorrectionFactor() const;
  double getCollisionCorrectionFactor() const;

  /*
   * Features and feature generator
   */
  void setFeatureGenerator(ExecutionContext& context, PoseFeaturesPtr& features);
  PoseFeaturesPtr getFeatureGenerator() const;
  Variable getFeatures(ExecutionContext& context);

  /*
   * Features
   */
  DenseDoubleVectorPtr getHistogram();

protected:
  void initializeEnergyFunction();

  friend class PoseClass;

  PoseFeaturesPtr featureGenerator;

# ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
  core::scoring::ScoreFunctionOP score_fct;
# endif //! LBCPP_PROTEIN_ROSETTA
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_
