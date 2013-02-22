/*-----------------------------------------.---------------------------------.
| Filename: TertiaryStructure.h            | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 15:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_TERTIARY_STRUCTURE_H_
# define LBCPP_PROTEIN_TERTIARY_STRUCTURE_H_

# include "Residue.h"
# include "../Geometry/CartesianPositionVector.h"
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Data/SymmetricMatrix.h>

namespace lbcpp
{

class TertiaryStructure;
typedef ReferenceCountedObjectPtr<TertiaryStructure> TertiaryStructurePtr;

class TertiaryStructure : public Object
{
public:
  TertiaryStructure(size_t numResidues);
  TertiaryStructure() {}

  static TertiaryStructurePtr createFromCAlphaTrace(VectorPtr primaryStructure, CartesianPositionVectorPtr trace);
  //static TertiaryStructurePtr createFromBackbone(VectorPtr primaryStructure, ProteinBackboneBondSequencePtr backbone);

  VectorPtr makePrimaryStructure() const;
  CartesianPositionVectorPtr makeCAlphaTrace() const;
  CartesianPositionVectorPtr makeCBetaTrace() const;
  //ProteinBackboneBondSequencePtr makeBackbone() const;
  SymmetricMatrixPtr makeCAlphaDistanceMatrix() const;
  SymmetricMatrixPtr makeCBetaDistanceMatrix(ExecutionContext& context) const;
  SymmetricMatrixPtr makeSulfurDistanceMatrix(ExecutionContext& context, const std::vector<size_t>& cysteines) const;

  size_t getNumResidues() const
    {return residues->getNumElements();}

  ResiduePtr getResidue(size_t index) const
    {jassert(index < getNumResidues()); return index < getNumResidues() ? residues->getAndCast<Residue>(index) : ResiduePtr();}

  ResiduePtr getLastResidue() const
    {size_t n = residues->getNumElements(); return n ? getResidue(n - 1) : ResiduePtr();}

  void setResidue(size_t index, ResiduePtr residue)
    {residues->setElement(index, residue);}

  void append(ResiduePtr residue)
    {residues->append(residue);}

  bool hasCompleteBackbone() const;
  bool hasBackboneAndCBetaAtoms() const;
  bool hasCAlphaAtoms() const;

  bool isConsistent(String& failureReason) const;

  void pruneResiduesThatDoNotHaveCompleteBackbone();
  size_t getNumSpecifiedResidues() const;

  impl::Matrix4 superposeCAlphaAtoms(TertiaryStructurePtr targetStructure) const;
  double computeCAlphaAtomsRMSE(TertiaryStructurePtr targetStructure) const;
  size_t computeCAlphaAtomsGDTTS(TertiaryStructurePtr targetStructure, double cutoff) const;
  void applyAffineTransform(const impl::Matrix4& affineTransform) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class TertiaryStructureClass;

  ObjectVectorPtr residues;
};

extern ClassPtr tertiaryStructureClass;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_