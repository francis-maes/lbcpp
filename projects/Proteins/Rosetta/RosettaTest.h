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
		//makePoseFromSequence(
		//		pose,
		//		String(
		//				"MERLLDDVCVMVDVTSAEIRNIVTRQLENWGATCITPDERLISQDYDIFLTDNPSNLTASGLLLSDDESGVREIGPGQLCVNFNMSNAMQEAVLQLIEVQLAQEEVTESRSHHHHHH"));
		makePoseFromSequence(pose, String("AAAAAAAAAAAA"));
		core::io::pdb::dump_pdb((*pose), "/Users/alex/Desktop/init.pdb");
		cout << "energy init : " << getTotalEnergy(pose) << endl;

		std::vector<void*>* optArgs = new std::vector<void*>();
		double ang = 40;
		optArgs->push_back((void*) &ang);

		cout << "test compil argument : " << arg << endl;
		// TODO Q SCore
		/*double TertiaryStructure::computeCAlphaAtomsQScore(TertiaryStructurePtr targetStructure, int cutoff) const
		 {
		 // TODO
		 return 0.0;
		 }
		 */
		/*
		 struct CAlphaDist {
		 int i;
		 int j;
		 double dist;
		 };
		 typedef struct CAlphaDist CAlphaDist;
		 */
		/*
		 class QScoreObject
		 {
		 private:
		 vector<CAlphaDist> *scores;
		 double mean;
		 int size;
		 public:
		 */
		/*QScoreObject(int s) {
		 size = 0;
		 scores = NULL;
		 };
		 ~QScoreObject() {
		 if (scores != NULL)
		 delete(scores);
		 };
		 */
		/*double setScores(list<CAlphaDist>* l) {
		 double acc = 0;
		 l->sort(compare_CAlphaDist);
		 size = l->size();
		 scores = new vector<CAlphaDist>(size);
		 for (int i = 0; i < size; i++)
		 {
		 (*scores)[i] = l->front();
		 l->pop_front();
		 acc += (*scores)[i].dist;
		 }
		 mean = acc / (double)size;
		 return mean;
		 };
		 double getScore(int i) {
		 if (size == 0)
		 {
		 return -1.0;
		 }
		 else
		 {
		 if (i < 0)
		 i = 0;
		 if (i >= size - 1)
		 i = size - 1;
		 return (*scores)[i];
		 }
		 };
		 int getSize() {
		 return size;
		 };
		 double getMean() {
		 return mean;
		 };
		 */
		//};

		/*
		 bool compare_CAlphaDist(CAlphaDist calpha1, CAlphaDist calpha2) {
		 return (calpha1.dist > calpha2.dist);
		 };
		 */

		//core::pose::PoseOP init = new core::pose::Pose("/Users/alex/Desktop/2KX7.pdb");
		//cout << "2KX7 numres : " << init->n_residue() << endl;
		//cout << "energy 2KX7 : " << getTotalEnergy(init) << endl;

		core::pose::PoseOP result3 = simulatedAnnealingOptimization(pose, phiPsiMover, optArgs);
		core::io::pdb::dump_pdb((*result3), "/Users/alex/Desktop/sa_aprescompil.pdb");
		cout << "energy sa : " << getTotalEnergy(result3) << endl;
		cout << "sa numres : " << result3->n_residue() << endl;
		cout << "sa done." << endl;
		/*
		 core::pose::PoseOP result1 = sequentialOptimization(pose, phiPsiMover, optArgs);
		 core::io::pdb::dump_pdb((*result1), "/Users/alex/Desktop/sequential.pdb");
		 cout << "energy sequential : " << getTotalEnergy(result1) << endl;
		 cout << "sequential numres : " << result1->n_residue() << endl;
		 cout << "sequential done." << endl;
		 */
		context.informationCallback(T("RosettaTest done."));
		return Variable();

	}
};

}
; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_
