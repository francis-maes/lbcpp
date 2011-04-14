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
		//makePoseFromSequence(
		//		pose,
		//		String(
		//				"MERLLDDVCVMVDVTSAEIRNIVTRQLENWGATCITPDERLISQDYDIFLTDNPSNLTASGLLLSDDESGVREIGPGQLCVNFNMSNAMQEAVLQLIEVQLAQEEVTESRSHHHHHH"));
		makePoseFromSequence(pose, String("AAA"));
		core::io::pdb::dump_pdb((*pose), "/Users/alex/Desktop/init.pdb");
		cout << "energy init : " << getTotalEnergy(pose) << endl;
		cout << "num residus : " << pose->n_residue() << endl;

		/*
		 // TEST
		 // Creation string a partir de rosetta
		 std::ostringstream oss;
		 core::io::pdb::FileData::dump_pdb((*pose), oss);
		 oss.flush();
		 std::string poseString = oss.str();
		 cout << poseString << endl;

		 // Lecture string a partir de lbcpp
		 juce::String js(poseString.c_str());
		 const char* tab0 = js.toUTF8();
		 juce::MemoryInputStream mb((const void*)tab0,js.length(),true);
		 juce::String testStringJuce = (juce::String)mb.readString();
		 cout << "test juce string apres memoryinputstream: " << (const char*)testStringJuce << endl;

		 // FIN TEST
		 */
		juce::String path("/Users/alex/Desktop/init.pdb");
		ProteinPtr prottest = Protein::createFromPDB(context, juce::File(path), true);
		cout << "nombre acides amines dans prottest : " << prottest->getLength() << endl;

		cout << "test convertPose2protein " << endl;
		ProteinPtr prot = convertPose2Protein(context, pose);
		cout << "test convertPose2protein : OK." << endl;
		TertiaryStructurePtr ts = prot->getTertiaryStructure();
		cout << "nombre acides amines dans prot : " << prot->getLength() << endl;
		CartesianPositionVectorPtr calphatrace = ts->makeCAlphaTrace();
		impl::Vector3 v0 = calphatrace->get(0);
		impl::Vector3 v1 = calphatrace->get(1);
		impl::Vector3 v2 = calphatrace->get(2);
		cout << "v0 : " << v0.x << " , " << v0.y << " , " << v0.z << endl;
		cout << "v1 : " << v1.x << " , " << v1.y << " , " << v1.z << endl;
		cout << "v2 : " << v2.x << " , " << v2.y << " , " << v2.z << endl;

		ResiduePtr r0 = ts->getResidue(0);
		cout << "CA dans residu 0 ? " << r0->hasCAlphaAtom() << endl;
		ResiduePtr r1 = ts->getResidue(1);
		cout << "CA dans residu 1 ? " << r1->hasCAlphaAtom() << endl;
		ResiduePtr r2 = ts->getResidue(2);
		cout << "CA dans residu 2 ? " << r2->hasCAlphaAtom() << endl;

		// Symmetric matrix
		SymmetricMatrixPtr mat = (prot->getTertiaryStructure())->makeCAlphaDistanceMatrix();
		cout << "matrice nombre de colonnes : " << mat->getNumColumns() << endl;
		cout << "matrice nombre de lignes : " << mat->getNumRows() << endl;
		cout << "matrice element 0,0 : " << mat->getElement(0, 0) << endl;
		cout << "matrice element 0,1 : " << mat->getElement(0, 1) << endl;
		cout << "matrice element 1,0 : " << mat->getElement(1, 0) << endl;
		cout << "matrice element 0,2 : " << mat->getElement(0, 2) << endl;
		cout << "matrice element 2,0 : " << mat->getElement(2, 0) << endl;
		cout << "matrice element 1,2 : " << mat->getElement(1, 2) << endl;
		cout << "matrice element 2,1 : " << mat->getElement(2, 1) << endl;
		//cout << "matrice element 0,3 : " << mat->getElement(0,3) << endl;
		//cout << "matrice element 3,0 : " << mat->getElement(3,0) << endl;

		// AUTRE
		//std::vector<void*>* optArgs = new std::vector<void*>();
		//double ang = 40;
		//optArgs->push_back((void*) &ang);

		//core::pose::PoseOP init = new core::pose::Pose("/Users/alex/Desktop/2KX7.pdb");
		//cout << "2KX7 numres : " << init->n_residue() << endl;
		//cout << "energy 2KX7 : " << getTotalEnergy(init) << endl;
		/*
		 core::pose::PoseOP result3 = simulatedAnnealingOptimization(pose, phiPsiMover, optArgs);
		 core::io::pdb::dump_pdb((*result3), "/Users/alex/Desktop/sa_aprescompil.pdb");
		 cout << "energy sa : " << getTotalEnergy(result3) << endl;
		 cout << "sa numres : " << result3->n_residue() << endl;
		 cout << "sa done." << endl;

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
