/*-----------------------------------------.---------------------------------.
| Filename: RosettaTest.h                  | Rosetta Test                    |
| Author  : Francis Maes                   |                                 |
| Started : 30/03/2011 13:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_TEST_H_
# define LBCPP_PROTEINS_ROSETTA_TEST_H_

# include "precompiled.h"
# include "../Data/Protein.h"
# include "../Data/AminoAcid.h"
# include "../Data/Residue.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "../Evaluator/QScoreEvaluator.h"
# include "RosettaUtils.h"
# include "RosettaMover.h"
# include "RosettaOptimizer.h"
# include <iostream>
# include <string>
# include <vector>
# include <cmath>
# include <time.h>

# undef T
#  include <core/pose/Pose.hh>
#  include <core/chemical/util.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <protocols/init.hh>
# define T JUCE_T

using namespace std;

namespace lbcpp
{

class RosettaTest: public WorkUnit
{
private:
  friend class RosettaTestClass;
  size_t arg;
  String proteinsDir;
  std::vector<double> parameters;

public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);

    File fichier = context.getFile(T("init.pdb"));
    //File fichier("/Users/alex/Desktop/2KX7.pdb");
    File fichierout("/Users/alex/Desktop/init_lbcpp.pdb");
    ProteinPtr protein = Protein::createFromPDB(context, fichier, true);
    core::pose::PoseOP pose = convertProteinToPose(context, protein);

    // Number of residues
       int numRes = pose->n_residue();

       // Tertiary structure
       TertiaryStructurePtr ts = protein->getTertiaryStructure();
       for (int i = 0; i < numRes; i++)
       {
         ResiduePtr residue = ts->getResidue(i);

       // rosetta : get residue information
       core::conformation::Residue tempResRos = pose->residue(i + 1);
       core::chemical::ResidueType tempResRosType = pose->residue_type(i + 1);
       int nbatoms = tempResRos.natoms();

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
       }
       }

    protein->saveToPDBFile(context, fichierout);
    core::io::pdb::dump_pdb((*pose), "/Users/alex/Desktop/init_rosetta.pdb");

    //pose_from_pdb(*pose2, );
    //ProteinPtr prot1 = convertPoseToProtein(context, pose2);
    //core::pose::PoseOP pose3 = convertProteinToPose(context, prot1);

    //cout << "energie cas 1 : " << getTotalEnergy(pose) << endl;
    //cout << "energie cas 2 : " << getTotalEnergy(pose2) << endl;
/*
    std::ostringstream oss;
      core::io::pdb::FileData::dump_pdb((*pose2), oss);
      oss.flush();
      std::string poseString = oss.str();
      String pdbString(poseString.c_str());
*/
      //cout << "pose2 : " << pdbString <<endl;

//    core::pose::PoseOP pose = new core::pose::Pose();
//    makePoseFromSequence(pose, T("AAAAAA"));
//    ProteinPtr protein = convertPoseToProtein(context, pose);


    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_
