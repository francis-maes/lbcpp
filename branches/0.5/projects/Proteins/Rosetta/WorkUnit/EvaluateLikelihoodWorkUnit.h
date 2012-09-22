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
  EvaluateLikelihoodWorkUnit(String referencesDirectory, String moversDirectory, double ratioTestSet, size_t authorized, size_t numSteps, size_t repeat)
    : referencesDirectory(referencesDirectory), moversDirectory(moversDirectory), ratioTestSet(ratioTestSet), authorized(authorized), numSteps(numSteps), repeat(repeat) {}

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

    context.enterScope(T("Learning"));
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

    context.enterScope(T("Evaluating"));
    context.enterScope(T("Loading test examples..."));

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

    context.enterScope(T("Computing likelihood vector"));
    DenseDoubleVectorPtr likelihoodVector = sampler->computeLogProbabilities(inputWorkers, inputMovers);
    context.leaveScope();

    for (size_t i = 0; i < likelihoodVector->getNumElements(); ++i)
      likelihood += likelihoodVector->getValue(i);

    context.leaveScope();

    return likelihood;
  }

  DenseDoubleVectorPtr performLearningAndEvaluation(ExecutionContext& context, RandomGeneratorPtr& rand, const juce::OwnedArray<File>& references, const File& moversFile, size_t sizeTestSet,
      size_t maxCount, const DenseDoubleVectorPtr& sizes) const
  {
    DenseDoubleVectorPtr results = new DenseDoubleVector(numSteps, 0.0);

    size_t numReferences = (size_t)references.size();

    // generate orders of test set and learning set
    std::vector<size_t> res;
    rand->sampleOrder(numReferences, res);

    std::vector<size_t> testSet(sizeTestSet);
    for (size_t i = 0; (i < sizeTestSet) && (i < numReferences); ++i)
      testSet[i] = res[i];

    std::vector<size_t> learningSet(maxCount);
    for (size_t i = 0; (i < maxCount) && (i + sizeTestSet < numReferences); ++i)
      learningSet[i] = res[i + sizeTestSet];

    // computing
    context.enterScope(T("Computing"));
    for (size_t i = 0; i < numSteps; ++i)
    {
      context.enterScope(T("Step : ") + String((int)i));
      size_t thisSize = (size_t)sizes->getValue(i);
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
      context.enterScope(T("r"));
      context.resultCallback(T("Size"), sizes->getValue(i));
      double likelihood = results->getValue(i);
      context.resultCallback(T("Likelihood"), likelihood);
      context.leaveScope(likelihood);
    }
    context.leaveScope();

    return results;
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
    RandomGeneratorPtr rand = new RandomGenerator();

    // sizesof sets
    size_t sizeTestSet = (size_t)((double)references.size() * ratioTestSet);
    size_t sizeLearningSet = references.size() - sizeTestSet;
    repeat = juce::jmax((int)repeat, 1);

    size_t maxCount = juce::jmin((int)authorized, (int)sizeLearningSet);
    if (maxCount <= numSteps)
      numSteps = maxCount / 2;
    if ((sizeTestSet <= 1) || (maxCount <= 1) || (numSteps <= 1))
    {
      context.errorCallback(T("Set sizes : sets too small."));
      return Variable();
    }

    // results
    DenseDoubleVectorPtr results = new DenseDoubleVector(numSteps, 0.0);
    DenseDoubleVectorPtr sizes = new DenseDoubleVector(numSteps, 0.0);

    // computing sizes of learning sets
    for (size_t i = 0; i < numSteps; ++i)
    {
      size_t thisSize = (size_t)(maxCount * ((double)(numSteps - i) / (double)(numSteps)));
      sizes->setValue(i, (double)thisSize);
    }

    // perform likelihood estimations
    std::vector<ScalarVariableMeanAndVariancePtr> means(numSteps);
    for (size_t j = 0; j < numSteps; ++j)
      means[j] = new ScalarVariableMeanAndVariance();

    context.enterScope(T("Computing iterations"));
    for (size_t i = 0; i < repeat; ++i)
    {
      context.enterScope(T("It"));
      DenseDoubleVectorPtr r = performLearningAndEvaluation(context, rand, references, moversFile, sizeTestSet, maxCount, sizes);
      context.leaveScope();

      context.progressCallback(new ProgressionState((size_t)(i + 1), repeat, T("It")));

      // aggregate results
      for (size_t j = 0; j < numSteps; ++j)
        means[j]->push(r->getValue(j));
    }
    context.leaveScope();

    // show results
    context.enterScope(T("Results"));
    for (int i = (numSteps - 1); i >= 0; --i)
    {
      context.enterScope(T("Result"));
      context.resultCallback(T("Size"), sizes->getValue(i));
      double likelihood = means[i]->getMean();
      results->setValue(i, likelihood);
      context.resultCallback(T("Likelihood"), likelihood);
      context.resultCallback(T("Var"), means[i]->getVariance());
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
  size_t repeat;
};

extern WorkUnitPtr evaluateLikelihoodWorkUnit(String referencesDirectory,
                                              String moversDirectory,
                                              double ratioTestSet,
                                              size_t authorized,
                                              size_t numSteps,
                                              size_t repeat);

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_EVALUATELIKELIHOODWORKUNIT_H_
