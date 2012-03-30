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
# include "Data/Mover/PoseMover.h"
# include "ProteinOptimizer/ProteinOptimizer.h"
# include "ProteinOptimizer/SequentialOptimizer.h"
# include "ProteinOptimizer/SimulatedAnnealingOptimizer.h"
# include <iostream>
# include <fstream>
# include <string>
# include <vector>
# include <cmath>
# include <time.h>
# include "RosettaSandBox.h"
# include "RosettaProtein.h"
# include "WorkUnit/DistributableWorkUnit.h"
# include "Data/MoverSampler/BlindPoseMoverSampler.h"
# include "Data/MoverSampler/ConditionalPoseMoverSampler.h"

# include "Data/Features/PoseFeatureGenerator.h"
# include "Data/Features/BlindPoseFeatureGenerator.h"

# include "ProteinOptimizer/SimulatedAnnealing.h"

# include "Data/Rosetta.h"
# include "Data/Pose.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/pose/Pose.hh>
#  include <core/chemical/util.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/kinematics/FoldTree.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{
extern EnumerationPtr featuresMoverEnumerationEnumeration;

void generateMoversDataSet(VectorPtr& inputs, VectorPtr& samples);

class RosettaTest : public WorkUnit
{
private:
  friend class RosettaTestClass;
  size_t arg;
  double value;
  String proteinsDir;

public:
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

# ifdef LBCPP_PROTEIN_ROSETTA

    File referencesFile = context.getFile(T("data/psipred_test"));

    juce::OwnedArray<File> references;
    referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));

//    size_t num100 = 0;
//    size_t num200 = 0;
//    size_t numplus = 0;
//    size_t maxSize = 0;
//    size_t mean = 0;
//    for (size_t i = 0; i < references.size(); i++)
//    {
//      ProteinPtr protein = Protein::createFromPDB(context, *references[i]);
//
//      if (protein->getLength() <= 100)
//        ++num100;
//      else if ((protein->getLength() > 100) && (protein->getLength() <= 200))
//        ++num200;
//      else
//      {
//        ++numplus;
//        mean += protein->getLength();
//        if (protein->getLength() > maxSize)
//          maxSize = protein->getLength();
//      }
//    }
//
//    std::cout << "num -100 : " << num100 << std::endl;
//    std::cout << "num +100 -200 : " << num200 << std::endl;
//    std::cout << "num plus : " << numplus << std::endl;
//    std::cout << "maxSize : " << maxSize << std::endl;
//    std::cout << "mean size plus : " << (double)mean / (double)numplus << std::endl;

    for (size_t i = 0; i < references.size(); i++)
    {
      ProteinPtr protein = Protein::createFromPDB(context, *references[i]);

      if (protein->getLength() > 200)
        (*references[i]).deleteFile();
    }

    //    jassert(false);
    //
    //    RosettaPtr ros = new Rosetta();
    //    ros->init(context, false, 0, 0);
    //
    //    // without learning
    //    PosePtr pose = new Pose(T("AAAAAAAAAAACDEDCDEDC"));
    //    pose->initializeToHelix();
    //    PoseOptimizationStatePtr optState = new PoseOptimizationState(pose);
    //    SamplerPtr sampler = new BlindPoseMoverSampler(20);
    //    FeatureGeneratorPtr blindFeatures = blindPoseFeatureGenerator();
    //    PoseOptimizationStateModifierPtr modifier = new PoseOptimizationStateModifier(sampler, blindFeatures);
    //
    //    SimulatedAnnealingParametersPtr params = new SimulatedAnnealingParameters(200, 4, 0.01, 20);
    //    SimulatedAnnealingPtr sa = new SimulatedAnnealing(optState, modifier, GeneralOptimizerStoppingCriterionPtr(), params);
    //
    //    VariableVectorPtr results1 = sa->optimize(context);
    //
    //    // with learning
    //    pose = new Pose(T("AAAAAAAAAAACDEDCDEDC"));
    //    pose->initializeToHelix();
    //
    //    // features
    //    PoseFeatureGeneratorPtr feats = new PoseFeatureGenerator();
    //    feats->initialize(context, poseClass);
    //    DoubleVectorPtr tmpFeatures = feats->compute(context, pose).getObjectAndCast<DoubleVector>();
    //
    //    optState = new PoseOptimizationState(pose);
    //    sampler = new ConditionalPoseMoverSampler(20);
    //
    //    // learn the distribution
    //    File referencesFile = context.getFile(T("phipsipetitPDB"));
    //    File moversFile = context.getFile(T("phipsipetit_movers"));
    //
    //    VectorPtr inputWorkers = vector(doubleVectorClass(tmpFeatures->getElementsEnumeration(), doubleType));
    //    VectorPtr inputMovers = vector(doubleVectorClass(tmpFeatures->getElementsEnumeration(), doubleType));
    //    context.enterScope(T("Loading learning examples..."));
    //    juce::OwnedArray<File> references;
    //    referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));
    //
    //    std::vector<size_t> res;
    //    RandomGeneratorPtr rand = new RandomGenerator();
    //    rand->sampleOrder(references.size(), res);
    //
    //    for (size_t i = 0; (i < references.size()) && (i < 10); i++)
    //    {
    //      size_t index = res[i];
    //      juce::OwnedArray<File> movers;
    //      String nameToSearch = (*references[index]).getFileNameWithoutExtension();
    //
    //      PosePtr protein = new Pose(*references[index]);
    //
    //      nameToSearch += T("_mover.xml");
    //      moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
    //      if (movers.size() > 0)
    //      {
    //        context.informationCallback(T("Structure : ") + nameToSearch);
    //        DoubleVectorPtr inFeatures = feats->compute(context, protein).getObjectAndCast<DoubleVector> ();
    ////        std::cout << (const char*)protein->getAminoAcidSequence() << std::endl;
    ////        std::cout << (const char*)protein->getAminoAcidHistogram()->toString() << std::endl;
    ////        std::cout << (const char*)inFeatures->toString() << std::endl << std::endl;
    //        PoseMoverPtr inMover = Variable::createFromFile(context, (*movers[0])).getObjectAndCast<PoseMover> ();
    //        inputWorkers->append(inFeatures);
    //        inputMovers->append(inMover);
    //        context.progressCallback(new ProgressionState((size_t)(i + 1), (size_t)juce::jmin((int)10000, (int)references.size()), T("Intermediate conformations")));
    //      }
    //    }
    //    context.leaveScope();
    //
    //    ContainerPtr addWorkers = inputWorkers;
    //    ContainerPtr addMovers = inputMovers;
    //    sampler->learn(context, addWorkers, addMovers, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(), DenseDoubleVectorPtr());
    //
    //    modifier = new PoseOptimizationStateModifier(sampler, feats);
    //
    //    sa = new SimulatedAnnealing(optState, modifier, GeneralOptimizerStoppingCriterionPtr(), params);
    //
    //    VariableVectorPtr results2 = sa->optimize(context);

    //    DoubleVectorPtr v1 = new DenseDoubleVector(2, 0.0);
    //    DenseDoubleVectorPtr v2 = new DenseDoubleVector(2, 1.0);
    //
    //    std::cout << v1->l2norm(v2) << std::endl;
    //    std::cout << v2->l2norm(v1) << std::endl;
    //
    //    v1 = new DenseDoubleVector(5, 0.0);
    //    v2 = new DenseDoubleVector(5, 1.0);
    //
    //    std::cout << v1->l2norm(v2) << std::endl;
    //    std::cout << v2->l2norm(v1) << std::endl;
    //
    //    v1 = new DenseDoubleVector(9, 0.0);
    //    v2 = new DenseDoubleVector(9, 1.0);
    //
    //    std::cout << v1->l2norm(v2) << std::endl;
    //    std::cout << v2->l2norm(v1) << std::endl;
    //
    //    v1 = new DenseDoubleVector(16, 0.0);
    //    v2 = new DenseDoubleVector(16, 1.0);
    //
    //    std::cout << v1->l2norm(v2) << std::endl;
    //    std::cout << v2->l2norm(v1) << std::endl;
    //
    //    v1 = new DenseDoubleVector(25, 0.0);
    //    v2 = new DenseDoubleVector(25, 2.0);
    //
    //    std::cout << v1->l2norm(v2) << std::endl;
    //    std::cout << v2->l2norm(v1) << std::endl;
    //
    //    v1 = new DenseDoubleVector(100, 0.0);
    //    v2 = new DenseDoubleVector(100, 1.0);
    //
    //    std::cout << v1->l2norm(v2) << std::endl;
    //    std::cout << v2->l2norm(v1) << std::endl;

    //    RosettaPtr ros = new Rosetta();
    //    ros->init(context, false, 0, 0);
    //
    //    PosePtr pose = new Pose(T("AAAAAAAAAAAA"));
    //    std::cout << "energy : " << pose->getEnergy() << std::endl;
    //
    //    GeneralFeaturesPtr feat = new SimplePoseFeatures();
    //    feat->initialize(context, pose);
    //
    //    pose->setFeatureGenerator(context, feat);
    //
    //    Variable features1 = pose->getFeatures(context);
    //    Variable features2 = feat->computeFeatures(context, pose);
    //
    //    std::cout << "features 1 = " << (const char*)features1.getObjectAndCast<DoubleVector>()->toString() << std::endl;
    //    std::cout << "features 2 = " << (const char*)features2.getObjectAndCast<DoubleVector>()->toString() << std::endl;
    //    std::cout << "blabla" << std::endl;

    //    FeatureGeneratorPtr fg = softDiscretizedNumberFeatureGenerator(0.0, 1000, 100, true);
    //
    //    fg->initialize(context, doubleType);
    //
    //    Variable inputTest((double)-19.43);
    //    Variable result = fg->computeFunction(context, &inputTest);
    //    std::cout << "input " << inputTest.toString() << std::endl;
    //    std::cout << "output " << result.toString() << std::endl;
    //
    //    inputTest = Variable((double)0.0);
    //    result = fg->computeFunction(context, &inputTest);
    //    std::cout << "input " << inputTest.toString() << std::endl;
    //    std::cout << "output " << result.toString() << std::endl;
    //
    //    inputTest = Variable((double)10.0);
    //    result = fg->computeFunction(context, &inputTest);
    //    std::cout << "input " << inputTest.toString() << std::endl;
    //    std::cout << "output " << result.toString() << std::endl;
    //
    //    inputTest = Variable((double)20);
    //    result = fg->computeFunction(context, &inputTest);
    //    std::cout << "input " << inputTest.toString() << std::endl;
    //    std::cout << "output " << result.toString() << std::endl;
    //
    //    inputTest = Variable((double)1001);
    //    result = fg->computeFunction(context, &inputTest);
    //    std::cout << "input " << inputTest.toString() << std::endl;
    //    std::cout << "output " << result.toString() << std::endl;

    //    Rosetta ros;
    //    ros.init(context, false);
    //
    //    File referencesFile = context.getFile(T("data/psipred"));
    //    File outFile = context.getFile(T("data/psipredros"));
    //
    //    context.enterScope(T("Testing proteins..."));
//    juce::OwnedArray<File> references;
//    referencesFile.findChildFiles(references, File::findFiles, false, T("*.xml"));
//
//    for (size_t i = 0; i < references.size(); i++)
//    {
//      String nameToSearch = (*references[i]).getFileNameWithoutExtension();
//      std::cout << (const char*)nameToSearch << std::endl;
//
//      ProteinPtr protein = Protein::createFromXml(context, (*references[i]));
//      (*references[i]).deleteFile();
//
//      core::pose::PoseOP pose;
//      convertProteinToPose(context, protein, pose);
//
//      if (pose() == NULL)
//        continue;
//
//      protein->saveToPDBFile(context, File(outFile.getFullPathName() + T("/") + nameToSearch + T(".pdb")));
//
//      context.progressCallback(new ProgressionState(i, references.size(), T("Intermediate conformations")));
//    }
//    context.leaveScope();

//    Rosetta ros;
//
//    ros.init(context, false);
//
//    File referencesFile = context.getFile(T("phipsipetitPDB"));
//    File moversFile = context.getFile(T("phipsipetit_movers"));
//
//    VariableVectorPtr inputWorkers = new VariableVector(0);
//    VariableVectorPtr inputMovers = new VariableVector(0);
//    size_t learningPolicy = 3;
//
//    context.enterScope(T("Loading learning examples..."));
//    juce::OwnedArray<File> references;
//    referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));
//
//    std::vector<size_t> res;
//    RandomGeneratorPtr rand = new RandomGenerator();
//    rand->sampleOrder(references.size(), res);
//
//    size_t j = 0;
//
//    for (size_t i = 0; (i < references.size()) && (i < 10000); i++)
//    {
//      size_t index = res[i];
//      juce::OwnedArray<File> movers;
//      String nameToSearch = (*references[index]).getFileNameWithoutExtension();
//
//      ProteinPtr protein = Protein::createFromPDB(context, (*references[index]));
//      core::pose::PoseOP pose;
//      convertProteinToPose(context, protein, pose);
//
//      if (pose() == NULL)
//        continue;
//
//      nameToSearch += T("_mover.xml");
//      moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
//      if (movers.size() > 0)
//      {
//        context.informationCallback(T("Structure : ") + nameToSearch);
//        RosettaProteinPtr inWorker = new RosettaProtein(pose, 1, 1, 1, 1);
//        PoseMoverPtr inMover = Variable::createFromFile(context, (*movers[0])).getObjectAndCast<PoseMover> ();
//        inputWorkers->append(inWorker);
//        inputMovers->append(inMover);
//        context.progressCallback(new ProgressionState((size_t)(++j), (size_t)juce::jmin((int)10000,
//            (int)references.size()), T("Intermediate conformations")));
//      }
//      else
//        (*references[index]).deleteFile();
//    }
//    context.leaveScope();
//
//    core::pose::PoseOP currentPose = new core::pose::Pose();
//    makePoseFromSequence(currentPose, T("AAAAAA"));
//    core::pose::PoseOP initialPose = new core::pose::Pose();
//    initializeProteinStructure(currentPose, initialPose);
//
//    RosettaWorkerPtr worker = new RosettaWorker(initialPose, learningPolicy);
//    ContainerPtr addWorkers = inputWorkers;
//    ContainerPtr addMovers = inputMovers;
//    worker->learn(context, addWorkers, addMovers);
//
//    Variable currFeatures  = worker->getFeatures(context);
//    std::cout << "currFeatures " << currFeatures.toString() << std::endl;
//    std::cout << "currFeatures name " << (const char*)currFeatures.getTypeName() << std::endl;
//    DenseDoubleVectorPtr feat = currFeatures.getObjectAndCast<DenseDoubleVector>();
//    std::cout << "feat " << feat->toString() << std::endl;

//    context.resultCallback(T("Features"), currFeatures);

//    PoseMoverPtr mover = new PhiPsiMover(2, 23, -43);
//    double probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new PhiPsiMover(3, 23, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new PhiPsiMover(10, 23, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new ShearMover(2, 23, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new ShearMover(12, 23, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new RigidBodyMover(2, 12, 0.5, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new RigidBodyMover(2, 5, 0.5, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new PhiPsiMover(6, -10, 50);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    currentPose = new core::pose::Pose();
//    makePoseFromSequence(currentPose, T("RRRRRAAAAAAAAAAAAAAAA"));
//    initialPose = new core::pose::Pose();
//    initializeProteinStructure(currentPose, initialPose);
//
//    worker = new RosettaWorker(initialPose, learningPolicy);
//    worker->learn(context, addWorkers, addMovers);
//
//    mover = new PhiPsiMover(2, 23, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;
//
//    mover = new PhiPsiMover(3, 23, -43);
//    probs = worker->getMoverProbability(context, mover);
//    std::cout << mover->toString() << " : " << probs << std::endl;


# if 0
    File referenceFile = context.getFile(T("GoodDataset/0-100/dataset0-100"));
    File targetFile = context.getFile(T("GoodDataset/0-100/phipsi"));

    juce::OwnedArray<File> references;
    referenceFile.findChildFiles(references, File::findFiles, false, T("*.xml"));
    ScalarVariableMeanAndVariancePtr qsMeans = new ScalarVariableMeanAndVariance();

    for (size_t j = 0; j < references.size(); j++)
    {
      ProteinPtr proteinRef = Protein::createFromXml(context, *references[j]);
      core::pose::PoseOP referencePose;
      convertProteinToPose(context, proteinRef, referencePose);

      std::cout << "=============================================" << std::endl;
      std::cout << "======== protein : "
          << (const char*)(*references[j]).getFileNameWithoutExtension() << " ==============="
          << std::endl;

      juce::OwnedArray<File> targets;
      targetFile.findChildFiles(targets, File::findFiles, false,
          (*references[j]).getFileNameWithoutExtension() + T("*.xml"));

      RandomGeneratorPtr random = new RandomGenerator();
      bool cont = true;
      for (int i = 0; cont && (i < targets.size()); i++)
      {
        ProteinPtr proteinTarget = Protein::createFromXml(context, *targets[i]);
        core::pose::PoseOP targetPose;
        convertProteinToPose(context, proteinTarget, targetPose);

        std::cout << "---------- structure : "
            << (const char*)(*targets[i]).getFileNameWithoutExtension() << " ---------"
            << std::endl;
        int minDist = juce::jlimit(1, (int)targetPose->n_residue(), juce::jmin(20,
            targetPose->n_residue() / 2));
        int maxDist = -1;
        // pose Qscore
        QScoreObjectPtr scoresPose = QScoreSingleEvaluator(targetPose, referencePose, minDist,
            maxDist);
        double QScorePose = scoresPose->getMean();
        std::cout << "tertiary structure score pose : " << QScorePose << std::endl;
        qsMeans->push(QScorePose);
      }
    }

    std::cout << "mean of everything : " << qsMeans->getMean() << std::endl;
    std::cout << "std of everything : " << qsMeans->getStandardDeviation() << std::endl;

    //    core::pose::PoseOP pose = new core::pose::Pose("/Users/alex/Documents/Ulg/2M/tfe/structs/final_struct_lo_100mil.pdb");
    //
    //    for (size_t i = 1; i < pose->n_residue(); i++)
    //      std::cout << "i : " << i << " phi : " << pose->phi(i) << ", psi : " << pose->psi(i) << std::endl;
    //
    //    ShearMoverPtr shear = shearMover(3, 10, -10);
    //    shear->move(pose);
    //    core::io::pdb::dump_pdb(*pose, "/Users/alex/Desktop/shear.pdb");
    //
    //    for (size_t i = 1; i < pose->n_residue(); i++)
    //      std::cout << "i : " << i << " phi : " << pose->phi(i) << ", psi : " << pose->psi(i) << std::endl;

    // -------------- rosetta protein features
    //    core::pose::PoseOP pose = new core::pose::Pose();
    //    core::pose::PoseOP initialized;
    //    FunctionPtr features = new RosettaProteinFeatures(0, 0, 0, 1);
    //    features->initialize(context, rosettaProteinClass);
    //    String acc;
    //
    //    VectorPtr inputWorker = new VariableVector(0);
    //    VectorPtr inputMover = vector(poseMoverClass, 0);
    //
    //    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    //    VectorPtr inputTest = vector(inputClass, 0);
    //    VectorPtr samples = vector(poseMoverClass);
    //
    //    generateMoversDataset(inputWorker, inputMover);

    //    for (size_t i = 0; i < inputMover->getNumElements(); i++)
    //      cout << inputWorker->getElement(i).getObjectAndCast<RosettaWorker>()->getFeatures(context).getObjectAndCast<DoubleVector>()->toString() << " : " << (const char*)inputMover->getElement(i).getObjectAndCast<PoseMover>()->toString() << endl;

    //SamplerPtr maxent = maximumEntropySampler(poseMoverEnumerationEnumeration);

    ObjectVectorPtr featureVectors = new ObjectVector(doubleVectorClass(), 0);

    //    for (size_t i = 0; i < inputWorker->getNumElements(); i++)
    //    {
    //      RosettaWorkerPtr klWorker = inputWorker->getElement(i).getObjectAndCast<RosettaWorker>();
    //      Variable klFeature = klWorker->getFeatures(context);
    //      //featureVectors->append(klFeature);
    //
    //
    //      // TEST
    //      DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    //      input->setValue(0, 1.0); // first distribution
    //      inputTest->append(input);
    //      samples->append(inputMover->getElement(i));
    //    }

    //    maxent->learn(context, inputTest, inputMover, DenseDoubleVectorPtr(),
    //        ContainerPtr(), ContainerPtr(), DenseDoubleVectorPtr());


    //    Variable result2 = worker->getFeatures(context);
    //    cout << worker->getNumResidues() << ": " << (const char*)result2.toString() << endl;

    //    EnumerationPtr kl = aminoAcidTypeEnumeration;
    //    for (size_t i = 0; i < kl->getNumElements(); i++)
    //    {
    //      EnumerationElementPtr l = kl->getElement(i);
    //      for (size_t j = 0; j < l->getNumVariables(); j++)
    //        cout << (const char*)l->getVariable(j).toString() << endl;
    //      cout << (const char*)kl->getElement(i)->getVariable(1).toString() << endl;
    //    }

# endif

# endif // LBCPP_PROTEIN_ROSETTA
# if 0
    // -------------- max ent
    VectorPtr inputs;
    VectorPtr samples;
    generateMoversDataSet(inputs, samples);

    ConditionalPoseMoverSamplerPtr MEsampler = new ConditionalPoseMoverSampler(0);

    MEsampler->learn(context, inputs, samples, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());

    ClassPtr inputClass = denseDoubleVectorClass(featuresMoverEnumerationEnumeration, doubleType);
    random = new RandomGenerator();

    context.enterScope(T("Samples 1"));
    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass, 3, 0.0);
    input->setValue(0, 20.0); // length of protein
    input->setValue(1, 1.0); // first distribution
    Variable inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = MEsampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();

    context.enterScope(T("Samples 2"));
    input = new DenseDoubleVector(inputClass, 3, 0.0);
    input->setValue(0, 10.0); // length of protein
    input->setValue(2, 1.0); // second distribution
    inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = MEsampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();
# endif

# if 0
    // --------------- Conditional sampler
    ConditionalPoseMoverSamplerPtr gen = new ConditionalPoseMoverSampler(0);
    ConditionalSimpleResidueSamplerPtr simple = new ConditionalSimpleResidueSampler();
    ConditionalResiduePairSamplerPtr pair = new ConditionalResiduePairSampler();

    VariableVectorPtr samples = new VariableVector(0);
    VectorPtr inputs = vector(denseDoubleVectorClass(falseOrTrueEnumeration, doubleType), 0);
    DenseDoubleVectorPtr features1 = new DenseDoubleVector(2, 1.0);
    features1->setValue(0, 0.21);
    DenseDoubleVectorPtr features2 = new DenseDoubleVector(2, 1.0);
    features2->setValue(0, 0.22);
    DenseDoubleVectorPtr features3 = new DenseDoubleVector(2, 1.0);
    features3->setValue(0, 0.23);
    DenseDoubleVectorPtr features4 = new DenseDoubleVector(2, 1.0);
    features4->setValue(0, 0.51);
    DenseDoubleVectorPtr features5 = new DenseDoubleVector(2, 1.0);
    features5->setValue(0, 0.49);
    DenseDoubleVectorPtr features6 = new DenseDoubleVector(2, 1.0);
    features6->setValue(0, 0.54);
    DenseDoubleVectorPtr features7 = new DenseDoubleVector(2, 1.0);
    features7->setValue(0, 0.52);
    Variable input1(features1);
    Variable input2(features2);
    Variable input3(features3);
    Variable input4(features4);
    Variable input5(features5);
    Variable input6(features6);
    Variable input7(features7);
    inputs->append(features1);
    inputs->append(features2);
    inputs->append(features3);
    inputs->append(features4);
    inputs->append(features5);
    inputs->append(features6);
    inputs->append(features7);
    //    inputs->append(input1);
    //    inputs->append(input2);
    //    inputs->append(input3);
    //    inputs->append(input4);
    //    inputs->append(input5);
    //    inputs->append(input6);
    //    inputs->append(input7);
    Variable temp1(Variable(10.0));
    Variable temp2(Variable(11.0));
    Variable temp3(Variable(12.0));
    Variable temp4(Variable(33.0));
    Variable temp5(Variable(34.0));
    Variable temp6(Variable(32.0));
    Variable temp7(Variable(31.0));
    samples->append(temp1);
    samples->append(temp2);
    samples->append(temp3);
    samples->append(temp4);
    samples->append(temp5);
    samples->append(temp6);
    samples->append(temp7);

    VectorPtr samples2 = new DenseDoubleVector(7, 0.0);
    samples2->setElement(0, (double)0.10);
    samples2->setElement(1, (double)0.11);
    samples2->setElement(2, (double)0.12);
    samples2->setElement(3, (double)0.33);
    samples2->setElement(4, (double)0.34);
    samples2->setElement(5, (double)0.32);
    samples2->setElement(6, (double)0.31);

    simple->learn(context, inputs, samples2, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(),
        DenseDoubleVectorPtr());

    for (size_t j = 10; j <= 50; j += 10)
    {
      for (size_t i = 0; i < 5; i++)
      {
        DenseDoubleVectorPtr features = new DenseDoubleVector(2, 1.0);
        features->setValue(0, (double)j / 100);
        Variable input(features);

        //PoseMoverPtr mover =
        //    gen->sample(context, random, &input).getObjectAndCast<PoseMover> ();
        Variable mover =
                    simple->sample(context, random, &input);
        //cout << (const char*)mover->toString() << endl;
        cout << (const char*)mover.toString() << endl;
      }
      cout << "===" << endl;
    }

//    VariableVectorPtr samples = new VariableVector(0);
//    VariableVectorPtr inputs = new VariableVector(0);
//    Variable input1(Variable((int)20));
//    Variable input2(Variable((int)20));
//    Variable input3(Variable((int)20));
//    Variable input4(Variable((int)50));
//    Variable input5(Variable((int)50));
//    Variable input6(Variable((int)50));
//    Variable input7(Variable((int)50));
//    inputs->append(input1);
//    inputs->append(input2);
//    inputs->append(input3);
//    inputs->append(input4);
//    inputs->append(input5);
//    inputs->append(input6);
//    inputs->append(input7);
//    RigidBodyMoverPtr temp1 = new RigidBodyMover(10, 15, 4.5, -23.3);
//    RigidBodyMoverPtr temp2 = new RigidBodyMover(11, 15, 4.1, -22.3);
//    RigidBodyMoverPtr temp3 = new RigidBodyMover(12, 17, 4.2, -21.3);
//    RigidBodyMoverPtr temp4 = new RigidBodyMover(22, 26, 5.5, -24.3);
//    RigidBodyMoverPtr temp5 = new RigidBodyMover(20, 26, 3.5, -23.1);
//    PhiPsiMoverPtr temp6 = new PhiPsiMover(24, 34.2, 87.3);
//    PhiPsiMoverPtr temp7 = new PhiPsiMover(25, 33.2, 84.3);
//    samples->append(temp1);
//    samples->append(temp2);
//    samples->append(temp3);
//    samples->append(temp4);
//    samples->append(temp5);
//    samples->append(temp6);
//    samples->append(temp7);
//
//    gen->learn(context, inputs, samples, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(),
//        DenseDoubleVectorPtr());
//
//    int count1 = 0;
//    int count2 = 0;
//    int count3 = 0;
//    for (size_t j = 10; j < 40; j += 10)
//    {
//      for (size_t i = 0; i < 20; i++)
//      {
//        Variable input((int)j);
//        PoseMoverPtr mover =
//            gen->sample(context, random, &input).getObjectAndCast<PoseMover> ();
//        cout << (const char*)mover->toString() << endl;
//
//        if (mover.isInstanceOf<PhiPsiMover> ())
//          count1++;
//        else if (mover.isInstanceOf<ShearMover> ())
//          count2++;
//        else if (mover.isInstanceOf<RigidBodyMover> ())
//          count3++;
//      }
//      cout << "===" << endl;
//    }
//
//    int num = count1 + count2 + count3;
//    cout << "il y a  " << endl;
//    cout << "phipsi : " << (double)count1 / (double)num << endl;
//    cout << "shear : " << (double)count2 / (double)num << endl;
//    cout << "rigidbody : " << (double)count3 / (double)num << endl;
# endif
#  if 0
    // --------------- residuepair sampler
    cout << "======================= dual residue sampler ===================" << endl;
    ofstream ofsdual("/Users/alex/Documents/MATLAB/residuepair_init.data");
    PairResidueSamplerPtr dual = new PairResidueSampler(50);
    random = new RandomGenerator(0);
    for (double i = 0; i < 20; i++)
    {
      PairPtr out = dual->sample(context, random).getObjectAndCast<Pair> ();
      ofsdual << out->getFirst().getInteger() << " "
      << out->getSecond().getInteger() << endl;
    }

    ofsdual.flush();
    ofsdual.close();

    dual->saveToFile(context, context.getFile(T("dualresidue.xml")));

    std::vector<PairPtr> mm(8);
    mm[0] = new Pair(4, 21);
    mm[1] = new Pair(6, 20);
    mm[2] = new Pair(5, 19);
    mm[3] = new Pair(7, 20);
    mm[4] = new Pair(46, 5);
    mm[5] = new Pair(47, 3);
    mm[6] = new Pair(47, 2);
    mm[7] = new Pair(45, 4);

    ObjectVectorPtr dataset = new ObjectVector(pairClass(integerType, integerType), 0);

    for (int i = 0; i < mm.size(); i++)
    dataset->append(mm[i]);

    dual->learn(context, ContainerPtr(), dataset, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());
    dual->computeProbabilities();

    //====================================================

    dual->saveToFile(context, context.getFile(T("dualresidue_afterlearn.xml")));
    PairResidueSamplerPtr dual2 = Variable::createFromFile(context, context.getFile(
            T("dualresidue_afterlearn.xml"))).getObjectAndCast<PairResidueSampler> ();
    ofstream ofsduallearned("/Users/alex/Documents/MATLAB/residuepair_learned.data");
    random = new RandomGenerator(0);
    RandomGeneratorPtr random2 = new RandomGenerator(0);
    for (double i = 0; i < 2000; i++)
    {
      PairPtr out = dual->sample(context, random).getObjectAndCast<Pair> ();
      cout << out->getFirst().getInteger() << ", " << out->getSecond().getInteger() << endl;
      PairPtr out2 = dual2->sample(context, random2).getObjectAndCast<Pair> ();
      cout << out2->getFirst().getInteger() << ", " << out2->getSecond().getInteger() << endl;
      ofsduallearned << out2->getFirst().getInteger() << ", "
      << out2->getSecond().getInteger() << endl;
    }

    ofsduallearned.flush();
    ofsduallearned.close();
#  endif

#  if 0
    // --------------- univarate mixture learning
    SamplerPtr gauss1 = gaussianSampler(-2, 5);
    SamplerPtr gauss2 = gaussianSampler(9, 5);
    std::vector<SamplerPtr> mixtsamp;
    mixtsamp.push_back(gauss1);
    mixtsamp.push_back(gauss2);
    DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0);
    probas->setValue(0, 0.3);
    probas->setValue(1, 0.7);
    CompositeSamplerPtr mix = mixtureSampler(probas, mixtsamp);
    cout << "=========== test sample =======================" << endl;
    ofstream ofs("/Users/alex/Documents/MATLAB/unigauss.data");

    for (int i = 0; i < 10000; i++)
    ofs << mix->sample(context, random).getDouble() << endl;

    ofs.flush();
    ofs.close();

    VariableVectorPtr learning = new VariableVector(0);

    std::vector<double> mm(8);
    mm[0] = -10.2;
    mm[1] = -12.3;
    mm[2] = -9.2;
    mm[3] = -15.2;
    mm[4] = 98.5;
    mm[5] = 99.0;
    mm[6] = 100.5;
    mm[7] = 100.0;

    for (size_t i = 0; i < mm.size(); i++)
    learning->append(Variable(mm[i]));

    cout << "=========== learning =======================" << endl;
    mix->learn(context, ContainerPtr(), learning, ContainerPtr(), ContainerPtr());

    cout << "=========== sampling again =======================" << endl;
    ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
    for (double i = 0; i < 10000; i++)
    ofs2 << mix->sample(context, random) << endl;

    ofs2.flush();
    ofs2.close();

    mix->saveToFile(context, context.getFile(T("testunimulti.xml")));

    random = new RandomGenerator(0);
    RandomGeneratorPtr random2 = new RandomGenerator(0);
    SamplerPtr
    mix2 =
    Variable::createFromFile(context, context.getFile(T("testunimulti.xml"))).getObjectAndCast<
    CompositeSampler> ();
    for (size_t i = 0; i < 10; i++)
    {
      cout << mix->sample(context, random).getDouble() << endl;
      cout << mix2->sample(context, random2).getDouble() << endl;
    }
#  endif

# if 0
    // --------------- multivarate mixture learning
    DoubleMatrixPtr means0 = new DoubleMatrix(2, 1, 0.0);
    DoubleMatrixPtr means1 = new DoubleMatrix(2, 1, 0.0);
    means0->setValue(0, 0, -14.9);
    means0->setValue(1, 0, 4.9);
    means1->setValue(0, 0, 15.0);
    means1->setValue(1, 0, -5.0);

    DoubleMatrixPtr covarianceMatrix0 = new DoubleMatrix(2, 2, 0.0);
    covarianceMatrix0->setValue(0, 0, 5.0);
    covarianceMatrix0->setValue(0, 1, 0.01);
    covarianceMatrix0->setValue(1, 0, 0.01);
    covarianceMatrix0->setValue(1, 1, 5.0);
    DoubleMatrixPtr covarianceMatrix1 = new DoubleMatrix(2, 2, 0.0);
    covarianceMatrix1->setValue(0, 0, 5.0);
    covarianceMatrix1->setValue(0, 1, 0.01);
    covarianceMatrix1->setValue(1, 0, 0.01);
    covarianceMatrix1->setValue(1, 1, 5.0);

    ScalarContinuousSamplerPtr gauss0 = multiVariateGaussianSampler(means0, covarianceMatrix0);
    ScalarContinuousSamplerPtr gauss1 = multiVariateGaussianSampler(means1, covarianceMatrix1);
    DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0);
    probas->setValue(0, 0.5);
    probas->setValue(1, 0.5);
    std::vector<SamplerPtr> mixtsamp;
    mixtsamp.push_back(gauss0);
    mixtsamp.push_back(gauss1);
    CompositeSamplerPtr mix = mixtureSampler(probas, mixtsamp);

    cout << "=========== test sample =======================" << endl;
    ofstream ofs("/Users/alex/Documents/MATLAB/multi.data");

    ObjectVectorPtr learning = new ObjectVector(doubleMatrixClass(doubleType), 0);

    std::vector<DoubleMatrixPtr> mm(8);
    mm[0] = new DoubleMatrix(2, 1, 0.0);
    mm[0]->setValue(0, 0, -12.54634);
    mm[0]->setValue(1, 0, 8.5464);
    mm[1] = new DoubleMatrix(2, 1, 0.0);
    mm[1]->setValue(0, 0, -8.2462);
    mm[1]->setValue(1, 0, 12.14646);
    mm[2] = new DoubleMatrix(2, 1, 0.0);
    mm[2]->setValue(0, 0, -10.34644);
    mm[2]->setValue(1, 0, 8.5356);
    mm[3] = new DoubleMatrix(2, 1, 0.0);
    mm[3]->setValue(0, 0, -11.59);
    mm[3]->setValue(1, 0, 12.57);
    mm[4] = new DoubleMatrix(2, 1, 0.0);
    mm[4]->setValue(0, 0, 20.57);
    mm[4]->setValue(1, 0, -26.52);
    mm[5] = new DoubleMatrix(2, 1, 0.0);
    mm[5]->setValue(0, 0, 20.95);
    mm[5]->setValue(1, 0, -25.52);
    mm[6] = new DoubleMatrix(2, 1, 0.0);
    mm[6]->setValue(0, 0, 20.52);
    mm[6]->setValue(1, 0, -26.3);
    mm[7] = new DoubleMatrix(2, 1, 0.0);
    mm[7]->setValue(0, 0, 20.2);
    mm[7]->setValue(1, 0, -25.2);

    for (size_t i = 0; i < mm.size(); i++)
    learning->append(mm[i]);

    cout << "=========== learning =======================" << endl;
    mix->learn(context, ContainerPtr(), learning, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());

    File outmix = context.getFile(T("mixturesampler_mix.xml"));
    File outp = context.getFile(T("mixturesampler_p.xml"));
    mix->saveToFile(context, outmix);

    for (int i = 0; i < 10; i++)
    {
      DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<DoubleMatrix> ();
      cout << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
    }

    for (double i = -50; i <= 50; i++)
    {
      for (double j = -50; j <= 50; j++)
      {
        DoubleMatrixPtr point = new DoubleMatrix(2, 1, i);
        point->setValue(1, 0, j);
        ObjectVectorPtr conteneur = new ObjectVector(doubleMatrixClass(doubleType), 0);
        conteneur->append(point);
        DenseDoubleVectorPtr probs = mix->computeProbabilities(ContainerPtr(), conteneur);
        ofs << probs->getValue(0) << " ";
      }
      ofs << endl;
    }

    ofs.flush();
    ofs.close();

    //            cout << "=========== sampling again =======================" << endl;
    //            ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
    //            for (double i = 0; i < 10000; i++)
    //            {
    //              DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<Matrix> ();
    //              ofs2 << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
    //            }
    //
    //            ofs2.flush();
    //            ofs2.close();
    //
    //            mix->saveToFile(context, context.getFile(T("testgaussmulti.xml")));
    //
    //            random = new RandomGenerator(0);
    //            RandomGeneratorPtr random2 = new RandomGenerator(0);
    //            SamplerPtr mix2 = Variable::createFromFile(context, context.getFile(
    //                T("testgaussmulti.xml"))).getObjectAndCast<CompositeSampler> ();
    //            for (size_t i = 0; i < 5; i++)
    //            {
    //              DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<Matrix> ();
    //              cout << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
    //              DoubleMatrixPtr out2 = mix2->sample(context, random2).getObjectAndCast<Matrix> ();
    //              cout << out2->getValue(0, 0) << ", " << out2->getValue(1, 0) << endl;
    //            }
#  endif

#  if 0
    // --------------- gaussian log value function
    ScalarVectorFunctionPtr func = gaussianLogValueFunction();
    DenseDoubleVectorPtr params = new DenseDoubleVector(2, 0);
    params->setValue(0, 3.0);
    params->setValue(1, 1.5);
    Variable val(0.13);
    double result = 0;
    DenseDoubleVectorPtr grad = new DenseDoubleVector(2, 0);
    double weight = 1;
    func->computeScalarVectorFunction(params, &val, &result, &grad, weight);
    cout << result << endl;
    cout << (const char* )grad->toString() << endl;
#  endif

#  if 0
    // --------------- Samplers
    SamplerPtr samp = new PoseMoverSampler(5);
    ObjectVectorPtr learning = new ObjectVector(poseMoverClass, 0);

    // phipsi
    learning->append(phiPsiMover(1, 34, -123));
    learning->append(phiPsiMover(0, 30, -122));
    learning->append(phiPsiMover(2, 27, -121));
    learning->append(phiPsiMover(3, 33, -121));
    learning->append(phiPsiMover(1, 34, -123));
    learning->append(phiPsiMover(0, 30, -122));
    learning->append(phiPsiMover(2, 27, -121));
    // shear
    learning->append(shearMover(3, 0.9, 4.5));
    learning->append(shearMover(4, 0.7, 4.3));
    learning->append(shearMover(3, 0.8, 3.4));
    // general
    learning->append(rigidBodyMover(3, 5, 2.8, -3.4));
    learning->append(rigidBodyMover(2, 5, 2.5, -2.4));
    learning->append(rigidBodyMover(1, 3, 0.8, 3.4));
    learning->append(rigidBodyMover(3, 4, 1.2, 2.4));
    learning->append(rigidBodyMover(3, 5, 0.3, 3.4));
    // spin
    learning->append(rigidBodyMover(3, 5, 0.0, 11.3));
    learning->append(rigidBodyMover(4, 0, 0.0, 12.4));
    // trans
    learning->append(rigidBodyMover(4, 1, 10.2, 0.0));
    learning->append(rigidBodyMover(4, 2, 9.2, 0.0));
    learning->append(rigidBodyMover(4, 0, 12.1, 0.0));

    samp->learn(context, ContainerPtr(), learning);

    random = new RandomGenerator(0);

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int num = 10;
    for (int i = 0; i < num; i++)
    {
      Variable v = samp->sample(context, random);

      PoseMoverPtr t = v.getObjectAndCast<PoseMover> ();
      cout << i << " : " << (const char*)t->toString() << endl;

      if (t.isInstanceOf<PhiPsiMover> ())
      count0++;
      else if (t.isInstanceOf<ShearMover> ())
      count1++;
      else if (t.isInstanceOf<RigidBodyMover> ())
      count2++;
    }

    cout << "il y a  " << endl;
    cout << "phipsi : " << (double)count0 / (double)num << endl;
    cout << "shear : " << (double)count1 / (double)num << endl;
    cout << "rigidbody : " << (double)count2 / (double)num << endl;

    ObjectVectorPtr learning2 = new ObjectVector(poseMoverClass, 0);
    learning2->append(phiPsiMover(1, 34, -123));
    learning2->append(phiPsiMover(0, 30, -122));
    learning2->append(phiPsiMover(2, 27, -121));
    learning2->append(phiPsiMover(3, 33, -121));
    learning2->append(phiPsiMover(1, 34, -123));
    learning2->append(phiPsiMover(0, 30, -122));
    learning2->append(phiPsiMover(2, 27, -121));

    RandomGeneratorPtr random2 = new RandomGenerator(0);
    //        SamplerPtr phipsisampler = objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(5), gaussianSampler(0, 25), gaussianSampler(0, 25));
    //        phipsisampler->learn(context, ContainerPtr(), learning2);
    //        for (int i = 0; i < num; i++)
    //        {
    //          Variable v = phipsisampler->sample(context, random2);
    //          PoseMoverPtr t = v.getObjectAndCast<PoseMover>();
    //          cout << i << " : " << (const char* )t->toString() << endl;
    //        }

    File outfile = context.getFile(T("protSampler.xml"));
    samp->saveToFile(context, outfile);

    CompositeSamplerPtr rebirth = Variable::createFromFile(context, outfile).getObjectAndCast<
    CompositeSampler> ();
    CompositeSamplerPtr rebirth2 = samp->cloneAndCast<CompositeSampler> ();
    count0 = 0;
    count1 = 0;
    count2 = 0;
    random2 = new RandomGenerator(0);
    RandomGeneratorPtr random3 = new RandomGenerator(0);
    for (int i = 0; i < num; i++)
    {

      Variable v = rebirth->sample(context, random2);
      Variable v2 = rebirth2->sample(context, random3);

      PoseMoverPtr t = v.getObjectAndCast<PoseMover> ();
      PoseMoverPtr t2 = v2.getObjectAndCast<PoseMover> ();
      cout << (const char*)t->toString() << endl;
      cout << (const char*)t2->toString() << endl;
      if (t.isInstanceOf<PhiPsiMover> ())
      count0++;
      else if (t.isInstanceOf<ShearMover> ())
      count1++;
      else if (t.isInstanceOf<RigidBodyMover> ())
      count2++;
    }

    cout << "il y a  " << endl;
    cout << "phipsi : " << (double)count0 / (double)num << endl;
    cout << "shear : " << (double)count1 / (double)num << endl;
    cout << "rigidbody : " << (double)count2 / (double)num << endl;

#  endif

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

void generateMoversDataSet(VectorPtr& inputs, VectorPtr& samples)
{
  size_t numExamples = 100;
  ClassPtr inputClass = denseDoubleVectorClass(featuresMoverEnumerationEnumeration, doubleType);
  inputs = vector(inputClass);
  samples = vector(poseMoverClass);

  DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass, 3, 0.0);
  input->setValue(0, 20.0); // length of protein
  input->setValue(1, 1.0); // first distribution
  for (size_t i = 0; i < numExamples / 2; ++i)
  {
    inputs->append(input);
    samples->append(phiPsiMover(15, 32, -123));

    inputs->append(input);
    samples->append(phiPsiMover(12, 34, -120));

    inputs->append(input);
    samples->append(phiPsiMover(14, 38, -121));

    inputs->append(input);
    samples->append(phiPsiMover(13, 30, -122));
  }

  input = new DenseDoubleVector(inputClass, 3, 0.0);
  input->setValue(0, 10.0); // length of protein
  input->setValue(2, 1.0); // second distribution
  for (size_t i = 0; i < numExamples / 2; ++i)
  {
    inputs->append(input);
    samples->append(shearMover(3, 0.9, 4.5));

    inputs->append(input);
    samples->append(shearMover(4, 0.7, 4.3));

    inputs->append(input);
    samples->append(shearMover(3, 0.8, 3.4));

    inputs->append(input);
    samples->append(shearMover(2, 1.0, 4.53));

//    inputs->append(input);
//    samples->append(shearMover(4, 1.1, 4.32));
//
//    inputs->append(input);
//    samples->append(shearMover(5, 0.853, 3.45));

    // general
    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.8, -3.4));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 3, 20.5, -2.4));

    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.18, -3.4));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 4, 21.2, -2.4));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 4, 20.3, -3.4));

    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.76, -4.2));

    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.76, -4.2));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 3, 21.01, -4));
  }
};

class ProteinSubWorkUnitExample : public WorkUnit
{
public:
  ProteinSubWorkUnitExample() : WorkUnit() {}

  ProteinSubWorkUnitExample(size_t num, RosettaPtr& rosetta)
    : WorkUnit(), num(num), rosetta(rosetta) {}

  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA

    rosetta->init(context, true, 0, 0);

    context.enterScope(T("Optimization protein : ") + String((int)num));
    context.informationCallback(T("Before"));

    rosetta->getLock();
    core::pose::PoseOP pose = new core::pose::Pose();
    makePoseFromSequence(
        pose,
        T("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
    rosetta->releaseLock();
    RandomGeneratorPtr gen = new RandomGenerator((int)num);

    for (size_t i = 0; i < 2000; ++i)
    {
      rosetta->getLock();
      PhiPsiMover::move(pose, gen->sampleInt(0, 19), gen->sampleDouble(-50.0, 50.0),
          gen->sampleDouble(-50.0, 50.0));
      context.progressCallback(new ProgressionState(i + 1.0, 2000.0, T("%")));
      rosetta->releaseLock();
    }
    context.informationCallback(T("After"));
    rosetta->getLock();
    context.informationCallback(String(T("Energie : ")) + String(fullAtomEnergy(pose)));
    context.leaveScope();
    rosetta->releaseLock();

#endif // LBCPP_PROTEIN_ROSETTA
    return T("Hello");
  }
protected:
  size_t num;
  RosettaPtr rosetta;
};

class ProteinTestParallelWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
//    context.informationCallback(T("test parallel WU : "));
//
//    size_t size = 6;
//
//    //    RosettaPtr rosetta = new Rosetta();
//    //    rosetta->init(context, true);
//
//    CompositeWorkUnitPtr subWorkUnits(new CompositeWorkUnit(T("Parallel protein workUnit"), size));
//    for (size_t i = 0; i < subWorkUnits->getNumWorkUnits(); ++i)
//    {
//      RosettaPtr r = pool->getElement(i).getObjectAndCast<Rosetta> ();
//      subWorkUnits->setWorkUnit(i, new ProteinSubWorkUnitExample(i, r));
//      //      subWorkUnits->setWorkUnit(i, new ProteinSubWorkUnitExample(i, rosetta));
//    }
//    subWorkUnits->setPushChildrenIntoStackFlag(true);
//    context.run(subWorkUnits);

    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class ProteinTestParallelWorkUnitClass;
  size_t arg;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

