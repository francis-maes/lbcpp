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
# include "ProteinOptimizer/GreedyOptimizer.h"
# include "ProteinOptimizer/SequentialOptimizer.h"
# include "ProteinOptimizer/SimulatedAnnealingOptimizer.h"
# include "ProteinOptimizer/MonteCarloOptimizer.h"
# include <iostream>
# include <fstream>
# include <string>
# include <vector>
# include <cmath>
# include <time.h>
# include "ProteinMover/PhiPsiMover.h"
# include "ProteinMover/RigidBodySpinMover.h"
# include "ProteinMover/ShearMover.h"
# include "ProteinMover/RigidBodyGeneralMover.h"
# include "ProteinMover/RigidBodyTransMover.h"
# include "Sampler.h"
# include "Sampler/GaussianContinuousSampler.h"
# include "Sampler/ParzenContinuousSampler.h"
# include "Sampler/EnumerationDiscreteSampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "Sampler/PhiPsiMoverSampler.h"
# include "Sampler/ShearMoverSampler.h"
# include "Sampler/RigidBodyGeneralMoverSampler.h"
# include "Sampler/RigidBodyTransMoverSampler.h"
# include "Sampler/RigidBodySpinMoverSampler.h"
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/GaussianMultivariateSampler.h"

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

public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);
    RandomGeneratorPtr random = new RandomGenerator(0);

    MatrixPtr m = new DoubleSymmetricMatrix(doubleType, 4, 0.0);
    MatrixPtr probabilities = new DoubleMatrix(2,1,0.0);
    std::vector<MatrixPtr> means(2);
    std::vector<MatrixPtr> covarianceMatrix(2);
    std::vector<MatrixPtr> invCov(2);
    probabilities->setElement(0, 0, Variable(0.9));
    probabilities->setElement(1, 0, Variable(0.3));
    means[0] = new DoubleMatrix(2,1,0.0);
    means[1] = new DoubleMatrix(2,1,0.0);
    means[0]->setElement(0, 0, Variable(-5.0));
    means[0]->setElement(1, 0, Variable(-5.0));
    means[1]->setElement(0, 0, Variable(5.0));
    means[1]->setElement(1, 0, Variable(5.0));

    covarianceMatrix[0] = new DoubleMatrix(2,2,0.0);
    covarianceMatrix[0]->setElement(0, 0, Variable(49.0));
    covarianceMatrix[0]->setElement(0, 1, Variable(40.0));
    covarianceMatrix[0]->setElement(1, 0, Variable(40.0));
    covarianceMatrix[0]->setElement(1, 1, Variable(50.0));
    covarianceMatrix[1] = new DoubleMatrix(2,2,0.0);
    covarianceMatrix[1]->setElement(0, 0, Variable(50.0));
    covarianceMatrix[1]->setElement(0, 1, Variable(0.2));
    covarianceMatrix[1]->setElement(1, 0, Variable(0.1));
    covarianceMatrix[1]->setElement(1, 1, Variable(40.0));
    GaussianMultivariateSamplerPtr p = new GaussianMultivariateSampler(2, 2, probabilities, means,
        covarianceMatrix);

    invCov[0] = p->inverseMatrix(covarianceMatrix[0]);
    invCov[1] = p->inverseMatrix(covarianceMatrix[1]);

    m->setElement(0, 0, Variable(2.0));
    m->setElement(0, 1, Variable(0.5));
    m->setElement(0, 2, Variable(0.5));
    m->setElement(0, 3, Variable(0.6));
    m->setElement(1, 0, Variable(0.5));
    m->setElement(1, 1, Variable(2.0));
    m->setElement(1, 2, Variable(0.3));
    m->setElement(1, 3, Variable(0.4));
    m->setElement(2, 0, Variable(0.5));
    m->setElement(2, 1, Variable(0.3));
    m->setElement(2, 2, Variable(1.5));
    m->setElement(2, 3, Variable(0.2));
    m->setElement(3, 0, Variable(0.6));
    m->setElement(3, 1, Variable(0.4));
    m->setElement(3, 2, Variable(0.2));
    m->setElement(3, 3, Variable(3.0));

    p->printMatrix(m);
    cout << endl;

    MatrixPtr r1 = p->choleskyDecomposition(m);
    p->printMatrix(r1);
    cout << endl;

    MatrixPtr r2 = p->transposeMatrix(r1);
    p->printMatrix(r2);
    cout << endl;

    MatrixPtr r3 = p->matrixProduct(r1, r2);
    p->printMatrix(r3);

    MatrixPtr t2 = new DoubleMatrix(1, 4, 0.0);
    t2->setElement(0, 0, Variable(1.2));
    t2->setElement(0, 1, Variable(2.1));
    t2->setElement(0, 2, Variable(3.1));
    t2->setElement(0, 3, Variable(0.5));
    p->printMatrix(t2);

    MatrixPtr transpose = p->transposeMatrix(t2);
    MatrixPtr r4 = p->matrixProduct(r1, transpose);
    p->printMatrix(r4);

    MatrixPtr t3 = new DoubleMatrix(2, 2, 0.0);
    t3->setElement(0, 0, Variable(3.0));
    t3->setElement(0, 1, Variable(5.0));
    t3->setElement(1, 0, Variable(2.0));
    t3->setElement(1, 1, Variable(-1.0));
    p->printMatrix(t3);

    MatrixPtr t4 = new DoubleMatrix(2, 2, 0.0);
    t4->setElement(0, 0, Variable(-3.0));
    t4->setElement(0, 1, Variable(-5.0));
    t4->setElement(1, 0, Variable(12.0));
    t4->setElement(1, 1, Variable(2.0));
    p->printMatrix(t4);

    MatrixPtr add = p->addMatrix(t3, t4);
    MatrixPtr sub = p->subtractMatrix(t3, t4);
    p->printMatrix(add);
    p->printMatrix(sub);

    MatrixPtr inv = p->inverseMatrix(t3);
    p->printMatrix(inv);

    MatrixPtr verif = p->matrixProduct(inv, t3);
    p->printMatrix(verif);
    p->printMatrix(p->matrixProduct(inv, t3));

    cout << p->normOneMatrix(t3) << endl;

    cout << "=========== test sample =======================" << endl;
    ofstream ofs("/Users/alex/Documents/MATLAB/multi.data");

    for (int i = 0; i < 10000; i++)
    {
      MatrixPtr out = p->sample(context, random).getObjectAndCast<Matrix> ();
      ofs << out->getElement(0, 0).getDouble() << ", " << out->getElement(1, 0) << endl;
    }

    ofs.flush();
    ofs.close();

    std::vector<std::pair<Variable, Variable> > dataset;
    std::vector<MatrixPtr> mm(8);
    mm[0] = new DoubleMatrix(2, 1, 0.0);
    mm[0]->setElement(0, 0, Variable(-13.54634));
    mm[0]->setElement(1, 0, Variable(-13.5464));
    mm[1] = new DoubleMatrix(2, 1, 0.0);
    mm[1]->setElement(0, 0, Variable(-16.2462));
    mm[1]->setElement(1, 0, Variable(-18.14646));
    mm[2] = new DoubleMatrix(2, 1, 0.0);
    mm[2]->setElement(0, 0, Variable(-12.5644));
    mm[2]->setElement(1, 0, Variable(-19.5356));
    mm[3] = new DoubleMatrix(2, 1, 0.0);
    mm[3]->setElement(0, 0, Variable(-15.59));
    mm[3]->setElement(1, 0, Variable(-17.57));
    mm[4] = new DoubleMatrix(2, 1, 0.0);
    mm[4]->setElement(0, 0, Variable(20.57));
    mm[4]->setElement(1, 0, Variable(96.52));
    mm[5] = new DoubleMatrix(2, 1, 0.0);
    mm[5]->setElement(0, 0, Variable(19.95));
    mm[5]->setElement(1, 0, Variable(90.52));
    mm[6] = new DoubleMatrix(2, 1, 0.0);
    mm[6]->setElement(0, 0, Variable(25.52));
    mm[6]->setElement(1, 0, Variable(89.3));
    mm[7] = new DoubleMatrix(2, 1, 0.0);
    mm[7]->setElement(0, 0, Variable(17.9));
    mm[7]->setElement(1, 0, Variable(88.2));

    for (int i = 0; i < mm.size();i++)
      dataset.push_back(std::pair<Variable, Variable>(Variable(mm[i]), Variable(0.0)));

    cout << "=========== learning =======================" << endl;
    p->learn(context, random, dataset);

    cout << "=========== sampling again =======================" << endl;
    ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
    for (double i = 0; i < 10000; i++)
    {
        MatrixPtr out = p->sample(context, random).getObjectAndCast<Matrix> ();
        ofs2 << out->getElement(0, 0).getDouble() << ", " << out->getElement(1, 0) << endl;
    }

    ofs2.flush();
    ofs2.close();

    //    MatrixPtr means0 = new DoubleMatrix(2, 1, 0.0);
    //    MatrixPtr means1 = new DoubleMatrix(2, 1, 0.0);
    //    means0->setElement(0, 0, Variable(-10.0));
    //    means0->setElement(1, 0, Variable(-10.0));
    //    means1->setElement(0, 0, Variable(20.0));
    //    means1->setElement(1, 0, Variable(50.0));
    //
    //    SymmetricMatrixPtr covarianceMatrix0 = new DoubleSymmetricMatrix(doubleType, 2, 0.0);
    //    SymmetricMatrixPtr covarianceMatrix1 = new DoubleSymmetricMatrix(doubleType, 2, 0.0);
    //    covarianceMatrix0->setElement(0, 0, Variable(49.0));
    //    covarianceMatrix0->setElement(0, 1, Variable(40.0));
    //    covarianceMatrix0->setElement(1, 0, Variable(40.0));
    //    covarianceMatrix0->setElement(1, 1, Variable(50.0));
    //
    //    covarianceMatrix1->setElement(0, 0, Variable(5.0));
    //    covarianceMatrix1->setElement(0, 1, Variable(0.2));
    //    covarianceMatrix1->setElement(1, 0, Variable(0.1));
    //    covarianceMatrix1->setElement(1, 1, Variable(4.0));
    //
    //    MatrixPtr inv0 = p->inverseMatrix(covarianceMatrix0);
    //    MatrixPtr inv1 = p->inverseMatrix(covarianceMatrix1);
    //

//    ofstream ofs3("/Users/alex/Documents/MATLAB/multiProb.data");
//
//    for (double i = -50; i <= 60; i++)
//    {
//      for (double j = -50; j <= 60; j++)
//      {
//        MatrixPtr obs = new DoubleMatrix(2, 1, 0.0);
//        obs->setElement(0, 0, Variable(i));
//        obs->setElement(1, 0, Variable(j));
//        double val = probabilities->getElement(0, 0).getDouble() * p->computeProbability(obs,
//            means[0], covarianceMatrix[0], invCov[0]);
//        val += probabilities->getElement(1, 0).getDouble() * p->computeProbability(obs, means[1],
//            covarianceMatrix[1], invCov[1]);
//        ofs3 << val << " ";
//      }
//      ofs3 << endl;
//    }
//
//    ofs3.flush();
//    ofs3.close();

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_
