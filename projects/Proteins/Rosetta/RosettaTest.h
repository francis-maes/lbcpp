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
		makePoseFromSequence(pose, String("AAAAAAAAAAAA"));
		core::io::pdb::dump_pdb((*pose), "/Users/alex/Desktop/init.pdb");
		cout << "energy init : " << getTotalEnergy(pose) << endl;
		cout << "num residus : " << pose->n_residue() << endl;

		juce::String* str = new juce::String[2];
		str[0]=juce::String("AAAAAAA");
		str[1] =juce::String((double)0.01);

				/*
		 std::vector<Variable*>* vs =new std::vector<Variable*>;
		 cout << "test 1 " << endl;
		 //Variable temp1 = new Variable(opt->getVariable(0));
		 vs->push_back(tempv);
		 cout << "test 2 " << endl;
		 cout << " var 0 : " << (const char*) ((vs->at(0))->toString()) << endl << flush;
		 cout << "test 3 " << endl << flush;
		 Variable* temp2 = new Variable(0.1);
		 //opt->addVariable(temp2);
		 cout << "test 4 " << endl << flush;
		 vs->push_back(temp2);
		 cout << " var 1 : " << (vs->at(1))->getDouble() << endl << flush;
		 cout << "size : " << vs->size() << endl << flush;

		 delete(tempv);
		 delete(temp2);
		 delete(vs);
		 */

		/*
		juce::String scopeName("energy");
		 context.enterScope(T("Protein name : AAAAAAAA"));
		 for (int i=0;i<10;i++)
		 {
		 context.enterScope(scopeName);
		 context.resultCallback(T("energies"),Variable(i));
		 context.leaveScope(Variable(i*i));
		 }
		 context.leaveScope();
*/


		 std::vector<void*>* optArgs = new std::vector<void*>();
		 double ang = 50;
		 optArgs->push_back((void*) &ang);
		 core::pose::PoseOP result1 = greedyOptimization(pose, phiPsiMover, optArgs, 1000, str, &context);
		 cout << "energy final : " << getTotalEnergy(result1) << endl;
		 cout << "num residus final : " << result1->n_residue() << endl;
		 //cout << "energy greedy1 : " << getTotalEnergy(result1) << endl;
		 //cout << "greedy done." << endl;



		/*std::vector<void*>* optArgs = new std::vector<void*>();
		 double ang = 50;
		 optArgs->push_back((void*) &ang);
		 core::pose::PoseOP result1 = monteCarloOptimization(pose, phiPsiMover, optArgs, 2.0, 1000);
		 core::io::pdb::dump_pdb((*result1), "/Users/alex/Desktop/result_greedy1.pdb");
		 cout << "energy greedy1 : " << getTotalEnergy(result1) << endl;
		 cout << "greedy done." << endl;

		 core::pose::PoseOP result2 = monteCarloOptimization(pose, phiPsiMover, optArgs, 2.0, 20000);
		 core::io::pdb::dump_pdb((*result2), "/Users/alex/Desktop/result_greedy2.pdb");
		 cout << "energy greedy2 : " << getTotalEnergy(result2) << endl;
		 cout << "greedy done." << endl;
		 */
		context.informationCallback(T("RosettaTest done."));
		return Variable();

	}
};

}
; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_
