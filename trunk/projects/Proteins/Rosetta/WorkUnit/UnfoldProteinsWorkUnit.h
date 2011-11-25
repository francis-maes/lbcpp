/*-----------------------------------------.---------------------------------.
| Filename: UnfoldProteinsWorkUnit.h       | WorkUnit that unfolds proteins  |
| Author  : Alejandro Marcos Alvarez       | to generate good movers.        |
| Started : 25/11/2011 10:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_

//# include "precompiled.h"
//# include <lbcpp/Data/RandomVariable.h>
//# include "../../Data/Protein.h"
//# include "../../Data/Formats/PDBFileGenerator.h"
# include "../RosettaUtils.h"
//# include "../ProteinOptimizer.h"
//# include "../ProteinMover.h"
//# include "../ProteinOptimizer/SimulatedAnnealingOptimizer.h"
//# include "../Sampler.h"
//# include "../Sampler/ProteinMoverSampler.h"
//# include "../../Data/TertiaryStructure.h"

namespace lbcpp
{

class UnfoldProteinsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(context, true);

    context.informationCallback(T("It works..."));

//    File referenceFile = context.getFile(referenceDirectory);
//    if (!referenceFile.exists())
//    {
//      context.errorCallback(T("References' directory not found."));
//      return Variable();
//    }
//    File outputFile = context.getFile(outputDirectory);
//    if (!outputFile.exists())
//    {
//      outputFile.createDirectory();
//    }
//
//    juce::OwnedArray<File> references;
//    referenceFile.findChildFiles(references, File::findFiles, false, T("*.xml"));
//
//    context.enterScope(T("Finding best movers"));
//
//    for (int j = 0; j < references.size(); j++)
//    {
//      ProteinPtr proteinRef = Protein::createFromXml(context, *references[j]);
//      core::pose::PoseOP referencePose;
//      convertProteinToPose(context, proteinRef, referencePose);
//
//      // verbosity
//      context.enterScope((*references[j]).getFileNameWithoutExtension() + T(" discovering movers."));
//      context.resultCallback(T("Energy"), Variable(getConformationScore(referencePose,
//          centroidEnergy)));
//
//      juce::OwnedArray<File> targets;
////      targetFile.findChildFiles(targets, File::findFiles, false,
////          (*references[j]).getFileNameWithoutExtension() + T("*.xml"));
//
//      RandomGeneratorPtr random = new RandomGenerator();
//      for (int i = 0; i < targets.size(); i++)
//      {
//        ProteinPtr proteinTarget = Protein::createFromXml(context, *targets[i]);
//        core::pose::PoseOP targetPose;
//        convertProteinToPose(context, proteinTarget, targetPose);
//
//        // verbosity
//        context.progressCallback(new ProgressionState((double)i, (double)targets.size(),
//            T("Intermediate conformations")));
//        context.enterScope((*targets[i]).getFileNameWithoutExtension()
//            + T(" intermediate conformation"));
//        context.resultCallback(T("Energy"), Variable(getConformationScore(targetPose,
//            centroidEnergy)));
//
//        // choose sampler
//        SamplerPtr moverSampler;
//        if (oneOrAll == 0)
//          moverSampler = objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(
//              targetPose->n_residue()), gaussianSampler(0, 25), gaussianSampler(0, 25));
//        else
//          moverSampler = new ProteinMoverSampler(targetPose->n_residue());
//
//        bool bestLearning = true;
//        if (includeBestMoversInLearning == 0)
//          bestLearning = false;
//
//        std::vector<ProteinMoverPtr> returnMovers(numMoversToKeep);
//
//        // find best movers by EDA
//        DenseDoubleVectorPtr energyMeans = new DenseDoubleVector(numIterations, 0);
//        DenseDoubleVectorPtr qScoreMeans = new DenseDoubleVector(numIterations, 0);
//        DenseDoubleVectorPtr scoreMeans = new DenseDoubleVector(numIterations, 0);
//        ProteinEDAOptimizerPtr opti = new ProteinEDAOptimizer(energyWeight);
//        SamplerPtr out = opti->findBestMovers(context, random, targetPose, referencePose,
//            moverSampler, returnMovers, numIterations, numSamples, numGoodSamples, numMoversToKeep,
//            bestLearning, &energyMeans, &qScoreMeans, &scoreMeans);
//
//        // save gathered data
//        for (size_t k = 0;k < numIterations; k++)
//        {
////          meansAndVariances[k]->push(energyMeans->getValue(k));
////          meansAndVariancesQScore[k]->push(qScoreMeans->getValue(k));
////          meansAndVariancesScores[k]->push(scoreMeans->getValue(k));
//        }
//
//        out->saveToFile(context, File(outputFile.getFullPathName() + T("/")
//            + (*targets[i]).getFileNameWithoutExtension() + T("_sampler.xml")));
//
//        for (int k = 0; k < numMoversToKeep; k++)
//          returnMovers[k]->saveToFile(context, File(outputFile.getFullPathName() + T("/")
//              + (*targets[i]).getFileNameWithoutExtension() + T("_mover_") + String(k) + T(".xml")));
//
//        // verbosity
//        context.leaveScope();
//      }
//      context.progressCallback(new ProgressionState((double)targets.size(), (double)targets.size(),
//          T("Intermediate conformations")));
//      context.leaveScope();
//    }

    context.informationCallback(T("All movers generated."));
    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class UnfoldProteinsWorkUnitClass;

//  String referenceDirectory;
//  String targetDirectory;
//  String outputDirectory;
//  double energyWeight;
//  int numIterations;
//  int numSamples;
//  int includeBestMoversInLearning;
//  int numMoversToKeep;
//  int oneOrAll;
//  int numGoodSamples;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_
