/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.h             | Class for protein inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_H_
# define LBCPP_PROTEIN_INFERENCE_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class ProteinToInputOutputPairFunction : public ObjectFunction
{
public:
  virtual String getOutputClassName(const String& inputClassName) const
    {return T("ObjectPair");}

  virtual ObjectPtr function(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    protein->computeMissingFields();
    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setObject(protein->getAminoAcidSequence());
    inputProtein->setObject(protein->getPositionSpecificScoringMatrix());
    return new ObjectPair(inputProtein, protein);
  }
};

class ProteinInference : public VectorSequentialInference
{
public:
  ProteinInference() : VectorSequentialInference(T("Protein"))
    {}

  void setPDBDebugDirectory(const File& directory)
    {pdbDebugDirectory = directory;}

  // child inference are all of the form
  // Protein -> Protein sub object
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    // working proteins
    ProteinPtr workingProtein = input.dynamicCast<Protein>();
    jassert(workingProtein);
    
    // supervision
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
    if (pdbDebugDirectory.exists() && correctProtein && correctProtein->getTertiaryStructure())
      correctProtein->saveToPDBFile(pdbDebugDirectory.getChildFile(correctProtein->getName() + T("_correct.pdb")));

    // main inference loop
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      InferencePtr inferenceStep = subInferences[i];

      ObjectPtr inferenceOutput = context->runInference(inferenceStep, workingProtein, correctProtein, returnCode);
      jassert(inferenceOutput);
      //std::cout << "INFERED: " << inferenceOutput->toString() << std::endl;
      workingProtein = addObjectToProtein(workingProtein, inferenceOutput, correctProtein);
      
      if (pdbDebugDirectory.exists() &&  workingProtein->getTertiaryStructure())
        workingProtein->saveToPDBFile(pdbDebugDirectory.getChildFile
          (workingProtein->getName() + T("_pred") + lbcpp::toString(i) + T(".pdb")));

      if (returnCode != finishedReturnCode)
        break;
    }

    // return the last version of the working protein
    jassert(workingProtein);
    return workingProtein;
  }

  ProteinPtr addObjectToProtein(ProteinPtr workingProtein, ObjectPtr newObject, ProteinPtr correctProtein)
  {
    if (newObject.dynamicCast<Protein>())
      return newObject.dynamicCast<Protein>(); // when a whole protein is predicted, it replaces the current protein

    // we have to clone the protein, so that feature generators to may be called later keep the correct versions of their input objects
    ProteinPtr res = workingProtein->clone();
    res->setObject(newObject);
    res->setVersionNumber(workingProtein->getVersionNumber() + 1);

    LabelSequencePtr aminoAcids = res->getAminoAcidSequence();
    jassert(aminoAcids);

    ProteinTertiaryStructurePtr tertiaryStructure = newObject.dynamicCast<ProteinTertiaryStructure>();
    if (tertiaryStructure)
    {
      // we do not fill amino acids in tertiary structure predictors, so do it now
      size_t n = aminoAcids->size();
      for (size_t i = 0; i < n; ++i)
      {
        ProteinResiduePtr residue = tertiaryStructure->getResidue(i);
        if (residue)
          residue->setAminoAcid((AminoAcidDictionary::Type)aminoAcids->getIndex(i));
      }

      // when a tertiary structure is predicted, update the backbone bond representation automatically
      res->setObject(tertiaryStructure->makeBackbone());
    }

    // when a backbone is predicted, update the tertiary structure automatically
    BondCoordinatesSequencePtr calphaBondSequence = newObject.dynamicCast<BondCoordinatesSequence>();
    CartesianCoordinatesSequencePtr calphaTrace = newObject.dynamicCast<CartesianCoordinatesSequence>();
    if (calphaBondSequence && calphaBondSequence->getName() == T("CAlphaBondSequence"))
      res->setObject(calphaTrace = calphaBondSequence->makeCartesianCoordinates(T("CAlphaTrace")));

    ProteinBackboneBondSequencePtr backbone = newObject.dynamicCast<ProteinBackboneBondSequence>();
    if (backbone)
      res->setObject(tertiaryStructure = ProteinTertiaryStructure::createFromBackbone(aminoAcids, backbone));
  
    // when a c-alpha trace is predicted, update the tertiary structure automatically
    if (calphaTrace)
      res->setObject(tertiaryStructure = ProteinTertiaryStructure::createFromCAlphaTrace(aminoAcids, calphaTrace));

    /*
    // if we have access to the correct tertiary structure and if we have freshly created a tertiary structure
    // superpose our structure to the correct one.
    // Thisway, when performing tertiary structure refinement, the input, output and supervision tertiary structures
    // are all in the same referential.
    if (tertiaryStructure && correctProtein)
    {
      ProteinTertiaryStructurePtr correctTertiaryStructure = correctProtein->getTertiaryStructure();
      if (correctTertiaryStructure)
      {
        Matrix4 transformation = tertiaryStructure->superposeCAlphaAtoms(correctTertiaryStructure);
        tertiaryStructure->applyAffineTransform(transformation);
        calphaTrace = workingProtein->getCAlphaTrace();
        if (calphaTrace)
          calphaTrace->applyAffineTransform(transformation);
      }
    }*/
    return res;
  }

private:
  File pdbDebugDirectory;
};

typedef ReferenceCountedObjectPtr<ProteinInference> ProteinInferencePtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
