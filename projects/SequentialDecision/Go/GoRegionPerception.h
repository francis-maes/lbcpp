/*-----------------------------------------.---------------------------------.
| Filename: GoRegionPerception.h           | Go Region Perception            |
| Author  : Francis Maes                   |                                 |
| Started : 06/04/2011 01:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GO_REGION_PERCEPTION_H_
# define LBCPP_SEQUENTIAL_DECISION_GO_REGION_PERCEPTION_H_

# include "GoProblem.h"
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Data/Matrix.h>

namespace lbcpp
{

///////////////////////////////
// Connectivity Features //////
///////////////////////////////

class SumFeatureGenerator : public Function
{
public:
  SumFeatureGenerator(FeatureGeneratorPtr baseFeatureGenerator)
    : baseFeatureGenerator(baseFeatureGenerator) {}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return sparseDoubleVectorClass(baseFeatureGenerator->getFeaturesEnumeration());}

protected:
  friend class SumFeatureGeneratorClass;

  FeatureGeneratorPtr baseFeatureGenerator;
};

// Matrix[Variable], Row, Column, Arg -> DoubleVector
class MatrixNeighborhoodFeatureGenerator : public SumFeatureGenerator
{
public:
  MatrixNeighborhoodFeatureGenerator(FunctionPtr relationFeatures, FunctionPtr valueFeatures)
    : SumFeatureGenerator(cartesianProductFeatureGenerator()), relationFeatures(relationFeatures), valueFeatures(valueFeatures) {}

  virtual size_t getNumRequiredInputs() const
    {return 4;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)matrixClass() : (index == 3 ? variableType : positiveIntegerType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr matrixElementsType = Container::getTemplateParameter(inputVariables[0]->getType());
    std::vector<VariableSignaturePtr> relationVariables = inputVariables;
    relationVariables.back() = new VariableSignature(matrixElementsType, T("neighbor"));

    if (!relationFeatures->initialize(context, relationVariables))
      return TypePtr();
    if (!valueFeatures->initialize(context, inputVariables[3]->getType(), matrixElementsType))
      return TypePtr();

    if (!baseFeatureGenerator->initialize(context, relationFeatures->getOutputType(), valueFeatures->getOutputType()))
      return TypePtr();

    return SumFeatureGenerator::initializeFunction(context, inputVariables, outputName, outputShortName);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const MatrixPtr& matrix = inputs[0].getObjectAndCast<Matrix>();
    size_t row = (size_t)inputs[1].getInteger();
    size_t column = (size_t)inputs[2].getInteger();
    const Variable& argument = inputs[3];

    std::set<Variable> neighbors;
    for (int r = (int)row - 1; r <= (int)row + 1; ++r)
      for (int c = (int)column - 1; c <= (int)column + 1; ++c)
        if (r >= 0 && c >= 0 && r < (int)matrix->getNumRows() && c < (int)matrix->getNumColumns())
          neighbors.insert(matrix->getElement(r, c));

    if (!neighbors.size())
      return Variable::missingValue(getOutputType());

    SparseDoubleVectorPtr res = new SparseDoubleVector(getOutputType());
    double weight = 1.0 / neighbors.size();
    for (std::set<Variable>::const_iterator it = neighbors.begin(); it != neighbors.end(); ++it)
    {
      DoubleVectorPtr rel = relationFeatures->compute(context, matrix, row, column, *it).getObjectAndCast<DoubleVector>();
      DoubleVectorPtr val = valueFeatures->compute(context, argument, *it).getObjectAndCast<DoubleVector>();
      DoubleVectorPtr relTimesVal = baseFeatureGenerator->compute(context, rel, val).getObjectAndCast<DoubleVector>();
      relTimesVal->addWeightedTo(res, 0, weight);
    }
    return res;
  }

protected:
  FunctionPtr relationFeatures; // Matrix, Row, Column, Value -> DoubleVector 
  FunctionPtr valueFeatures;    // Matrix, Value -> DoubleVector
};

// Matrix[Variable], Row, Column, Variable -> DoubleVector
class MatrixConnectivityFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 4;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)matrixClass() : (index == 3 ? anyType : positiveIntegerType);}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    connectivityPatternsMap.resize(1 << 9, (size_t)-1);

    DefaultEnumerationPtr features = new DefaultEnumeration(T("connectivityFeatures"));

    for (int i = connectivityPatternsMap.size() - 1; i >= 0; --i)
      if (connectivityPatternsMap[i] == (size_t)-1)
      {
        bool values[9];
        makePattern(i, values);
        size_t currentElement = features->getNumElements();
        features->addElement(context, getPatternName(values));
 
        for (size_t j = 0; j < 4; ++j)
        {
          connectivityPatternsMap[getPatternIndex(values)] = currentElement;
          rotatePattern(values);
        }
        mirrorPattern(values);
        for (size_t j = 0; j < 4; ++j)
        {
          connectivityPatternsMap[getPatternIndex(values)] = currentElement;
          rotatePattern(values);
        }
      }

    return features;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const MatrixPtr& matrix = inputs[0].getObjectAndCast<Matrix>();
    size_t row = (size_t)inputs[1].getInteger();
    size_t column = (size_t)inputs[2].getInteger();
    const Variable& neighborValue = inputs[3];

    bool values[9];
    for (int i = 0; i < 9; ++i)
    {
      int r = (int)row + (-1 + (i / 3));
      int c = (int)column + (-1 + (i % 3));
      values[i] = (r >= 0 && r < (int)matrix->getNumRows() && c >= 0 && c < (int)matrix->getNumColumns() && matrix->getElement(r, c) == neighborValue);
    }
    size_t index = getPatternIndex(values);
    callback.sense(connectivityPatternsMap[index], 1.0);
  }

private:
  std::vector<size_t> connectivityPatternsMap;

  static String getPatternName(bool values[9])
  {
    String res;
    for (size_t i = 0; i < 9; ++i)
      if (values[i])
      {
        juce::tchar c = 'a' + i;
        res += c;
      }
    return res.isEmpty() ? T("none") : res;
  }

  static size_t getPatternIndex(bool values[9])
  {
    size_t pattern = 0;
    for (size_t i = 0; i < 9; ++i)
    {
      pattern <<= 1;
      if (values[i])
        ++pattern;
    }
    return pattern;
  }

  static void makePattern(size_t index, bool values[9])
  {
    for (int i = 8; i >= 0; --i)
    {
      values[i] = ((index & 0x1) == 0x1);
      index >>= 1;
    }
  }

  static void rotatePattern(bool values[9])
  {
    bool oldValues[9];
    for (size_t i = 0; i < 9; ++i)
      oldValues[i] = values[i];
    
    values[0] = oldValues[6];
    values[1] = oldValues[3];
    values[2] = oldValues[0];

    values[3] = oldValues[7];
    values[4] = oldValues[4];
    values[5] = oldValues[1];

    values[6] = oldValues[8];
    values[7] = oldValues[5];
    values[8] = oldValues[2];
  }

  static void mirrorPattern(bool values[9])
  {
    std::swap(values[0], values[2]);
    std::swap(values[3], values[5]);
    std::swap(values[6], values[8]);
  }
};

///// GoCount

extern EnumerationPtr discretizedGoCountEnumeration;

class GoCountFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return positiveIntegerType;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {elementsType = probabilityType; return discretizedGoCountEnumeration;}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
    {callback.sense(getEnumValue((size_t)inputs[0].getInteger()), 1.0);}

  static size_t getEnumValue(const size_t value)
  {
    if (value < 5)
      return value;
    if (value < 7)
      return 5;
    if (value < 10)
      return 6;
    if (value < 15)
      return 7;
    if (value < 20)
      return 8;
    return 9;
  }
};

class GetMatrixRegionNumNeighboringElementsFunction : public SimpleBinaryFunction
{
public:
  GetMatrixRegionNumNeighboringElementsFunction()
    : SimpleBinaryFunction(matrixRegionClass(variableType), variableType, positiveIntegerType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const MatrixRegionPtr& region = inputs[0].getObjectAndCast<MatrixRegion>();
    const Variable& value = inputs[1];
    const MatrixPtr& matrix = region->getMatrix();
    if (!matrix)
      return Variable::missingValue(getOutputType());

    typedef MatrixRegion::Position Position;

    size_t res = 0;
    const std::set<Position>& pos = region->getNeighborPositions();
    for (std::set<Position>::const_iterator it = pos.begin(); it != pos.end(); ++it)
    {
      if (matrix->getElement(it->first, it->second) == value)
        ++res;
    }
    return Variable(res, positiveIntegerType);
  }
};

//MatrixRegion<Player> -> DoubleVector
class GoRegionPerception : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t region = builder.addInput(matrixRegionClass(playerEnumeration));
    size_t regionPlayer = builder.addFunction(getVariableFunction(T("value")), region);
    size_t regionSize = builder.addFunction(getVariableFunction(T("size")), region);

    size_t noPlayers = builder.addConstant(Variable(0, playerEnumeration));
    size_t blackPlayer = builder.addConstant(Variable(1, playerEnumeration));
    size_t whitePlayer = builder.addConstant(Variable(2, playerEnumeration));
    size_t outside = builder.addConstant(Variable::missingValue(playerEnumeration));

    size_t noPlayersCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, noPlayers);
    size_t blackPlayerCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, blackPlayer);
    size_t whitePlayerCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, whitePlayer);
    size_t outsideCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, outside);

    size_t i1 = builder.addFunction(enumerationFeatureGenerator(false), regionPlayer);

    builder.startSelection();

      builder.addFunction(new GoCountFeatureGenerator(), regionSize, T("regionSize"));
      builder.addFunction(new GoCountFeatureGenerator(), noPlayersCount, T("noPlayersCount"));
      builder.addFunction(new GoCountFeatureGenerator(), blackPlayerCount, T("blackPlayerCount"));
      builder.addFunction(new GoCountFeatureGenerator(), whitePlayerCount, T("whitePlayerCount"));
      builder.addFunction(new GoCountFeatureGenerator(), outsideCount, T("borderCount"));

    size_t i2 = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true), T("features"));

    builder.addFunction(cartesianProductFeatureGenerator(false), i1, i2);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GO_REGION_PERCEPTION_H_
