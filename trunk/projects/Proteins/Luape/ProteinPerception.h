/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 23/12/2011 12:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_PERCEPTION_H_
# define LBCPP_PROTEINS_LUAPE_PERCEPTION_H_

# include "ProteinResiduePerception.h"
# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

class ProteinPerception : public Object
{
public:
  ProteinPerception(const ProteinPtr& protein)
  {
    // create residues
    size_t n = protein->getLength();
    residues = new ObjectVector(proteinResiduePerceptionClass, n);

    std::vector<ProteinResiduePerceptionPtr> lastResiduesByType(totalNumAminoAcids);
    std::vector<size_t> countByType(totalNumAminoAcids, 0);
    for (size_t i = 0; i < n; ++i)
    {
      ProteinResiduePerceptionPtr residue(new ProteinResiduePerception(this, protein, i));
      residues->set(i, residue);
      if (i > 0)
        residue->setPrevious(residues->getAndCast<ProteinResiduePerception>(i - 1));
      
      ProteinResiduePerceptionPtr& last = lastResiduesByType[residue->getAminoAcidType()];
      if (last)
        residue->setPreviousOfType(last);
      last = residue;

      size_t& positionByType = countByType[residue->getAminoAcidType()];
      positionByType++;
      residue->setPositionOfType(positionByType);
    }

    // cysteins
    cysteinIndices = protein->getCysteinIndices();
  }
  ProteinPerception() {}

  size_t getNumResidues() const
    {return residues->getNumElements();}

  const ObjectVectorPtr& getResidues() const
    {return residues;}

  const ProteinResiduePerceptionPtr& getResidue(size_t index) const
    {return residues->getAndCast<ProteinResiduePerception>(index);}

  const std::vector<size_t>& getCysteinIndices() const
    {return cysteinIndices;}

  virtual size_t getSizeInBytes(bool recursively) const
    {return sizeof (*this);}
 
protected:
  friend class ProteinPerceptionClass;

  ObjectVectorPtr residues;
  std::vector<size_t> cysteinIndices;
};

extern ClassPtr proteinPerceptionClass;

class ProteinGetRelativeResidueLuapeFunction : public LuapeFunction
{
public:
  ProteinGetRelativeResidueLuapeFunction(int delta = 0)
    : delta(delta) {}

  virtual String toShortString() const
    {return "[" + String(delta) + "]";}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type == proteinResiduePerceptionClass;}

  virtual bool acceptInputsStack(const std::vector<LuapeNodePtr>& stack) const
  {
    if (!LuapeFunction::acceptInputsStack(stack))
      return false;
    LuapeFunctionNodePtr functionNode = stack[0].dynamicCast<LuapeFunctionNode>();
    if (functionNode && functionNode->getFunction().isInstanceOf<ProteinGetRelativeResidueLuapeFunction>())
      return false; // cannot chain two calls to this function
    return true;
  }

  virtual TypePtr initialize(const std::vector<TypePtr>& )
    {return proteinResiduePerceptionClass;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(".neighbor[") + String(delta) + T("]");}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinResiduePerceptionPtr res = compute(inputs[0].getObjectAndCast<ProteinResiduePerception>());
    return Variable(res, proteinResiduePerceptionClass);
  }

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    const LuapeSampleVectorPtr& objects = inputs[0];
    size_t n = objects->size();
    ObjectVectorPtr res = new ObjectVector(outputType, n);
    size_t i = 0;
    for (LuapeSampleVector::const_iterator it = objects->begin(); it != objects->end(); ++it, ++i)
      res->set(i, compute(it.getRawObject()));
    return new LuapeSampleVector(objects->getIndices(), res);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    enum {windowHalfSize = 7};

    VectorPtr res = vector(integerType, windowHalfSize * 2);
    for (int i = -windowHalfSize; i < windowHalfSize; ++i)
      res->setElement((size_t)(i + windowHalfSize), i < 0 ? i : i + 1);
    return res;
  }

  int getDelta() const
    {return delta;}

protected:
  friend class ProteinGetRelativeResidueLuapeFunctionClass;

  int delta;

  ProteinResiduePerceptionPtr compute(const ProteinResiduePerceptionPtr& residue) const
  {
    if (!residue)
      return ProteinResiduePerceptionPtr();
    const ProteinPerceptionPtr& protein = residue->getProtein();
    int position = (int)residue->getPosition() + delta;
    if (position < 0 || position >= (int)protein->getNumResidues())
      return ProteinResiduePerceptionPtr();
    return protein->getResidue((size_t)position);
  }
};

typedef ReferenceCountedObjectPtr<ProteinGetRelativeResidueLuapeFunction> ProteinGetRelativeResidueLuapeFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_PERCEPTION_H_
