/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.cpp   | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinTertiaryStructure.h"
using namespace lbcpp;

/*
** ProteinTertiaryStructure
*/
ProteinTertiaryStructure::ProteinTertiaryStructure(size_t numResidues)
  : Sequence(T("TertiaryStructure")), residues(numResidues) {}

void ProteinTertiaryStructure::set(size_t index, ObjectPtr object)
{
  ProteinResiduePtr residue = object.dynamicCast<ProteinResidue>();
  jassert(residue);
  jassert(index < residues.size());
  residues[index] = residue;
}
