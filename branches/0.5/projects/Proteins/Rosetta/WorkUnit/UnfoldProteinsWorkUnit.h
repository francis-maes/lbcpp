/*-----------------------------------------.---------------------------------.
| Filename: UnfoldProteinsWorkUnit.h       | WorkUnit that unfolds proteins  |
| Author  : Alejandro Marcos Alvarez       | to generate good movers.        |
| Started : 25/11/2011 10:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "../RosettaUtils.h"
# include "../Data/MoverSampler/BlindPoseMoverSampler.h"

namespace lbcpp
{

// define function to evaluate "quality" of the unfolded protein
double evaluateQualityUnfold(const core::pose::PoseOP& pose, double* energy = NULL)
{
#ifdef LBCPP_PROTEIN_ROSETTA

  double quality = 0.0;
  quality = getTotalEnergy(pose, fullAtomEnergy);
  if (energy != NULL)
    *energy = quality;

  quality = 0;
  // so far, these factors make no real sense because dimensions do not match...
  quality -= computeCorrectionFactorForDistances(pose);
  quality -= computeCorrectionFactorForCollisions(pose);
  double mean;
  double min;
  double max;
  SymmetricMatrixPtr distMatrix = createCalphaMatrixDistance(pose, &mean, &min, &max);
  quality += mean * max;
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
    File outputFileStructures = context.getFile(outputDirectoryStructures);
    if (!outputFileStructures.exists())
    {
      outputFileStructures.createDirectory();
    }

    File outputFileMovers = context.getFile(outputDirectoryMovers);
    if (!outputFileMovers.exists())
    {
      outputFileMovers.createDirectory();
    }

    File pdbFile = context.getFile(outputPdb);

    juce::OwnedArray<File> references;
    referenceFile.findChildFiles(references, File::findFiles, false, T("*.xml"));

    context.enterScope(T("Finding best movers"));

    for (int j = 0; j < references.size(); j++)
    {
      // load protein
      ProteinPtr proteinRef = Protein::createFromXml(context, *references[j]);
      core::pose::PoseOP workingPose;
      core::pose::PoseOP tempPose = new core::pose::Pose();

      convertProteinToPose(context, proteinRef, workingPose);
      if (workingPose() == NULL)
        continue;

      double energy = 0.0;
      double qual = evaluateQualityUnfold(workingPose, &energy);
      double tempQual = 0.0;
      int numberSelected = 0;

      // create sampler
      SamplerPtr moverSampler = new BlindPoseMoverSampler(workingPose->n_residue());

      // verbosity
      context.enterScope((*references[j]).getFileNameWithoutExtension() + T(" discovering movers."));
      context.enterScope(T("Iteration"));
      context.resultCallback(T("Iteration number"), Variable((int)0));
      context.resultCallback(T("Energy"), Variable(getTotalEnergy(workingPose, fullAtomEnergy)));
      context.resultCallback(T("Mover"), PoseMoverPtr());
      context.leaveScope(Variable(qual));

      RandomGeneratorPtr random = new RandomGenerator();
      PoseMoverPtr mover;
      ProteinPtr workingProtein;

      for (int i = 0; (i < numIterations) && (numberSelected < maxPairs); i++)
      {
        // verbosity
        context.progressCallback(new ProgressionState((double)i, (double)numIterations,
            T("Iterations")));

        *tempPose = *workingPose;

        // sample mover, apply it to the structure and evaluate mover
        mover = moverSampler->sample(context, random).getObjectAndCast<PoseMover> ();
        mover->move(tempPose);
        tempQual = evaluateQualityUnfold(tempPose, &energy);

        // if good, modify structure, qual, save to trace and to disk
        if (tempQual > averageIncrease * qual)
        {
          qual = tempQual;
          *workingPose = *tempPose;
          numberSelected++;

          // save to trace
          context.enterScope(T("Iteration"));
          context.resultCallback(T("Iteration number"), Variable((int)i));
          context.resultCallback(T("Energy"), Variable(energy));
          context.resultCallback(T("Mover"), mover);
          context.leaveScope(Variable(qual));

          // save to disk
          workingProtein = convertPoseToProtein(context, workingPose);
          workingProtein->saveToXmlFile(context, File(outputFileStructures.getFullPathName()
              + T("/") + (*references[j]).getFileNameWithoutExtension() + T("_") + String(i)
              + T(".xml")));
          if (pdbFile != File::nonexistent)
            workingProtein->saveToPDBFile(context, File(pdbFile.getFullPathName() + T("/")
                + (*references[j]).getFileNameWithoutExtension() + T("_") + String(i) + T(".pdb")));

          PoseMoverPtr toSave = mover->getOpposite();
          toSave->saveToFile(context, File(outputFileMovers.getFullPathName() + T("/")
              + (*references[j]).getFileNameWithoutExtension() + T("_") + String(i) + T("_mover")
              + T(".xml")));
        }
      }
      context.progressCallback(new ProgressionState((double)numIterations, (double)numIterations,
          T("Interations")));
      context.leaveScope(Variable(numberSelected));
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
  String outputDirectoryStructures;
  String outputDirectoryMovers;
  String outputPdb;
  int numIterations;
  int maxPairs;
  double averageIncrease;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_UNFOLDPROTEINSWORKUNIT_H_
