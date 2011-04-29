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
# include "ProteinMover.h"
# include "ProteinOptimizer.h"
# include <iostream>
# include <string>
# include <vector>
# include <cmath>
# include <time.h>
# include "ProteinMover/PhiPsiMover.h"
# include "ProteinMover/RigidBodySpinMover.h"

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
  RigidBodySpinMoverPtr mover;
  std::vector<double> parameters;

public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);

    core::pose::PoseOP pose = new core::pose::Pose();
    makePoseFromSequence(pose, "AAAA");
    core::pose::PoseOP init = initializeProteinStructure(pose);

    for (int i = 1; i <= 4 ; i++)
    {
      cout << "phi : " << pose->phi(i) << endl;
      cout << "psi : " << pose->psi(i) << endl;
      cout << "omega : " << pose->omega(i) << endl;
    }

    cout << "energy init : " << getConformationScore(pose) << endl;

    //mover = new RigidBodySpinMover(0, 2, 15);
    cout << "residue 1 : " << mover->getIndexResidueOne() << endl;
    cout << "residue 2 : " << mover->getIndexResidueTwo() << endl;
    cout << "amplitude : " << mover->getAmplitude() << endl;

    mover->move(pose);

    for (int i = 1; i <= 4; i++)
    {
      cout << "phi : " << pose->phi(i) << endl;
      cout << "psi : " << pose->psi(i) << endl;
      cout << "omega : " << pose->omega(i) << endl;
    }

    cout << "energy final : " << getConformationScore(pose) << endl;

    //    File fichier = context.getFile(T("data/1A11.xml"));
    //    File fichierout = context.getFile(T("init_lbcpp.pdb"));
    //    ProteinPtr protein = Protein::createFromXml(context, fichier);
    //    core::pose::PoseOP pose = convertProteinToPose(context, protein);
    //
    //    protein->saveToPDBFile(context, fichierout);
    //    File fichieroutRos = context.getFile(T("init_rosetta.pdb"));
    //    core::io::pdb::dump_pdb((*pose), (const char*)fichieroutRos.getFullPathName());
    //
    //    core::pose::PoseOP pose2 = new core::pose::Pose();
    //    File fichieroutRos2 = context.getFile(T("init2_rosetta.pdb"));
    //    core::io::pdb::pose_from_pdb(*pose2, (const char*)fichieroutRos.getFullPathName());
    //    core::io::pdb::dump_pdb((*pose2), (const char*)fichieroutRos2.getFullPathName());
    //    //ProteinPtr prot1 = convertPoseToProtein(context, pose2);
    //    //core::pose::PoseOP pose3 = convertProteinToPose(context, prot1);
    //
    //    cout << "energie cas 1 : " << getTotalEnergy(pose) << endl;
    //    cout << "energie cas 2 : " << getTotalEnergy(pose2) << endl;
    //
    //    for (int i = 1; i <= pose->n_residue(); i++)
    //    {
    //      for (int j = 1; j <= pose->residue(i).natoms(); j++)
    //      {
    //        core::Vector v = pose->residue(i).atom(j).xyz();
    //        core::Vector v2 = pose2->residue(i).atom(j).xyz();
    //        if (((v.x() - v2.x()) != 0) || ((v.y() - v2.y()) != 0) || ((v.z() - v2.z()) != 0))
    //        {
    //          cout << "residu : " << i <<"atome : " << pose->residue(i).atom_name(j) << " x : " << v.x() << " y : "
    //              << v.y() << " z : " << v.z() << endl;
    //          cout << "residu : " << i <<"atome : " << pose2->residue(i).atom_name(j) << " x : " << v2.x() << " y : "
    //                        << v2.y() << " z : " << v2.z() << endl;
    //        }
    //      }
    //    }
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
