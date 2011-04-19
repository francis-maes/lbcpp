/*-----------------------------------------.---------------------------------.
 | Filename: RosettaTest.h                  | Rosetta Test                    |
 | Author  : Francis Maes                   |                                 |
 | Started : 30/03/2011 13:40               |                                 |
 `------------------------------------------/                                 |
 |                                             |
 `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_TEST_H_
#define LBCPP_PROTEINS_ROSETTA_TEST_H_

#include "../Data/Protein.h"
#include "../Data/AminoAcid.h"
#include "../Data/Residue.h"
#include "../Data/Formats/PDBFileGenerator.h"
#include "../Evaluator/QScoreEvaluator.h"
#include "RosettaUtils.h"
#include "RosettaMover.h"
#include "RosettaOptimizer.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <time.h>

#undef T
#include <core/pose/Pose.hh>
#include <core/chemical/util.hh>
#include <core/io/pdb/pose_io.hh>
#include <core/io/pdb/file_data.hh>
#include <core/kinematics/FoldTree.hh>
#include <protocols/init.hh>
#define T JUCE_T

using namespace std;

namespace lbcpp
{

class RosettaTest: public WorkUnit
{
protected:

public:
	size_t arg;
	virtual Variable run(ExecutionContext& context)
	{

		rosettaInit(false);
		core::pose::PoseOP pose = new core::pose::Pose();
		makePoseFromSequence(pose, String("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
		core::io::pdb::dump_pdb((*pose), "/Users/alex/Desktop/init.pdb");
		cout << "energy init : " << getTotalEnergy(pose) << endl;
		cout << "num residus : " << pose->n_residue() << endl;

		std::vector<Variable>* optArgs = new std::vector<Variable>;
		optArgs->push_back(Variable((double) 25));

		int maxit = 10000;

		time_t time1;
		time_t time2;
		struct tm * timeinfo;

		RosettaOptimizerPtr o = new RosettaOptimizer(&context, T("AAAAA"), 0.01);
		//RosettaOptimizerPtr o = new RosettaOptimizer();

		time(&time1);
		PhiPsiRandomMoverPtr pprm = new PhiPsiRandomMover(optArgs);
		core::pose::PoseOP result1 = o->greedyOptimization(pose, pprm, maxit);
		cout << "energy final : " << getTotalEnergy(result1) << endl;
		time(&time2);
		cout << "time : " << difftime(time2, time1) << endl;

		time(&time1);
		PhiPsiGaussRandomMoverPtr pprgm = new PhiPsiGaussRandomMover(optArgs);
		core::pose::PoseOP result2 = o->greedyOptimization(pose, pprgm, maxit);
		cout << "phipsi gauss energy final : " << getTotalEnergy(result2) << endl;
		time(&time2);
		cout << "time : " << difftime(time2, time1) << endl;

		optArgs->at(0) = Variable((double) 1.0);

		time(&time1);
		RigidBodyTransRandomMoverPtr rgtrm = new RigidBodyTransRandomMover(optArgs);
		core::pose::PoseOP result3 = o->greedyOptimization(pose, rgtrm, maxit);
		cout << "RBT energy final : " << getTotalEnergy(result3) << endl;
		time(&time2);
		cout << "time : " << difftime(time2, time1) << endl;

		optArgs->push_back(Variable((double) 5.0));
		time(&time1);
		RigidBodyPerturbRandomMoverPtr rbprm = new RigidBodyPerturbRandomMover(optArgs);
		core::pose::PoseOP result5 = o->greedyOptimization(pose, rbprm, maxit);
		cout << "RBP energy final : " << getTotalEnergy(result5) << endl;
		time(&time2);
		cout << "time : " << difftime(time2, time1) << endl;

		delete (optArgs);

		std::vector<Variable>* optArgs2 = new std::vector<Variable>;
		optArgs2->push_back(Variable((double) 10));
		optArgs2->push_back(Variable((double) 25));
		optArgs2->push_back(Variable((int) 1));

		time(&time1);
		ShearRandomMoverPtr srm = new ShearRandomMover(optArgs2);
		core::pose::PoseOP result4 = o->greedyOptimization(pose, srm, maxit);
		cout << "Shear energy final : " << getTotalEnergy(result4) << endl;
		time(&time2);
		cout << "time : " << difftime(time2, time1) << endl;

		delete (optArgs2);

		context.informationCallback(T("RosettaTest done."));
		return Variable();

	}
};

}
; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_
