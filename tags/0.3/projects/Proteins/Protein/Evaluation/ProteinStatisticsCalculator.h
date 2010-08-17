/*-----------------------------------------.---------------------------------.
| Filename: ProteinStatisticsCalculator.h  | Compute general statistics about|
| Author  : Francis Maes                   |  a set of proteins              |
| Started : 22/04/2010 21:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATION_STATISTICS_CALCULATOR_H_
# define LBCPP_PROTEIN_EVALUATION_STATISTICS_CALCULATOR_H_

# include "../ProteinObject.h"

namespace lbcpp
{

class ProteinStatisticsCalculator : public ObjectConsumer
{
public:
  ProteinStatisticsCalculator() : numProteins(0),
    aminoAcidsPerProtein(T("Amino Acids / ProteinObject")),
    secondaryStructureElementsPerProtein(T("Secondary Structure / ProteinObject")),
    dsspElementsPerProtein(T("DSSP Secondary Structure / ProteinObject")),
    solventAccesibilityElementsPerProtein(T("Solvent Accesibility / ProteinObject")),
    atomsPerProtein(T("Atoms / ProteinObject")),
    residuesPerProtein(T("Residues / ProteinObject")),
    nCalphaLength(T("N--CA length")),
    calphaCLength(T("CA--C length")), 
    cnLength(T("C--N length")),
    calphaCalphaLength(T("CA--CA length")),
    calphaAngle(T("N--CA--C angle")),
    carbonAngle(T("CA--C--N' angle")),
    nitrogenAngle(T("C--N'--CA' angle"))
    {}

  static String computeStatistics(ObjectContainerPtr proteins)
  {
    ReferenceCountedObjectPtr<ProteinStatisticsCalculator> calculator = new ProteinStatisticsCalculator();
    calculator->consumeContainer(proteins);
    return calculator->toString();
  }

  virtual void consume(ObjectPtr object)
  {
    ProteinObjectPtr protein = object.dynamicCast<ProteinObject>();
    jassert(protein);

    ++numProteins;

    aminoAcidsPerProtein.push(countNumberOfElements(protein->getAminoAcidSequence()));
    secondaryStructureElementsPerProtein.push(countNumberOfElements(protein->getSecondaryStructureSequence()));
    dsspElementsPerProtein.push(countNumberOfElements(protein->getDSSPSecondaryStructureSequence()));
    solventAccesibilityElementsPerProtein.push(countNumberOfElements(protein->getNormalizedSolventAccessibilitySequence()));

    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    residuesPerProtein.push(countNumberOfElements(tertiaryStructure));

    size_t numAtoms = 0;
    if (tertiaryStructure)
      for (size_t i = 0; i < tertiaryStructure->size(); ++i)
      {
        ProteinResidueAtomsPtr residue = tertiaryStructure->getResidue(i);
        if (residue)
          numAtoms += residue->getNumAtoms();
      }
    atomsPerProtein.push(numAtoms);
  }

  static size_t countNumberOfElements(SequencePtr sequence)
  {
    if (!sequence)
      return 0;
    size_t n = sequence->size();
    size_t res = 0;
    for (size_t i = 0; i < n; ++i)
      if (sequence->get(i) != ObjectPtr())
        ++res;
    return res;
  }

  /*
      for (size_t i = 0; i < tertiaryStructure->size(); ++i)
    {
      ProteinResidueAtomsPtr residue = tertiaryStructure->getResidue(i);

      Vector3 nitrogen = residue->getNitrogenAtom()->getPosition();
      Vector3 calpha = residue->getCAlphaAtom()->getPosition();
      Vector3 carbon = residue->getCarbonAtom()->getPosition();

      nCalphaLength.push((calpha - nitrogen).l2norm());
      calphaCLength.push((carbon - calpha).l2norm());
      ProteinResidueAtomsPtr nextResidue = i < tertiaryStructure->size() - 1 ? tertiaryStructure->getResidue(i + 1) : ProteinResidueAtomsPtr();
      if (nextResidue)
      {
        Vector3 nextNitrogen = nextResidue->getNitrogenAtom()->getPosition();
        cnLength.push((nextNitrogen - carbon).l2norm());

        Vector3 nextCalpha = nextResidue->getCAlphaAtom()->getPosition();
        calphaCalphaLength.push((nextCalpha - calpha).l2norm());

        carbonAngle.push((carbon - calpha).angle(nextNitrogen - carbon));
        nitrogenAngle.push((nextNitrogen - carbon).angle(nextCalpha - nextNitrogen));
      }

      calphaAngle.push((calpha - nitrogen).angle(carbon - calpha));
    }
    */

  virtual String toString() const
  {
    String res = T("Statistics on ") + lbcpp::toString(numProteins) + T(" protein(s)\n");
    res += countsPerProteinToString(aminoAcidsPerProtein);
    res += countsPerProteinToString(secondaryStructureElementsPerProtein);
    res += countsPerProteinToString(dsspElementsPerProtein);
    res += countsPerProteinToString(solventAccesibilityElementsPerProtein);
    res += countsPerProteinToString(atomsPerProtein);
    res += countsPerProteinToString(residuesPerProtein);
    return res;
  }

  static String countsPerProteinToString(const ScalarVariableStatistics& stats)
    {return stats.getName() + T(" average = ") + String(stats.getMean()) + T(" total = ") + String(stats.getSum()) + T("\n");}

private:
  size_t numProteins;

  ScalarVariableStatistics aminoAcidsPerProtein;
  ScalarVariableStatistics secondaryStructureElementsPerProtein;
  ScalarVariableStatistics dsspElementsPerProtein;
  ScalarVariableStatistics solventAccesibilityElementsPerProtein;
  ScalarVariableStatistics atomsPerProtein;
  ScalarVariableStatistics residuesPerProtein;

  ScalarVariableStatistics nCalphaLength;
  ScalarVariableStatistics calphaCLength;
  ScalarVariableStatistics cnLength;
  ScalarVariableStatistics calphaCalphaLength;

  ScalarVariableStatistics calphaAngle;
  ScalarVariableStatistics carbonAngle;
  ScalarVariableStatistics nitrogenAngle;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATION_STATISTICS_CALCULATOR_H_
