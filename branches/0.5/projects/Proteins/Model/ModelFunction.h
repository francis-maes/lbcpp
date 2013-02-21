/*-----------------------------------------.---------------------------------.
| Filename: ModelFunction.h                | Model Function                  |
| Author  : Julien Becker                  |                                 |
| Started : 15/02/2013 21:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_MODEL_FUNCTION_H_
# define LBCPP_PROTEIN_MODEL_FUNCTION_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class NumOfEachResidueFunction : public SimpleUnaryFunction
{
public:
  NumOfEachResidueFunction()
  : SimpleUnaryFunction(proteinClass,
                        denseDoubleVectorClass(standardAminoAcidTypeEnumeration, doubleType),
                        T("NumOfEachResidue")) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    if (!protein)
      return Variable::missingValue(positiveIntegerType);
    const size_t numStdAA = standardAminoAcidTypeEnumeration->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(standardAminoAcidTypeEnumeration, doubleType);
    const VectorPtr& ps = protein->getPrimaryStructure();
    const size_t n = ps->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const Variable v = ps->getElement(i);
      if (v.isMissingValue())
        continue;
      size_t aaIndex = ps->getElement(i).getInteger();
      if (aaIndex >= numStdAA)
        continue; // This is one of the non-standard AA
      res->getValueReference(aaIndex)++;
    }
    return res;
  }
};

class EnsureProteinTargetIsComputedFunction : public SimpleUnaryFunction
{
public:
  EnsureProteinTargetIsComputedFunction(ProteinTarget target = noTarget)
  : SimpleUnaryFunction(proteinClass, nilType, T("ComputeTarget")),
    target(target) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    protein->getTargetOrComputeIfMissing(context, target);
    return Variable();
  }

protected:
  friend class EnsureProteinTargetIsComputedFunctionClass;

  ProteinTarget target;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_MODEL_FUNCTION_H_
