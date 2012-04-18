/*-----------------------------------------.---------------------------------.
| Filename: EvaluateLikelihoodWorkUnit.h   | Evaluate Likelihood WorkUnit    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Apr 16, 2012  4:09:06 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_EVALUATELIKELIHOODWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_EVALUATELIKELIHOODWORKUNIT_H_

# include "../Data/Features/PoseFeatureGenerator.h"
# include "../Data/Pose.h"
# include "../Data/Rosetta.h"
# include "../Data/Mover/PoseMover.h"
# include "../Data/MoverSampler/ConditionalPoseMoverSampler.h"

namespace lbcpp
{

class EvaluateLikelihoodWorkUnit : public WorkUnit
{
public:
  EvaluateLikelihoodWorkUnit() {}
  EvaluateLikelihoodWorkUnit(String referencesDirectory, String moversDirectory, double ratioTestSet, size_t authorized, size_t numSteps)
    : referencesDirectory(referencesDirectory), moversDirectory(moversDirectory), ratioTestSet(ratioTestSet), authorized(authorized), numSteps(numSteps) {}

  SamplerPtr learnDistribution(ExecutionContext& context, const juce::OwnedArray<File>& references, const File& moversFile, const std::vector<size_t>& indexes, size_t numLearned) const
  {
    PosePtr pose = new Pose(T("AAAA"));

    // features
    PoseFeatureGeneratorPtr features = new PoseFeatureGenerator();
    features->initialize(context, poseClass);
    DoubleVectorPtr initializeFeatures = features->compute(context, pose).getObjectAndCast<DoubleVector> ();

    // learn the distribution
    VectorPtr inputWorkers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    VectorPtr inputMovers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    SamplerPtr sampler = new ConditionalPoseMoverSampler(pose->getLength());

    context.enterScope(T("Loading learning examples..."));

    size_t counter = 0;
    for (size_t i = 0; i < numLearned; i++)
    {
      size_t ind = indexes[i];

      juce::OwnedArray<File> movers;
      String nameToSearch = (*references[ind]).getFileNameWithoutExtension();

      PosePtr protein = new Pose(*references[ind]);

      nameToSearch += T("_mover.xml");
      moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
      if (movers.size() > 0)
      {
        DoubleVectorPtr inFeatures = features->compute(context, protein).getObjectAndCast<DoubleVector> ();
        PoseMoverPtr inMover = Variable::createFromFile(context, (*movers[0])).getObjectAndCast<PoseMover> ();
        inputWorkers->append(inFeatures);
        inputMovers->append(inMover);
        context.progressCallback(new ProgressionState((size_t)(++counter), numLearned, T("Intermediate conformations")));
      }
      else
        context.informationCallback(T("Structure not loaded : ") + nameToSearch);
    }
    context.leaveScope();

    ContainerPtr addWorkers = inputWorkers;
    ContainerPtr addMovers = inputMovers;

    context.enterScope(T("Learning distribution..."));
    sampler->learn(context, addWorkers, addMovers, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(), DenseDoubleVectorPtr());
    context.leaveScope();

    return sampler;
  }

  double evaluateLikelihood(ExecutionContext& context, SamplerPtr sampler, const juce::OwnedArray<File>& references, const File& moversFile, const std::vector<size_t>& indexes) const
  {
    double likelihood = 0;
    PosePtr pose = new Pose(T("AAAA"));

    // features
    PoseFeatureGeneratorPtr features = new PoseFeatureGenerator();
    features->initialize(context, poseClass);
    DoubleVectorPtr initializeFeatures = features->compute(context, pose).getObjectAndCast<DoubleVector> ();

    // learn the distribution
    VectorPtr inputWorkers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    VectorPtr inputMovers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));

    context.enterScope(T("Loading learning examples..."));

    size_t counter = 0;
    for (size_t i = 0; i < indexes.size(); i++)
    {
      size_t ind = indexes[i];

      juce::OwnedArray<File> movers;
      String nameToSearch = (*references[ind]).getFileNameWithoutExtension();

      PosePtr protein = new Pose(*references[ind]);

      nameToSearch += T("_mover.xml");
      moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
      if (movers.size() > 0)
      {
        DoubleVectorPtr inFeatures = features->compute(context, protein).getObjectAndCast<DoubleVector> ();
        PoseMoverPtr inMover = Variable::createFromFile(context, (*movers[0])).getObjectAndCast<PoseMover> ();
        inputWorkers->append(inFeatures);
        inputMovers->append(inMover);
        context.progressCallback(new ProgressionState((size_t)(++counter), indexes.size(), T("Intermediate conformations")));
      }
      else
        context.informationCallback(T("Structure not loaded : ") + nameToSearch);
    }
    context.leaveScope();

    ContainerPtr addWorkers = inputWorkers;
    ContainerPtr addMovers = inputMovers;

    DenseDoubleVectorPtr likelihoodVector = sampler->computeLogProbabilities(inputWorkers, inputMovers);

    for (size_t i = 0; i < likelihoodVector->getNumElements(); ++i)
      likelihood += likelihoodVector->getValue(i);

    return likelihood;
  }

  virtual Variable run(ExecutionContext& context)
  {
    // initialization
    if (referencesDirectory.isEmpty() || moversDirectory.isEmpty())
    {
      context.errorCallback(T("Directories name : empty."));
      return Variable();
    }

    File referencesFile = context.getFile(referencesDirectory);
    File moversFile = context.getFile(moversDirectory);
    if (!referencesFile.exists() || !moversFile.exists())
    {
      context.errorCallback(T("Directories : not found."));
      return Variable();
    }
    juce::OwnedArray<File> references;
    referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));
    if (references.size() < 0)
    {
      context.errorCallback(T("Directories : no proteins loaded."));
      return Variable();
    }

    Rosetta ros;
    ros.init(context, false, 0, 0);

    // sizesof sets
    size_t sizeTestSet = (double)references.size() * ratioTestSet;
    size_t sizeLearningSet = references.size() - sizeTestSet;

    size_t maxCount = juce::jmin((int)authorized, (int)sizeLearningSet);
    if (sizeLearningSet <= numSteps)
      numSteps = sizeLearningSet * 0.5;
    if ((sizeTestSet <= 1) || (sizeLearningSet <= 1) || (numSteps <= 1))
    {
      context.errorCallback(T("Set sizes : sets too small."));
      return Variable();
    }

    // generate orders of test set and learning set
    std::vector<size_t> res;
    RandomGeneratorPtr rand = new RandomGenerator();
    rand->sampleOrder(references.size(), res);

    std::vector<size_t> testSet(sizeTestSet);
    for (size_t i = 0; (i < sizeTestSet) && (i < references.size()); ++i)
      testSet[i] = res[i];

    std::vector<size_t> learningSet(maxCount);
    for (size_t i = 0; (i < sizeLearningSet) && (i + sizeTestSet < references.size()); ++i)
      learningSet[i] = res[i + sizeTestSet];

    // results
    DenseDoubleVectorPtr results = new DenseDoubleVector(numSteps, 0.0);
    DenseDoubleVectorPtr sizes = new DenseDoubleVector(numSteps, 0.0);

    // computing
    context.enterScope(T("Computing"));
    for (size_t i = 0; i < numSteps; ++i)
    {
      context.enterScope(T("Step : ") + String((int)i));
      size_t thisSize = maxCount * ((double)(numSteps - i) / (double)(numSteps));
      sizes->setValue(i, (double)thisSize);
      double likelihood = 0.0;
      if (thisSize > 1)
      {
        SamplerPtr sampler = learnDistribution(context, references, moversFile, learningSet, thisSize);
        likelihood = evaluateLikelihood(context, sampler, references, moversFile, testSet);
      }
      results->setValue(i, likelihood);
      context.leaveScope(likelihood);
    }
    context.leaveScope();

    // results
    context.enterScope(T("Results"));
    for (int i = (numSteps - 1); i >= 0; --i)
    {
      context.enterScope(T("Result"));
      context.resultCallback(T("Size"), sizes->getValue(i));
      double likelihood = results->getValue(i);
      context.resultCallback(T("Likelihood"), likelihood);
      context.leaveScope(likelihood);
    }
    context.leaveScope();

    return results;
  }

protected:
  friend class EvaluateLikelihoodWorkUnitClass;

  String referencesDirectory;
  String moversDirectory;
  double ratioTestSet;
  size_t authorized;
  size_t numSteps;
};

extern WorkUnitPtr evaluateLikelihoodWorkUnit(String referencesDirectory,
                                              String moversDirectory,
                                              double ratioTestSet,
                                              size_t authorized,
                                              size_t numSteps);

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_EVALUATELIKELIHOODWORKUNIT_H_

