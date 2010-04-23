/*-----------------------------------------.---------------------------------.
| Filename: ProteinBackboneBondSequence.h  | Protein Backbone Bonds          |
| Author  : Francis Maes                   |                                 |
| Started : 23/04/2010 13:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_BACKBONE_BOND_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_BACKBONE_BOND_SEQUENCE_H_

# include "../InferenceData/BondCoordinatesSequence.h"

namespace lbcpp
{

// predeclarations
class ProteinBackboneBondSequence;
typedef ReferenceCountedObjectPtr<ProteinBackboneBondSequence> ProteinBackboneBondSequencePtr;

class ProteinBackboneBond : public Object
{
public:
  BondCoordinates phi;
  BondCoordinates psi;
  BondCoordinates omega;
};

typedef ReferenceCountedObjectPtr<ProteinBackboneBond> ProteinBackboneBondPtr;

class ProteinBackboneBondSequence : public TypedObjectVectorBasedSequence<ProteinBackboneBond>
{
public:
  typedef TypedObjectVectorBasedSequence<ProteinBackboneBond> BaseClass;

  ProteinBackboneBondSequence(size_t proteinLength) : BaseClass(T("BackboneBondSequence"), proteinLength) {}
  ProteinBackboneBondSequence() {}

  bool hasBond(size_t position) const
    {return BaseClass::hasObject(position);}

  ProteinBackboneBondPtr getBond(size_t position) const
    {return BaseClass::get(position).dynamicCast<ProteinBackboneBond>();}

  void setBond(size_t position, ProteinBackboneBondPtr bond)
    {BaseClass::set(position, bond);}

  void clearBond(size_t position)
    {BaseClass::set(position, ObjectPtr());}

/*
  DihedralAnglesPair getAnglesPair(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  DihedralAngle getPhi(size_t index) const
    {return getAnglesPair(index).first;}

  DihedralAngle getPsi(size_t index) const
    {return getAnglesPair(index).second;}

  void setAnglesPair(size_t index, DihedralAngle phi, DihedralAngle psi)
    {jassert(index < elements.size()); elements[index] = DihedralAnglesPair(phi, psi);}*/
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_BACKBONE_BOND_SEQUENCE_H_
