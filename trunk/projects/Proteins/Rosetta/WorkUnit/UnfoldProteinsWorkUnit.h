/*-----------------------------------------.---------------------------------.
| Filename: UnfoldProteinsWorkUnit.h       | WorkUnit that unfolds proteins  |
| Author  : Alejandro Marcos Alvarez       | to generate good movers.        |
| Started : 25/11/2011 10:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_

# include "precompiled.h"
# include "../RosettaUtils.h"
# include "../Sampler/ProteinMoverSampler.h"

namespace lbcpp
{

// define function to evaluate "quality" of the unfolded protein
double evaluateQualityUnfold(const core::pose::PoseOP& pose)
{
#ifdef LBCPP_PROTEIN_ROSETTA

  double quality = getTotalEnergy(pose, fullAtomEnergy);
  quality -= computeCorrectionFactorForDistances(pose);
  quality -= computeCorrectionFactorForCollisions(pose);
  // so far, makes no real sense because dimensions do not match...
  quality += computeCompactness(pose);
  return quality;

#else
  jassert(false);
  return 0.0;
#endif //LBCPP_PROTEIN_ROSETTA
}


class UnfoldProteinsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA

    rosettaInitialization(context, true);

    File referenceFile = context.getFile(referenceDirectory);
    if (!referenceFile.exists())
    {
      context.errorCallback(T("References' directory not found."));
      return Variable();
    }
    File outputFile = context.getFile(outputDirectory);
    if (!outputFile.exists())
    {
      outputFile.createDirectory();
    }

    juce::OwnedArray<File> references;
    referenceFile.findChildFiles(references, File::findFiles, false, T("*.xml"));

    context.enterScope(T("Finding best movers"));

    for (int j = 0; j < references.size(); j++)
    {
      // load protein
      ProteinPtr proteinRef = Protein::createFromXml(context, *references[j]);
      core::pose::PoseOP workingPose;
      convertProteinToPose(context, proteinRef, workingPose);
      double qual = evaluateQualityUnfold(workingPose);
      double tempQual = 0.0;

      // create sampler
      SamplerPtr moverSampler;
      moverSampler = new ProteinMoverSampler(workingPose->n_residue());

      // verbosity
      context.enterScope((*references[j]).getFileNameWithoutExtension() + T(" discovering movers."));

      RandomGeneratorPtr random = new RandomGenerator();
      ProteinMoverPtr mover;
      ProteinPtr workingProtein;

      for (int i = 0; i < numIterations; i++)
      {
        // verbosity
        context.progressCallback(new ProgressionState((double)i, (double)numIterations,
            T("Iterations")));

        core::pose::PoseOP tempPose = new core::pose::Pose();
        *tempPose = *workingPose;

        // sample mover, apply it to the structure and evaluate mover
        mover = moverSampler->sample(context, random).getObjectAndCast<ProteinMover> ();
        mover->move(tempPose);
        tempQual = evaluateQualityUnfold(tempPose);

        // if good, modify structure, qual, save to trace and to disk
        if (tempQual > qual)
        {
          qual = tempQual;
          *workingPose = *tempPose;

          // save to trace
          context.enterScope(T("iteration"));
          context.resultCallback(T("Iteration number"), Variable((int)i));
          context.resultCallback(T("Quality"), Variable(qual));
          context.resultCallback(T("Mover"), mover);
          context.leaveScope();

          // save to disk
          workingProtein = convertPoseToProtein(context, workingPose);
          workingProtein->saveToXmlFile(context, File(outputFile.getFullPathName() + T("/")
              + (*references[j]).getFileNameWithoutExtension() + T("_") + String(i) + T(".xml")));
          mover->saveToFile(context, File(outputFile.getFullPathName() + T("/")
              + (*references[i]).getFileNameWithoutExtension() + T("_") + String(i) + T("_mover")
              + T(".xml")));
        }
      }
      context.progressCallback(new ProgressionState((double)numIterations, (double)numIterations,
          T("Interations")));
      context.leaveScope();
    }

    context.leaveScope();
    context.informationCallback(T("All movers generated."));
    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class UnfoldProteinsWorkUnitClass;

  String referenceDirectory;
  String outputDirectory;
  int numIterations;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_
