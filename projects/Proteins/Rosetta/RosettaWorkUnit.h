/*-----------------------------------------.---------------------------------.
| Filename: RosettaWorkUnit.h              | Rosetta WorkUnits               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 22/04/2011 15:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_

# include "precompiled.h"
# include <lbcpp/Data/RandomVariable.h>
# include "../Data/Protein.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "RosettaUtils.h"
# include "ProteinOptimizer.h"
# include "ProteinMover.h"
# include "ProteinOptimizer/SimulatedAnnealingOptimizer.h"
# include "Sampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "../Data/TertiaryStructure.h"

namespace lbcpp
{

class ProteinOptimizationWithLearningWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(context, false);

    File inputFile = context.getFile(inputDirectory);
    if (!inputFile.exists())
      context.errorCallback(T("Proteins' directory not found."));

    juce::File outputFile = context.getFile(outputDirectory);
    if (!outputFile.exists())
      outputFile.createDirectory();

    File referencesFile = context.getFile(referencesDirectory);
    if (!referencesFile.exists() && (learningPolicy > 0))
      context.errorCallback(T("References' directory not found."));

    File moversFile = context.getFile(moversDirectory);
    if (!moversFile.exists() && (learningPolicy > 0))
      context.errorCallback(T("Movers' directory not found."));

    VariableVectorPtr inputWorkers = new VariableVector(0);
    VariableVectorPtr inputMovers = new VariableVector(0);
    if (learningPolicy)
    {
      context.enterScope(T("Loading learning examples..."));
      juce::OwnedArray<File> references;
      referencesFile.findChildFiles(references, File::findFiles, false, T("*.xml"));

      for (size_t i = 0; i < references.size(); i++)
      {
        context.progressCallback(new ProgressionState((size_t)i, (size_t)references.size(),
            T("Intermediate conformations")));
        juce::OwnedArray<File> movers;
        String nameToSearch = (*references[i]).getFileNameWithoutExtension();

        ProteinPtr protein = Protein::createFromFile(context, (*references[i]));
        core::pose::PoseOP pose;
        convertProteinToPose(context, protein, pose);

        nameToSearch += T("_mover_*.xml");
        moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
        for (size_t j = 0; j < movers.size(); j++)
        {
          RosettaProteinPtr inWorker = new RosettaProtein(pose, residueFeatures, energyFeatures,
              histogramFeatures, distanceFeatures);
          ProteinMoverPtr inMover =
              Variable::createFromFile(context, (*movers[j])).getObjectAndCast<ProteinMover> ();
          inputWorkers->append(inWorker);
          inputMovers->append(inMover);
        }
      }
      context.progressCallback(new ProgressionState((size_t)references.size(), (size_t)references.size(),
          T("Intermediate conformations")));
      context.leaveScope();
    }

    core::pose::PoseOP currentPose;

    juce::OwnedArray<File> results;
    inputFile.findChildFiles(results, File::findFiles, false, T("*.xml"));

    double frequenceVerbosity = 0.0001;
    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariances;

    for (size_t i = 0; i < results.size(); i++)
    {
      ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));
      String currentName = currentProtein->getName();

      convertProteinToPose(context, currentProtein, currentPose);

      if ((int)currentProtein->getLength() != (int)currentPose->n_residue())
      {
        context.warningCallback(T("Conversion from protein to pose incorrect, skipping : ") + currentName);
        continue;
      }

      core::pose::PoseOP initialPose;
      initializeProteinStructure(currentPose, initialPose);
      context.enterScope(T("Optimizing protein : ") + currentName);

      RosettaWorkerPtr worker = new RosettaWorker(initialPose, learningPolicy, residueFeatures,
          energyFeatures, histogramFeatures, distanceFeatures);
      ContainerPtr addWorkers = inputWorkers;
      ContainerPtr addMovers = inputMovers;
      // learn
      worker->learn(context, addWorkers, addMovers);

      RandomGeneratorPtr random = new RandomGenerator();
      DenseDoubleVectorPtr energiesAtIteration;
      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(4.0,
          0.01, 50, 500000, 5, currentName, frequenceVerbosity, 10, outputFile);

      optimizer->apply(context, worker, random, energiesAtIteration);

      for (size_t j = 0; j < energiesAtIteration->getNumValues(); j++)
        if (j >= meansAndVariances.size())
        {
          meansAndVariances.push_back(new ScalarVariableMeanAndVariance());
          meansAndVariances[j]->push(energiesAtIteration->getValue(j));
        }
        else
          meansAndVariances[j]->push(energiesAtIteration->getValue(j));

      context.leaveScope(String("Done."));
    }

    // export results
    context.enterScope(T("Results"));
    for (size_t k = 0; k < meansAndVariances.size(); k++)
    {
      context.enterScope(T("Values"));
      context.resultCallback(T("Iteration"), Variable(k));
      double meanEnergy = meansAndVariances[k]->getMean();
      context.resultCallback(T("Mean energy"), Variable(meanEnergy));
      context.resultCallback(T("Std Dev energy"), Variable(meansAndVariances[k]->getStandardDeviation()));
      context.leaveScope(Variable(meanEnergy));
    }
    context.leaveScope();

    context.informationCallback(T("Done."));
# endif
    return Variable();
  }

protected:
  friend class ProteinOptimizationWithLearningWorkUnitClass;

  String inputDirectory;
  String outputDirectory;
  String referencesDirectory;
  String moversDirectory;
  size_t learningPolicy;
  size_t residueFeatures;
  size_t energyFeatures;
  size_t histogramFeatures;
  size_t distanceFeatures;
};

class ProteinOptimizerWorkUnit;
typedef ReferenceCountedObjectPtr<ProteinOptimizerWorkUnit> ProteinOptimizerWorkUnitPtr;

class ProteinOptimizerWorkUnit : public CompositeWorkUnit
{
public:
  // returnPose must be already instantiated
  ProteinOptimizerWorkUnit()
  {
  }

  ProteinOptimizerWorkUnit(const String& proteinName, const core::pose::PoseOP& pose,
      ProteinOptimizerPtr& optimizer, SamplerPtr& sampler, RandomGeneratorPtr& random)
    : CompositeWorkUnit(T("ProteinOptimizerWorkUnit"))
  {
    this->proteinName = proteinName;
#ifdef LBCPP_PROTEIN_ROSETTA
    this->pose = pose;
#endif // LBCPP_PROTEIN_ROSETTA
    this->optimizer = optimizer;
    this->sampler = sampler;
    this->random = random;
  }

  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    context.informationCallback(T("Protein : ") + proteinName + T(" loaded succesfully."));
    context.resultCallback(T("Initial energy"),
        Variable(getConformationScore(pose, fullAtomEnergy)));
    core::pose::PoseOP returnPose = new core::pose::Pose(*pose);

    RosettaWorkerPtr worker = new RosettaWorker(pose, 0, 0, 0, 0, 0);

    RandomGeneratorPtr random = new RandomGenerator();
    DenseDoubleVectorPtr energiesAtIteration;

    optimizer->apply(context, worker, random, energiesAtIteration);

    context.informationCallback(T("Optimization done."));
    double score = getConformationScore(returnPose, fullAtomEnergy);
    context.resultCallback(T("Final energy"), Variable(score));

    return Variable(proteinName);
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class ProteinOptimizerWorkUnitClass;
  String proteinName;
#ifdef LBCPP_PROTEIN_ROSETTA
  core::pose::PoseOP pose;
#endif // LBCPP_PROTEIN_ROSETTA
  ProteinOptimizerPtr optimizer;
  SamplerPtr sampler;
  RandomGeneratorPtr random;
};

class ProteinFeaturesGeneratorWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(context, false);

    // Load all xml files in proteinsDir
    File directory = context.getFile(proteinsDirectory);
    if (!directory.exists())
      context.errorCallback(T("Proteins' directory not found."));
    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, directory);
    int numProteins = proteins->getNumElements();
    double frequenceVerbosity = 0.0001;

    // Creating parallel workunits
    CompositeWorkUnitPtr proteinsOptimizer = new CompositeWorkUnit(T("ProteinsOptimizer"), numProteins);
    for (size_t i = 0; i < proteinsOptimizer->getNumWorkUnits(); i++)
    {
      ProteinPtr currentProtein = proteins->getElement(i).getObjectAndCast<Protein> ();
      String currentName = currentProtein->getName();
      core::pose::PoseOP currentPose;
      convertProteinToPose(context, currentProtein, currentPose);
      core::pose::PoseOP initialPose;
      initializeProteinStructure(currentPose, initialPose);
      core::pose::PoseOP returnPose = new core::pose::Pose();

      // Output arguments
      juce::File outputDirectory;
      if (timesFeatureGeneration > 0)
      {
        outputDirectory = context.getFile(resultsDirectory);
        if (!outputDirectory.exists())
          outputDirectory.createDirectory();
      }

      SamplerPtr moverSampler = new ProteinMoverSampler(currentProtein->getLength());

      ProteinOptimizerPtr o = new ProteinSimulatedAnnealingOptimizer(4.0, 0.01, 50,
          maxNumberIterations, 5, currentProtein->getName(), 0.0001, timesFeatureGeneration,
          outputDirectory);
      RandomGeneratorPtr random = new RandomGenerator();

      WorkUnitPtr childWorkUnit = new ProteinOptimizerWorkUnit(currentName, initialPose, o,
          moverSampler, random);
      proteinsOptimizer->setWorkUnit(i, childWorkUnit);
    }

    proteinsOptimizer->setPushChildrenIntoStackFlag(true);
    context.informationCallback(T("Computing..."));
    context.run(proteinsOptimizer);

    context.informationCallback(T("ProteinFeaturesGeneratorWorkUnit done."));
    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class ProteinFeaturesGeneratorWorkUnitClass;

  String proteinsDirectory;
  String resultsDirectory;
  int timesFeatureGeneration;
  int maxNumberIterations;
};

class XmlToPDBConverterWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    // Load all xml files in proteinsDir
    File directory = context.getFile(proteinsDirectory);
    if (!directory.exists())
      context.errorCallback(T("Proteins' directory not found."));

    juce::OwnedArray<File> results;
    directory.findChildFiles(results, File::findFiles, false, T("*.xml"));

    // Other arguments
    juce::File outputDirectory = context.getFile(resultsDirectory);
    if (!outputDirectory.exists())
      outputDirectory.createDirectory();

    context.informationCallback(T("Performing conversion..."));

    for (int i = 0; i < results.size(); i++)
    {
      ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));

      String nameXmlFile = results[i]->getFileNameWithoutExtension();

      File outFile(outputDirectory.getFullPathName() + T("/") + nameXmlFile + T(".pdb"));
      currentProtein->saveToPDBFile(context, outFile);
    }

    context.informationCallback(T("Conversion in PDB files done."));
    return Variable();
  }

protected:
  friend class XmlToPDBConverterWorkUnitClass;
  String proteinsDirectory;
  String resultsDirectory;
};

class FilesHarmonizationWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    File homeDirectory = context.getFile(proteinsDirectory);
    if (!homeDirectory.exists())
    {
      context.errorCallback(T("Proteins' directory not found."));
      return Variable();
    }

    juce::OwnedArray<File> directories;
    homeDirectory.findChildFiles(directories, File::findDirectories, false);

    for (int j = 0; j < directories.size(); j++)
    {
      juce::OwnedArray<File> results;
      (*directories[j]).findChildFiles(results, File::findFiles, false, T("*.xml"));

      for (int i = 0; i < results.size(); i++)
      {
        String nameToModify = (*results[i]).getFileNameWithoutExtension();
        String goodName;
        if (nameToModify.contains(T("_")))
          goodName = nameToModify.upToFirstOccurrenceOf(T("_"), false, true);
        else
          goodName = nameToModify;

        bool exist = existsForAllDirectories(directories, goodName);
        if (!exist)
          deleteForAllDirectories(directories, goodName);
        else
        {
          if (nameToModify.contains(T("_")))
          {
            String num = nameToModify.fromLastOccurrenceOf(T("_"), false, true);
            while (num.length() <= 4)
              num = T("0") + num;
            File newFile((*directories[j]).getFullPathName() + T("/") + goodName + T("_") + num
                + T(".xml"));
            (*results[i]).moveFileTo(newFile);
          }
        }

      }
    }
    context.informationCallback(T("All files harmonized."));
    return Variable();
  }

  bool existsForAllDirectories(juce::OwnedArray<File>& directories, String name)
  {
    bool exists = true;
    for (int k = 0; k < directories.size(); k++)
    {
      juce::OwnedArray<File> results;
      (*directories[k]).findChildFiles(results, File::findFiles, false, name + T("*.xml"));
      if (results.size() == 0)
      {
        exists = false;
        break;
      }
    }
    return exists;
  }

  void deleteForAllDirectories(juce::OwnedArray<File>& directories, String name)
  {
    for (int k = 0; k < directories.size(); k++)
    {
      juce::OwnedArray<File> results;
      (*directories[k]).findChildFiles(results, File::findFiles, false, name + T("*.xml"));
      for (int i = 0; i < results.size(); i++)
      {
        (*results[i]).deleteFile();
      }
    }
  }

protected:
  friend class FilesHarmonizationWorkUnitClass;
  String proteinsDirectory;
};

class SamplerGenerationWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(context, false);

    File referenceFile = context.getFile(referenceDirectory);
    if (!referenceFile.exists())
    {
      context.errorCallback(T("References' directory not found."));
      return Variable();
    }
    File targetFile = context.getFile(targetDirectory);
    if (!targetFile.exists())
    {
      context.errorCallback(T("Targets' directory not found."));
      return Variable();
    }
    File outputFile = context.getFile(outputDirectory);
    if (!outputFile.exists())
    {
      outputFile.createDirectory();
    }

    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariances(numIterations);
    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariancesQScore(numIterations);
    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariancesScores(numIterations);
    for (size_t i = 0; i < numIterations; i++)
    {
      meansAndVariances[i] = new ScalarVariableMeanAndVariance();
      meansAndVariancesQScore[i] = new ScalarVariableMeanAndVariance();
      meansAndVariancesScores[i] = new ScalarVariableMeanAndVariance();
    }

    juce::OwnedArray<File> references;
    referenceFile.findChildFiles(references, File::findFiles, false, T("*.xml"));

    context.enterScope(T("Finding best movers"));

    for (int j = 0; j < references.size(); j++)
    {
      ProteinPtr proteinRef = Protein::createFromXml(context, *references[j]);
      core::pose::PoseOP referencePose;
      convertProteinToPose(context, proteinRef, referencePose);

      // verbosity
      context.enterScope((*references[j]).getFileNameWithoutExtension() + T(" EDA optimization"));
      context.resultCallback(T("Energy"), Variable(getConformationScore(referencePose,
          centroidEnergy)));

      juce::OwnedArray<File> targets;
      targetFile.findChildFiles(targets, File::findFiles, false,
          (*references[j]).getFileNameWithoutExtension() + T("*.xml"));

      RandomGeneratorPtr random = new RandomGenerator();
      for (int i = 0; i < targets.size(); i++)
      {
        ProteinPtr proteinTarget = Protein::createFromXml(context, *targets[i]);
        core::pose::PoseOP targetPose;
        convertProteinToPose(context, proteinTarget, targetPose);

        // verbosity
        context.progressCallback(new ProgressionState((double)i, (double)targets.size(),
            T("Intermediate conformations")));
        context.enterScope((*targets[i]).getFileNameWithoutExtension()
            + T(" intermediate conformation"));
        context.resultCallback(T("Energy"), Variable(getConformationScore(targetPose,
            centroidEnergy)));

        // choose sampler
        SamplerPtr moverSampler;
        if (oneOrAll == 0)
          moverSampler = objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(
              targetPose->n_residue()), gaussianSampler(0, 25), gaussianSampler(0, 25));
        else
          moverSampler = new ProteinMoverSampler(targetPose->n_residue());

        bool bestLearning = true;
        if (includeBestMoversInLearning == 0)
          bestLearning = false;

        std::vector<ProteinMoverPtr> returnMovers(numMoversToKeep);

        // find best movers by EDA
        DenseDoubleVectorPtr energyMeans = new DenseDoubleVector(numIterations, 0);
        DenseDoubleVectorPtr qScoreMeans = new DenseDoubleVector(numIterations, 0);
        DenseDoubleVectorPtr scoreMeans = new DenseDoubleVector(numIterations, 0);
        ProteinEDAOptimizerPtr opti = new ProteinEDAOptimizer(energyWeight);
        SamplerPtr out = opti->findBestMovers(context, random, targetPose, referencePose,
            moverSampler, returnMovers, numIterations, numSamples, numGoodSamples, numMoversToKeep,
            bestLearning, &energyMeans, &qScoreMeans, &scoreMeans);

        // save gathered data
        for (size_t k = 0;k < numIterations; k++)
        {
          meansAndVariances[k]->push(energyMeans->getValue(k));
          meansAndVariancesQScore[k]->push(qScoreMeans->getValue(k));
          meansAndVariancesScores[k]->push(scoreMeans->getValue(k));
        }

        out->saveToFile(context, File(outputFile.getFullPathName() + T("/")
            + (*targets[i]).getFileNameWithoutExtension() + T("_sampler.xml")));

        for (int k = 0; k < numMoversToKeep; k++)
          returnMovers[k]->saveToFile(context, File(outputFile.getFullPathName() + T("/")
              + (*targets[i]).getFileNameWithoutExtension() + T("_mover_") + String(k) + T(".xml")));

        // verbosity
        context.leaveScope();
      }
      context.progressCallback(new ProgressionState((double)targets.size(), (double)targets.size(),
          T("Intermediate conformations")));
      context.leaveScope();
    }

    // export results
    context.enterScope(T("Results"));
    for (size_t k = 0; k < numIterations; k++)
    {
      context.enterScope(T("Values"));
      context.resultCallback(T("Iteration"), Variable(k));
      context.resultCallback(T("Mean deltaScore"), Variable(meansAndVariancesScores[k]->getMean()));
      context.resultCallback(T("Std deltaScore"), Variable(meansAndVariancesScores[k]->getStandardDeviation()));
      double meanDeltaEnergy = meansAndVariances[k]->getMean();
      context.resultCallback(T("Mean deltaEnergy"), Variable(meanDeltaEnergy));
      context.resultCallback(T("Std Dev deltaEnergy"), Variable(meansAndVariances[k]->getStandardDeviation()));
      context.resultCallback(T("Mean QScore"), Variable(meansAndVariancesQScore[k]->getMean()));
    context.resultCallback(T("Std Dev QScore"), Variable(meansAndVariancesQScore[k]->getStandardDeviation()));
      context.leaveScope(Variable(meanDeltaEnergy));
    }
    context.leaveScope();
    context.leaveScope();

    context.informationCallback(T("All samplers generated."));
    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class SamplerGenerationWorkUnitClass;

  String referenceDirectory;
  String targetDirectory;
  String outputDirectory;
  double energyWeight;
  int numIterations;
  int numSamples;
  int includeBestMoversInLearning;
  int numMoversToKeep;
  int oneOrAll;
  int numGoodSamples;
};

class ConformationSortingWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);
    RandomGeneratorPtr random = new RandomGenerator();

    File dir = context.getFile(referenceDir);
    if (!dir.exists())
    {
      context.errorCallback(T("Input directory for references not found."));
      return Variable();
    }

    juce::OwnedArray<File> references;
    dir.findChildFiles(references, File::findFiles, false, T("*.xml"));

    File opt = context.getFile(targetDir);
    if (!opt.exists())
    {
      context.errorCallback(T("Input directory for targets not found."));
      return Variable();
    }

    int count = 0;
    for (int j = 0; j < references.size(); j++)
    {
      ProteinPtr refProtein = Protein::createFromXml(context, (*references[j]));
      double refScore = getConformationScore(context, refProtein, centroidEnergy);
      double factor = 1.2;
      if (refScore < 1)
        factor = std::exp(-0.2 * (1 + std::log(refScore)));

      context.enterScope(T("Protein name : ") + (*references[j]).getFileNameWithoutExtension());
      context.informationCallback(T("Score : ") + String(refScore));
      context.informationCallback(T("Length : ") + String((int)refProtein->getLength()));

      juce::OwnedArray<File> results;
      opt.findChildFiles(results, File::findFiles, false,
          (*references[j]).getFileNameWithoutExtension() + T("*.xml"));

      int deleted = 0;
      for (int i = 0; i < results.size(); i++)
      {
        ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));
        double score = getConformationScore(context, currentProtein, centroidEnergy);
        context.informationCallback((*results[i]).getFileNameWithoutExtension() + T(" : ")
            + String(score));
        if (score <= factor * refScore)
        {
          count++;
          deleted++;
          context.informationCallback(T("To drop : ") + (*results[i]).getFileNameWithoutExtension());
          context.informationCallback(T("Its score : ") + String(score));

          if (del)
            (*results[i]).deleteFile();
        }
      }
      context.leaveScope(Variable(deleted));
    }

    if (del)
      context.informationCallback(T("Number deleted files : ") + String(count));
    else
      context.informationCallback(T("Number files found : ") + String(count));

    return Variable();
  }

protected:
  friend class ConformationSortingWorkUnitClass;
  String referenceDir;
  String targetDir;
  bool del;
};

class EnergyAndQScoreComparisonWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(context, false);
    File referenceFile = context.getFile(referenceDirectory);
    File targetFile = context.getFile(targetDirectory);

    File outputFile;
    bool out = false;
    juce::OutputStream *fos;
    if (!(outputPath.isEmpty()))
    {
      outputFile = context.getFile(outputPath);
      if (outputFile.exists())
      {
        outputFile.deleteFile();
        outputFile.create();
      }
      fos = outputFile.createOutputStream();
      out = true;
    }

    juce::OwnedArray<File> references;
    referenceFile.findChildFiles(references, File::findFiles, false, T("*.xml"));

    std::vector<File> toDelete;
    for (size_t j = 0; j < references.size(); j++)
    {
      context.progressCallback(new ProgressionState(j, (size_t)references.size(), T("Reference proteins")));
      ProteinPtr proteinRef = Protein::createFromXml(context, *references[j]);
      core::pose::PoseOP referencePose;
      convertProteinToPose(context, proteinRef, referencePose);

      std::cout << "=============================================" << std::endl;
      std::cout << "======== protein : "
          << (const char*)(*references[j]).getFileNameWithoutExtension() << " ==============="
          << std::endl;
      double totEnFA = getTotalEnergy(referencePose, fullAtomEnergy);
      double totEnC = getTotalEnergy(referencePose, centroidEnergy);
      double confScoreFA = getConformationScore(referencePose, fullAtomEnergy);
      double confScoreC = getConformationScore(referencePose, centroidEnergy);
      std::cout << "energy fa : " << totEnFA << std::endl;
      std::cout << "energy centroid : " << totEnC << std::endl;
      std::cout << "score fa : " << confScoreFA << std::endl;
      std::cout << "score centroid : " << confScoreC << std::endl;
      if (out)
      {
        *fos << (int)proteinRef->getLength() << " ";
        *fos << totEnFA << " ";
        *fos << totEnC << " ";
        *fos << confScoreFA << " ";
        *fos << confScoreC << " ";
      }

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
        totEnFA = getTotalEnergy(targetPose, fullAtomEnergy);
        totEnC = getTotalEnergy(targetPose, centroidEnergy);
        confScoreFA = getConformationScore(targetPose, fullAtomEnergy);
        confScoreC = getConformationScore(targetPose, centroidEnergy);
        std::cout << "energy fa : " << totEnFA << std::endl;
        std::cout << "energy centroid : " << totEnC << std::endl;
        std::cout << "score fa : " << confScoreFA << std::endl;
        std::cout << "score centroid : " << confScoreC << std::endl;
        if (out)
        {
          *fos << totEnFA << " ";
          *fos << totEnC << " ";
          *fos << confScoreFA << " ";
          *fos << confScoreC << " ";
        }

        int minDist = juce::jlimit(1, (int)targetPose->n_residue(), juce::jmin(20,
            targetPose->n_residue() / 2));
        int maxDist = -1;

        // pose Qscore
        QScoreObjectPtr scoresPose = QScoreSingleEvaluator(targetPose, referencePose, minDist,
            maxDist);

        if (scoresPose.get() == NULL)
        {
          toDelete.push_back((*references[j]));
          cont = false;
          continue;
        }
        double QScorePose = scoresPose->getMean();
        std::cout << "tertiary structure score pose : " << QScorePose << std::endl;
        if (out)
          *fos << QScorePose << " ";

        minDist = -1;
        maxDist = juce::jlimit(1, (int)targetPose->n_residue(), juce::jmin(20,
            targetPose->n_residue() / 2));
        // pose Qscore
        QScoreObjectPtr scoresPose2 = QScoreSingleEvaluator(targetPose, referencePose, minDist,
            maxDist);
        double QScorePose2 = scoresPose2->getMean();
        std::cout << "secondary structure score pose : " << QScorePose2 << std::endl;
        if (out)
          *fos << QScorePose2 << " ";

        size_t numGooAlign = proteinRef->getTertiaryStructure()->computeCAlphaAtomsGDTTS(
            proteinTarget->getTertiaryStructure(), 5.0);
        double gdtts = (double)numGooAlign / (double)proteinRef->getLength();
        std::cout << "GDTTS 5 ang : " << gdtts << std::endl;
        numGooAlign = proteinRef->getTertiaryStructure()->computeCAlphaAtomsGDTTS(
            proteinTarget->getTertiaryStructure(), 2.0);
        if (out)
          *fos << gdtts << " ";
        gdtts = (double)numGooAlign / (double)proteinRef->getLength();
        std::cout << "GDTTS 2 ang : " << gdtts << std::endl;
        if (out)
          *fos << gdtts << " ";
      }

      if (out)
        *fos << "\r\n";
    }

    context.progressCallback(new ProgressionState((size_t)references.size(),
        (size_t)references.size(), T("Reference proteins")));
    if (out)
    {
      fos->flush();
      delete fos;
    }
    std::cout << "deleting : " << std::endl;
    for (size_t i = 0; i < toDelete.size(); i++)
    {
      std::cout << "deleting : " << (const char*)toDelete[i].getFileNameWithoutExtension()
          << std::endl;
      toDelete[i].deleteFile();
    }

    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class EnergyAndQScoreComparisonWorkUnitClass;
  String referenceDirectory;
  String targetDirectory;
  String outputPath;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_
