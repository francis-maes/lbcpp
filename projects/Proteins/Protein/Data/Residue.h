/*-----------------------------------------.---------------------------------.
| Filename: Residue.h                      | Protein Residue                 |
| Author  : Francis Maes                   |                                 |
| Started : 28/06/2010 18:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_RESIDUE_H_
# define LBCPP_PROTEINS_RESIDUE_H_

# include "AminoAcid.h"
# include "SecondaryStructure.h"

namespace lbcpp
{

class Residue;
typedef ReferenceCountedObjectPtr<Residue> ResiduePtr;
class Protein;
typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;

class Residue : public Object
{
public:
  Residue(ProteinPtr protein, size_t position);

  virtual String toString() const;
  virtual Variable getVariable(size_t index) const;

  AminoAcidPtr getAminoAcid() const
    {return aminoAcid;}

  AminoAcidType getAminoAcidType() const
    {jassert(aminoAcid); return aminoAcid->getType();}

  ResiduePtr getPrevious() const;
  ResiduePtr getNext() const;

  void clear()
    {protein = ProteinPtr();}

private:
  ProteinPtr protein;
  size_t position;

  AminoAcidPtr aminoAcid;
};

extern ClassPtr residueClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_RESIDUE_H_
