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
#  include <core/io/pdb/pose_io.hh>
#  include <core/pose/Pose.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class Pose : public Object
{
public:
  void createFromSequence(const String& sequence);
  void createFromPDB(const File& pdbFile);
  //createFromXml

  //saveToPDB
  //saveToXml
  //
  //getPhi
  //getPsi
  //setPhi
  //setPsi
  //
  //getRawEnergy
  //getCorrectedEnergy
  //
  //getDistanceCorrectionFactor
  //getCollisionCorrectionFactor
protected:
  friend class PoseClass;
# ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
# endif //! LBCPP_PROTEIN_ROSETTA
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_POSE_H_
