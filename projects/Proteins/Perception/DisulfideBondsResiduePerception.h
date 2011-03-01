/*-----------------------------------------.---------------------------------.
| Filename: DisulfideBondsResiduePercep...h| Disulfide Bonds seen from a     |
| Author  : Francis Maes                   |  cysteine residue               |
| Started : 01/11/2010 00:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_DISULFIDE_BONDS_RESIDUE_H_
# define LBCPP_PROTEIN_PERCEPTION_DISULFIDE_BONDS_RESIDUE_H_

# include "ProteinPerception.h"
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

class DisulfideBondsResiduePairPerception : public Perception
{
public:
  DisulfideBondsResiduePairPerception()
    {computeOutputType();}

  virtual TypePtr getInputType() const
    {return pairClass(proteinClass, pairClass(positiveIntegerType, positiveIntegerType));}

  virtual void computeOutputType()
  {
    addOutputVariable(T("probability"), probabilityType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    const ProteinPtr& protein = pair->getFirst().getObjectAndCast<Protein>(context);
    const PairPtr& positionPair = pair->getSecond().getObjectAndCast<Pair>(context);

     // check that we are a pair of cysteins
    int cysteinIndex1 = protein->getCysteinInvIndices()[positionPair->getFirst().getInteger()];
    int cysteinIndex2 = protein->getCysteinInvIndices()[positionPair->getSecond().getInteger()];
    if (cysteinIndex1 < 0 || cysteinIndex2 < 0)
      return;

    // check that disulfide bonds exists
    const SymmetricMatrixPtr& disulfideBonds = protein->getDisulfideBonds(context);    
    if (!disulfideBonds)
      return;

    callback->sense(0, disulfideBonds->getElement(cysteinIndex1, cysteinIndex2));
    // todo: rank1, margin1 and rank2, margin2
  }
};

class DisulfideBondsResiduePerception : public Perception
{
public:
  DisulfideBondsResiduePerception()
    {computeOutputType();}

  virtual TypePtr getInputType() const
    {return pairClass(proteinClass, positiveIntegerType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("bondsCount"), positiveIntegerType);
    addOutputVariable(T("veryLowCount"), positiveIntegerType);
    addOutputVariable(T("lowCount"), positiveIntegerType);
    addOutputVariable(T("mediumCount"), positiveIntegerType);
    addOutputVariable(T("highCount"), positiveIntegerType);
    addOutputVariable(T("veryHighCount"), positiveIntegerType);
    addOutputVariable(T("probMin"), probabilityType);
    addOutputVariable(T("probMax"), probabilityType);
    addOutputVariable(T("probMean"), probabilityType);
    addOutputVariable(T("probStddev"), probabilityType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    const ProteinPtr& protein = pair->getFirst().getObjectAndCast<Protein>(context);
    size_t position = (size_t)pair->getSecond().getInteger();

    // check that we are a cystein
    int cysteinIndex = protein->getCysteinInvIndices()[position];
    if (cysteinIndex < 0)
      return;

    // check that disulfide bonds exists
    const SymmetricMatrixPtr& disulfideBonds = protein->getDisulfideBonds(context);    
    if (!disulfideBonds)
      return;
  
    // compute
    size_t n = disulfideBonds->getDimension();
    size_t counts[5];
    memset(counts, 0, sizeof (size_t) * 5);
    
    ScalarVariableStatistics stats;
    for (size_t i = 0; i < n; ++i)
    {
      static const double epsilon = 1e-6;

      Variable v = disulfideBonds->getElement((size_t)cysteinIndex, i);
      if (!v.exists())
        continue;
      double p = v.getDouble();
      if (fabs(p) < epsilon)
        continue;

      stats.push(p);
      size_t confidence = (size_t)(p * 5.0);
      if (confidence == 5)
        confidence = 4;
      counts[confidence]++;
    }

    // here comes the perception
    callback->sense(0, (size_t)stats.getCount());
    for (size_t i = 0; i < 5; ++i)
      callback->sense(i + 1, counts[i]);
    callback->sense(6, stats.getMinimum());
    callback->sense(7, stats.getMaximum());
    callback->sense(8, stats.getMean());
    callback->sense(9, stats.getStandardDeviation());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_DISULFIDE_BONDS_RESIDUE_H_
