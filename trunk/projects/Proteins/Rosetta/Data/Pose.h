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
# include <lbcpp/Core/CompositeFunction.h>

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
extern ClassPtr poseClass;
typedef ReferenceCountedObjectPtr<Pose> PosePtr;

class Pose : public Object
{
public:
  /*
   * Constructor
   */
  Pose() {}
  Pose(const String& sequence);
  Pose(const File& pdbFile);
  Pose(const PosePtr& copy);

  /*
   * I/O
   */
  void saveToPDB(const File& pdbFile);
  Pose& operator=(const Pose& copy);

  /*
   * Structure
   */
  size_t getLength();
  double getPhi(size_t residue);
  double getPsi(size_t residue);
  void setPhi(size_t residue, double phi);
  void setPsi(size_t residue, double psi);
  SymmetricMatrixPtr getBackboneDistanceMatrix();

  /*
   * Energy
   */
  void initializeEnergyFunction();
  double getEnergy();
  double getCorrectedEnergy();
  double getDistanceCorrectionFactor();
  double getCollisionCorrectionFactor();

  /*
   * Features
   */
  DenseDoubleVectorPtr getHistogram();
  void setFeatureGenerator(CompositeFunctionPtr& features);
  CompositeFunctionPtr getFeatureGenerator();
  Variable getFeatures(ExecutionContext& context);

protected:
  friend class PoseClass;

  CompositeFunctionPtr features;

# ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
  core::scoring::ScoreFunctionOP score_fct;
# endif //! LBCPP_PROTEIN_ROSETTA
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_
