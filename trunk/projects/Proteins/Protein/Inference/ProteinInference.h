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
  virtual String getName() const
    {return T("ProteinToInputOutputPair");}

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
    inputProtein->setObject(protein->getReducedAminoAcidAlphabetSequence());
    return new ObjectPair(protein->getName(), inputProtein, protein);
  }
};
  
class ComputeMissingFieldsOfProteinPairFunction : public ObjectFunction
{
public:
  virtual String getName() const
    {return T("ComputeMissingFieldsOfProteinPair");}
  
  virtual String getOutputClassName(const String& inputClassName) const
    {return T("ObjectPair");}
  
  virtual ObjectPtr function(ObjectPtr input) const
  {
    ObjectPairPtr proteinPair = input.dynamicCast<ObjectPair>();
    jassert(proteinPair);
    proteinPair->getFirst().dynamicCast<Protein>()->computeMissingFields();
    proteinPair->getSecond().dynamicCast<Protein>()->computeMissingFields();
    return input;
  }
};

// Prototype:
//   Input: Protein
//   Supervision: Protein
//   Output: Protein

// Sub-inferences prototype:
//   Input: Protein
//   Supervision: Protein
//   Output: Protein Object or Protein

class ProteinInference : public VectorSequentialInference
{
public:
  ProteinInference() : VectorSequentialInference(T("Protein"))
    {setBatchLearner(sequentialInferenceLearner());}

  virtual ObjectPtr prepareInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
  {
    if (state->getSupervision())
    {
      ProteinPtr correctProtein = state->getSupervision().dynamicCast<Protein>();
      jassert(correctProtein);
      correctProtein->computeMissingFields();
      if (pdbDebugDirectory.exists() && correctProtein->getTertiaryStructure())
        correctProtein->saveToPDBFile(pdbDebugDirectory.getChildFile(correctProtein->getName() + T("_correct.pdb")));
      if (proteinDebugDirectory.exists())
        correctProtein->saveToFile(proteinDebugDirectory.getChildFile(correctProtein->getName() + T("_correct.protein")));
    }
    return state->getInput();
  }
  
  virtual Variable finalizeSubInference(SequentialInferenceStatePtr state, const Variable& subInferenceOutput, ReturnCode& returnCode) const
  {
    if (!subInferenceOutput)
      return state->getCurrentObject(); // skip empty predictions

    ProteinPtr workingProtein = state->getCurrentObject().dynamicCast<Protein>();
    jassert(workingProtein);
    workingProtein = addObjectToProtein(workingProtein, subInferenceOutput, state->getSupervision().dynamicCast<Protein>());

    if (pdbDebugDirectory.exists() &&  workingProtein->getTertiaryStructure())
      workingProtein->saveToPDBFile(pdbDebugDirectory.getChildFile
        (workingProtein->getName() + T("_pred") + lbcpp::toString(state->getCurrentStepNumber()) + T(".pdb")));

    if (proteinDebugDirectory.exists())
      workingProtein->saveToFile(proteinDebugDirectory.getChildFile
        (workingProtein->getName() + T("_pred") + lbcpp::toString(state->getCurrentStepNumber()) + T(".protein")));

    return workingProtein;
  }

  void setPDBDebugDirectory(const File& directory)
    {pdbDebugDirectory = directory;}
    
  void setProteinDebugDirectory(const File& directory)
  {
    proteinDebugDirectory = directory;
    if (!directory.exists())
      directory.createDirectory();
  }

private:
  File pdbDebugDirectory;
  File proteinDebugDirectory;

  static ProteinPtr addObjectToProtein(ProteinPtr workingProtein, ObjectPtr newObject, ProteinPtr correctProtein)
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
        ProteinResidueAtomsPtr residue = tertiaryStructure->getResidue(i);
        if (residue)
          residue->setAminoAcid((AminoAcidDictionary::Type)aminoAcids->getIndex(i));
      }

      // when a tertiary structure is predicted, update backbone bond and calpha trace automatically
      res->setObject(tertiaryStructure->makeBackbone());
      res->setObject(tertiaryStructure->makeCAlphaTrace());
      return res;
    }

    // when a backbone is predicted, update the tertiary structure automatically
    BondCoordinatesSequencePtr calphaBondSequence = newObject.dynamicCast<BondCoordinatesSequence>();
    CartesianCoordinatesSequencePtr calphaTrace = newObject.dynamicCast<CartesianCoordinatesSequence>();
    if (calphaBondSequence && calphaBondSequence->getName() == T("CAlphaBondSequence"))
      res->setObject(calphaTrace = calphaBondSequence->makeCartesianCoordinates(T("CAlphaTrace")));

    ProteinBackboneBondSequencePtr backbone = newObject.dynamicCast<ProteinBackboneBondSequence>();
    if (backbone)
    {
      res->setObject(tertiaryStructure = ProteinTertiaryStructure::createFromBackbone(aminoAcids, backbone));
      res->setObject(tertiaryStructure->makeCAlphaTrace());
    }
  
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
};

typedef ReferenceCountedObjectPtr<ProteinInference> ProteinInferencePtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_PREDICTION_PROBLEM_H_
