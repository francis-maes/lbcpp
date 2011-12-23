/*-----------------------------------------.---------------------------------.
| Filename: ProteinResiduePerception.h     | Protein Residue Perception      |
| Author  : Francis Maes                   |                                 |
| Started : 23/12/2011 12:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_RESIDUE_PERCEPTION_H_
# define LBCPP_PROTEINS_LUAPE_RESIDUE_PERCEPTION_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class ProteinPerception;
typedef ReferenceCountedObjectPtr<ProteinPerception> ProteinPerceptionPtr;
class ProteinResiduePerception;
typedef ReferenceCountedObjectPtr<ProteinResiduePerception> ProteinResiduePerceptionPtr;
class ProteinResiduePairPerception;
typedef ReferenceCountedObjectPtr<ProteinResiduePairPerception> ProteinResiduePairPerceptionPtr;

class ProteinResiduePerception : public Object
{
public:
  ProteinResiduePerception(const ProteinPerceptionPtr& proteinPerception, const ProteinPtr& protein, size_t index)
    : protein(proteinPerception), position(index), positionOfType((size_t)-1)
  {
    aminoAcidType = (AminoAcidType)protein->getPrimaryStructure()->getElement(index).getInteger();
    pssmRow = getDenseDoubleVector(protein->getPositionSpecificScoringMatrix(), index);
    secondaryStructure = getDenseDoubleVector(protein->getSecondaryStructure(), index);
    dsspSecondaryStructure = getDenseDoubleVector(protein->getDSSPSecondaryStructure(), index);
    structuralAlphabet = getDenseDoubleVector(protein->getStructuralAlphabetSequence(), index);
    solventAccessibilityAt20p = getProbability(protein->getSolventAccessibilityAt20p(), index);
    disordered = getProbability(protein->getDisorderRegions(), index);
  }
  ProteinResiduePerception() : position((size_t)-1), positionOfType((size_t)-1), aminoAcidType(totalNumAminoAcids), solventAccessibilityAt20p(doubleMissingValue), disordered(doubleMissingValue) {}

  void setPrevious(const ProteinResiduePerceptionPtr& previous)
  {
    this->previous = previous;
    previous->next = this;
  }

  void setPreviousOfType(const ProteinResiduePerceptionPtr& previous)
  {
    this->previousOfType = previous;
    previous->nextOfType = this;
  }
  
  void setPositionOfType(size_t position)
    {positionOfType = position;}

  const ProteinPerceptionPtr& getProtein() const
    {return protein;}

  size_t getPosition() const
    {return position;}

  AminoAcidType getAminoAcidType() const
    {return aminoAcidType;}

  virtual size_t getSizeInBytes() const
    {return sizeof (*this);}

protected:
  friend class ProteinResiduePerceptionClass;

  ProteinPerceptionPtr protein;
  size_t position;

  ProteinResiduePerceptionPtr previous;
  ProteinResiduePerceptionPtr next;

  ProteinResiduePerceptionPtr previousOfType;
  ProteinResiduePerceptionPtr nextOfType;
  size_t positionOfType;

  AminoAcidType aminoAcidType;
  DenseDoubleVectorPtr pssmRow;
  DenseDoubleVectorPtr secondaryStructure;
  DenseDoubleVectorPtr dsspSecondaryStructure;
  DenseDoubleVectorPtr structuralAlphabet;
  double solventAccessibilityAt20p;
  double disordered;

  static DenseDoubleVectorPtr getDenseDoubleVector(const ContainerPtr& container, size_t index)
  {
    if (!container)
      return DenseDoubleVectorPtr();
    jassert(index < container->getNumElements());
    DoubleVectorPtr res = container->getElement(index).getObjectAndCast<DoubleVector>();
    if (!res)
      return DenseDoubleVectorPtr();
    return res->toDenseDoubleVector();
  }

  static double getProbability(const ContainerPtr& container, size_t index)
  {
    if (!container)
      return doubleMissingValue;
    jassert(index < container->getNumElements());
    return container.staticCast<DenseDoubleVector>()->getValue(index);
  }
};

extern ClassPtr proteinResiduePerceptionClass;

class ProteinResiduePairPerception : public Object
{
public:
  ProteinResiduePairPerception(const ProteinPerceptionPtr& protein, const ProteinResiduePerceptionPtr& firstResidue, const ProteinResiduePerceptionPtr& secondResidue)
    : protein(protein), firstResidue(firstResidue), secondResidue(secondResidue) {}
  ProteinResiduePairPerception() {}

  virtual size_t getSizeInBytes() const
    {return sizeof (*this);}

protected:
  friend class ProteinResiduePairPerceptionClass;

  ProteinPerceptionPtr protein;
  ProteinResiduePerceptionPtr firstResidue;
  ProteinResiduePerceptionPtr secondResidue;
};

extern ClassPtr proteinResiduePairPerceptionClass;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_RESIDUE_PERCEPTION_H_
