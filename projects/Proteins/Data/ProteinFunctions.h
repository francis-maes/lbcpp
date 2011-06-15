/*-----------------------------------------.---------------------------------.
| Filename: ProteinFunctions.h             | Protein Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 12:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DATA_FUNCTIONS_H_
# define LBCPP_PROTEIN_DATA_FUNCTIONS_H_

# include <lbcpp/Core/Function.h>
# include "Protein.h"

namespace lbcpp
{

/*
** ProteinToInputOutputPairFunction
*/
class ProteinToInputOutputPairFunction : public SimpleUnaryFunction
{
public:
  ProteinToInputOutputPairFunction(bool keepTertiaryStructure = true)
    : SimpleUnaryFunction(proteinClass, pairClass(proteinClass, proteinClass), T("IO")), keepTertiaryStructure(keepTertiaryStructure) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    if (!keepTertiaryStructure)
    {
      protein->getDisorderRegions(); // be sure that disordered regions are computed
      protein->getStructuralAlphabetSequence(); // be sure that structural alphabet is computed
      protein->getDisulfideBonds(context);

      protein->setTertiaryStructure(TertiaryStructurePtr()); // remove tertiary structure
      protein->setCAlphaTrace(CartesianPositionVectorPtr()); // remove c-alpha trace
      protein->setDistanceMap(SymmetricMatrixPtr(), false);
      protein->setDistanceMap(SymmetricMatrixPtr(), true);
    }

    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    return Variable::pair(inputProtein, protein, outputType);
  }

protected:
  friend class ProteinToInputOutputPairFunctionClass;

  bool keepTertiaryStructure;
};

// protein, (targetIndex, targetValue)* -> protein
class MakeProteinFunction : public Function
{
public:
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)proteinClass : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("protein");
    outputShortName = T("prot");
    return proteinClass;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinPtr inputProtein = inputs[0].getObjectAndCast<Protein>();
    if (!inputProtein)
      return Variable::missingValue(proteinClass);

    ProteinPtr res = inputProtein->cloneAndCast<Protein>();
    size_t numInputs = getNumInputs();
    for (size_t i = 1; i < numInputs; i += 2)
    {
      size_t targetIndex = (size_t)inputs[i].getInteger();
      const Variable& target = inputs[i + 1];
      if (target.exists())
      {
        jassert(targetIndex < proteinClass->getNumMemberVariables());
        res->setVariable(targetIndex, target);
      }
    }
    
    return res;
  }
};

// protein -> variable
class GetProteinTargetFunction : public SimpleUnaryFunction
{
public:
  GetProteinTargetFunction(ProteinTarget target = noTarget)
    : SimpleUnaryFunction(proteinClass, proteinClass->getMemberVariableType(target), T("Target")), target(target) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>();
    if (protein)
      return protein->getTargetOrComputeIfMissing(context, target);
    else
      return Variable::missingValue(getOutputType());
  }

protected:
  friend class GetProteinTargetFunctionClass;

  ProteinTarget target;
};

class GreedyDisulfidePatternBuilder : public SimpleUnaryFunction
{
public:
  GreedyDisulfidePatternBuilder(size_t numOfRuns = 1, double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : SimpleUnaryFunction(symmetricMatrixClass(probabilityType), symmetricMatrixClass(probabilityType), T("PatternBuilder"))
    , numOfRuns(numOfRuns), threshold(threshold), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    SymmetricMatrixPtr matrix = input.getObjectAndCast<SymmetricMatrix>(context);
    if (!matrix)
      return Variable::missingValue(getOutputType());
    
    const size_t dimension = matrix->getDimension();
    if (dimension <= minimumDistanceFromDiagonal)
      return matrix;
    
    SortedScoresMap sortedScores;
    insertScoreToMap(matrix, sortedScores);
    SymmetricMatrixPtr mask;
    findBestMask(context, matrix, sortedScores, mask);

    SymmetricMatrixPtr res = makeResult(context, matrix, mask);
    return res;
  }

protected:
  typedef std::multimap<double, std::pair<size_t, size_t> > SortedScoresMap;
  enum {notBridged = -2, bridged = -1};
  
  friend class GreedyDisulfidePatternBuilderClass;
  
  size_t numOfRuns;
  double threshold;
  size_t minimumDistanceFromDiagonal;

  void insertScoreToMap(const SymmetricMatrixPtr& matrix, SortedScoresMap& sortedScores) const
  {
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        const double value = matrix->getElement(i, j).getDouble();
        sortedScores.insert(std::make_pair(value, std::make_pair(i, j)));
      }
  }
  
  void findBestMask(ExecutionContext& context, const SymmetricMatrixPtr& matrix, const SortedScoresMap& sortedScores, SymmetricMatrixPtr& bestMask) const
  {
    double bestScore = -DBL_MAX;
    const size_t n = sortedScores.size() < numOfRuns ? sortedScores.size() : numOfRuns;
    SortedScoresMap::const_reverse_iterator it = sortedScores.rbegin();
    for (size_t i = 0; i < n; ++i, it++)
    {
      SymmetricMatrixPtr mask = matrix->cloneAndCast<SymmetricMatrix>(context);
      // force first edge
      double score = (it->first > threshold) ? it->first : 0.f;
      updateMatrix(mask, it->second.first, it->second.second, score > threshold);
      // find the other edges
      while (true)
      {
        size_t i = (size_t)-1;
        size_t j = (size_t)-1;
        double value = findBestValue(mask, i, j);
        
        if (value < 0.f)
          break;
        updateMatrix(mask, i, j, value > threshold);
        score += (value > threshold) ? value : 0.f;
      }
      // keep best score
      if (score > bestScore)
      {
        bestScore = score;
        bestMask = mask;
      }
    }
    jassert(bestMask);
  }

  double findBestValue(const SymmetricMatrixPtr& resultMatrix, size_t& bestI, size_t& bestJ) const
  {
    double bestValue = -DBL_MAX;
    const size_t dimension = resultMatrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        const double value = resultMatrix->getElement(i, j).getDouble();
        if (value > bestValue)
        {
          bestI = i;
          bestJ = j;
          bestValue = value;
        }
      }
    return bestValue;
  }

  void updateMatrix(const SymmetricMatrixPtr& matrix, size_t x, size_t y, bool isConnected) const
  {
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension; ++i)
    {
      matrix->setElement(x, i, probability((double)notBridged));
      matrix->setElement(y, i, probability((double)notBridged));
    }
    if (isConnected)
      matrix->setElement(x, y, probability((double)bridged));
  }
  
  SymmetricMatrixPtr makeResult(ExecutionContext& context, const SymmetricMatrixPtr& matrix, const SymmetricMatrixPtr& mask) const
  {
    SymmetricMatrixPtr res = matrix->cloneAndCast<SymmetricMatrix>(context);
    
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
        if (mask->getElement(i, j).getDouble() == (double)notBridged)
          res->setElement(i, j, probability(0.f));
    
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DATA_FUNCTIONS_H_
