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

namespace lbcpp
{

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
    optimizer->apply(context, pose, random, sampler, returnPose);
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

      SamplerPtr moverSampler;

      if (oneOrAll == 0)
        moverSampler = objectCompositeSampler(rigidBodyMoverClass, new ResiduePairSampler(
            currentProtein->getLength()), gaussianSampler(0, 0), gaussianSampler(0, 25));
      else
        moverSampler = new ProteinMoverSampler(currentProtein->getLength());

      ProteinOptimizerPtr o = new ProteinSimulatedAnnealingOptimizer(4.0, 0.01, 50,
          maxNumberIterations, 5, currentProtein->getName(), 0.0001, timesFeatureGeneration,
          outputDirectory);
      RandomGeneratorPtr random = new RandomGenerator(0);

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
  int oneOrAll;
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
    for (size_t i = 0; i < numIterations; i++)
      meansAndVariances[i] = new ScalarVariableMeanAndVariance();

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

      RandomGeneratorPtr random = new RandomGenerator(0);
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
        ProteinEDAOptimizerPtr opti = new ProteinEDAOptimizer(energyWeight);
        SamplerPtr out = opti->findBestMovers(context, random, targetPose, referencePose,
            moverSampler, returnMovers, numIterations, numSamples, numGoodSamples,
            numMoversToKeep, bestLearning, &energyMeans);

        // save gathered data
        for (size_t k = 0;k < numIterations; k++)
          meansAndVariances[k]->push(energyMeans->getValue(k));
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
      double meanDeltaEnergy = meansAndVariances[k]->getMean();
      context.resultCallback(T("Mean"), Variable(meanDeltaEnergy));
      context.resultCallback(T("Std Dev"), Variable(meansAndVariances[k]->getStandardDeviation()));
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
    RandomGeneratorPtr random = new RandomGenerator(0);

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
    if (!(outputDirectory.isEmpty()))
    {
      outputFile = context.getFile(outputDirectory);
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
    for (int j = 0; j < references.size(); j++)
    {
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

      RandomGeneratorPtr random = new RandomGenerator(0);
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
      }

      if (out)
        *fos << "\r\n";
    }

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
  String outputDirectory;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_
