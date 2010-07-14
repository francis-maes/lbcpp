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
    ProteinObjectPtr protein = input.dynamicCast<ProteinObject>();
    jassert(protein);
    protein->computeMissingFields();
    ProteinObjectPtr inputProtein = new ProteinObject(protein->getName());
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
    proteinPair->getFirst().dynamicCast<ProteinObject>()->computeMissingFields();
    proteinPair->getSecond().dynamicCast<ProteinObject>()->computeMissingFields();
    return input;
  }
};

// Prototype:
//   Input: ProteinObject
//   Supervision: ProteinObject
//   Output: ProteinObject

// Sub-inferences prototype:
//   Input: ProteinObject
//   Supervision: ProteinObject
//   Output: ProteinObject Object or ProteinObject

class ProteinInference : public VectorSequentialInference
{
public:
  ProteinInference() : VectorSequentialInference(T("ProteinObject"))
    {}

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      ProteinObjectPtr correctProtein = supervision.dynamicCast<ProteinObject>();
      jassert(correctProtein);
      correctProtein->computeMissingFields();
      if (pdbDebugDirectory.exists() && correctProtein->getTertiaryStructure())
        correctProtein->saveToPDBFile(pdbDebugDirectory.getChildFile(correctProtein->getName() + T("_correct.pdb")));
      if (proteinDebugDirectory.exists())
        correctProtein->saveToFile(proteinDebugDirectory.getChildFile(correctProtein->getName() + T("_correct.protein")));
    }
    return VectorSequentialInference::prepareInference(context, input, supervision, returnCode);
  }

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
  {
    // we keep the same input and supervision for sub-inferences
    /*ProteinObjectPtr workingProtein = state->getUserVariable().getObjectAndCast<ProteinObject>();
    jassert(workingProtein);
    state->setSubInference(subInferences.get(index), workingProtein, state->getSupervision());*/
    jassert(false); 
  }

  virtual void finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
  {
    if (state->getSubOutput())
    {
/*      ProteinObjectPtr workingProtein = state->getUserVariable().dynamicCast<ProteinObject>();
      jassert(workingProtein);
      workingProtein = addObjectToProtein(workingProtein, state->getSubOutput(), state->getSupervision().dynamicCast<ProteinObject>());

      if (pdbDebugDirectory.exists() &&  workingProtein->getTertiaryStructure())
        workingProtein->saveToPDBFile(pdbDebugDirectory.getChildFile
          (workingProtein->getName() + T("_pred") + lbcpp::toString(state->getStepNumber()) + T(".pdb")));

      if (proteinDebugDirectory.exists())
        workingProtein->saveToFile(proteinDebugDirectory.getChildFile
          (workingProtein->getName() + T("_pred") + lbcpp::toString(state->getStepNumber()) + T(".protein")));

      state->setUserVariable(workingProtein);*/
      jassert(false);
    }
  }

  virtual Variable finalizeInference(InferenceContextPtr context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
  {jassert(false); return Variable();}//return finalState->getUserVariable();} // the working protein

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

  static ProteinObjectPtr addObjectToProtein(ProteinObjectPtr workingProtein, ObjectPtr newObject, ProteinObjectPtr correctProtein)
  {
    if (newObject.dynamicCast<ProteinObject>())
      return newObject.dynamicCast<ProteinObject>(); // when a whole protein is predicted, it replaces the current protein

    // we have to clone the protein, so that feature generators to may be called later keep the correct versions of their input objects
    ProteinObjectPtr res = workingProtein->clone();
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
