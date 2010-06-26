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

class Residue : public NameableObject
{
public:
  Residue(const String& name, AminoAcidPtr aminoAcid)
    : NameableObject(name), aminoAcid(aminoAcid) {}

  AminoAcidPtr getAminoAcid() const
    {return aminoAcid;}

  AminoAcidType getAminoAcidType() const
    {jassert(aminoAcid); return aminoAcid->getType();}

  virtual Variable getVariable(size_t index) const;

  void setNext(ResiduePtr next)
    {this->next = next; next->previous = ResiduePtr(this);}

  ResiduePtr getPrevious() const
    {return previous;}

  ResiduePtr getNext() const
    {return next;}

  void clear()
    {previous = next = ResiduePtr();}

private:
  AminoAcidPtr aminoAcid;
  ResiduePtr previous;
  ResiduePtr next;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_RESIDUE_H_
