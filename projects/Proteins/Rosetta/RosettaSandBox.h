/*-----------------------------------------.---------------------------------.
| Filename: RosettaSandBox.h               | Rosetta Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 15/05/2011 19:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SAND_BOX
# define LBCPP_PROTEINS_ROSETTA_SAND_BOX

# include "../Data/Protein.h"
# include "../Data/AminoAcid.h"
# include "../Data/Residue.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "../Evaluator/QScoreEvaluator.h"
# include "RosettaUtils.h"
# include "ProteinMover.h"
# include "ProteinMover/PhiPsiMover.h"
# include "ProteinMover/ShearMover.h"
# include "ProteinMover/RigidBodyMover.h"
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/ResiduePairSampler.h"
# include "Sampler.h"
using namespace std;

namespace lbcpp
{

class ProteinMoverSampler : public CompositeSampler
{
public:
  ProteinMoverSampler()
  {
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(5), gaussianSampler(0, M_PI), gaussianSampler(0, M_PI)));
    samplers.push_back(objectCompositeSampler(shearMoverClass, new SimpleResidueSampler(5), gaussianSampler(0, M_PI), gaussianSampler(0, M_PI)));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ResiduePairSampler(5), gaussianSampler(1, 1), gaussianSampler(1, 1)));
    probabilities = new DenseDoubleVector(3, 0.33);
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    jassert(probabilities->getNumValues() == samplers.size());
    size_t index = random->sampleWithProbabilities(probabilities->getValues());
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    std::vector< std::vector<Variable> > subDatasets(samplers.size());
    probabilities->clear();
    probabilities->resize(samplers.size());

    double invZ = 1.0 / dataset.size();
    for (size_t i = 0; i < dataset.size(); ++i)
    {
      TypePtr type = dataset[i].getType();
      size_t target;
      if (type == phiPsiMoverClass)
        target = 0;
      else if (type == shearMoverClass)
        target = 1;
      else if (type == rigidBodyMoverClass)
        target = 2;
      else
        jassert(false);
      probabilities->incrementValue(target, invZ);
      subDatasets[target].push_back(dataset[i]);
    }

    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->learn(context, subDatasets[i]);
  }

protected:
  friend class ProteinMoverSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

class RosettaSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    std::vector<Variable> learning;

    // phipsi
    learning.push_back(phiPsiMover(1, 34, -123));
    learning.push_back(phiPsiMover(0, 30, -122));
    learning.push_back(phiPsiMover(2, 27, -121));
    learning.push_back(phiPsiMover(3, 33, -121));
    // shear
    learning.push_back(shearMover(3, 0.9, 4.5));
    learning.push_back(shearMover(4, 0.7, 4.3));
    learning.push_back(shearMover(3, 0.8, 3.4));
    // general
    learning.push_back(rigidBodyMover(3, 5, 2.8, -3.4));
    learning.push_back(rigidBodyMover(3, 5, 2.5, -2.4));
    learning.push_back(rigidBodyMover(1, 3, 0.8, 3.4));
    learning.push_back(rigidBodyMover(0, 4, 1.2, 2.4));
    learning.push_back(rigidBodyMover(2, 4, 0.3, 3.4));
    learning.push_back(rigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(rigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(rigidBodyMover(0, 3, 1.01, 4));
    // spin
    learning.push_back(rigidBodyMover(0, 3, 0.0, 11.3));
    learning.push_back(rigidBodyMover(1, 3, 0.0, 12.4));
    learning.push_back(rigidBodyMover(3, 5, 0.0, 9.3));
    learning.push_back(rigidBodyMover(2, 5, 0.0, 10.2));
    // trans
    learning.push_back(rigidBodyMover(4, 1, 10.2, 0.0));
    learning.push_back(rigidBodyMover(4, 1, 9.2, 0.0));
    learning.push_back(rigidBodyMover(4, 0, 12.1, 0.0));
    learning.push_back(rigidBodyMover(1, 3, -0.3, 0.0));
    learning.push_back(rigidBodyMover(0, 2, -2.1, 0.0));
    learning.push_back(rigidBodyMover(0, 3, -1.3, 0.0));

    SamplerPtr sampler = new ProteinMoverSampler();
    sampler->learn(context, learning);
    context.resultCallback(T("sampler"), sampler);
    return Variable();

  }

private:
  friend class RosettaWorkUnitClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_
