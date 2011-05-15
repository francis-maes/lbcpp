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
  ProteinBackboneBond(const BondCoordinates& bond1, const BondCoordinates& bond2, const BondCoordinates& bond3)
    : bond1(bond1), bond2(bond2), bond3(bond3) {}
  ProteinBackboneBond() {}

  virtual String toString() const
    {return T("Bond1: ") + bond1.toString() + T("\n") +
            T("Bond2: ") + bond2.toString() + T("\n") +
            T("Bond3: ") + bond3.toString() + T("\n");}

  BondCoordinates getBond1() const
    {return bond1;}

  BondCoordinates& getBond1()
    {return bond1;}

  BondCoordinates getBond2() const
    {return bond2;}

  BondCoordinates& getBond2()
    {return bond2;}

  BondCoordinates getBond3() const
    {return bond3;}

  BondCoordinates& getBond3()
    {return bond3;}

  bool exists() const
    {return bond1.exists() || bond2.exists() || bond3.exists();}

  DihedralAngle getPhiAngle() const
    {return bond1.getPhiDihedralAngle();}

  DihedralAngle getPsiAngle() const
    {return bond2.getPhiDihedralAngle();}

  DihedralAngle getOmegaAngle() const
    {return bond3.getPhiDihedralAngle();}

private:
  BondCoordinates bond1; // N--CA
  BondCoordinates bond2; // CA--C
  BondCoordinates bond3; // C--N

  virtual bool load(InputStream& istr)
    {return bond1.load(istr) && bond2.load(istr) && bond3.load(istr);}

  virtual void save(OutputStream& ostr) const
    {bond1.save(ostr); bond2.save(ostr); bond3.save(ostr);}
};

typedef ReferenceCountedObjectPtr<ProteinBackboneBond> ProteinBackboneBondPtr;

class ProteinBackboneBondSequence : public TypedObjectVectorBasedSequence<ProteinBackboneBond>
{
public:
  typedef TypedObjectVectorBasedSequence<ProteinBackboneBond> BaseClass;

  ProteinBackboneBondSequence(size_t size) : BaseClass(T("BackboneBondSequence"), size) {}
  ProteinBackboneBondSequence() {}
  
  bool hasBond(size_t position) const
    {return BaseClass::hasObject(position);}

  ProteinBackboneBondPtr getBond(size_t position) const
    {return BaseClass::get(position).dynamicCast<ProteinBackboneBond>();}

  void setBond(size_t position, ProteinBackboneBondPtr bond)
    {BaseClass::setElement(position, bond);}

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const;

  DihedralAngle getPhi(size_t index) const
  {
    ProteinBackboneBondPtr bond = getBond(index);
    return bond ? bond->getPhiAngle().normalized() : DihedralAngle();
  }

  DihedralAngle getPsi(size_t index) const
  {
    ProteinBackboneBondPtr bond = getBond(index);
    return bond ? bond->getPsiAngle().normalized() : DihedralAngle();
  }
  
  DihedralAngle getOmega(size_t index) const
  {
    ProteinBackboneBondPtr bond = getBond(index);
    return bond ? bond->getOmegaAngle().normalized() : DihedralAngle();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_BACKBONE_BOND_SEQUENCE_H_