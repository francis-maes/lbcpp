/*-----------------------------------------.---------------------------------.
| Filename: GoActionsPerception.h          | Go Actions Perception           |
| Author  : Francis Maes                   |                                 |
| Started : 25/03/2011 18:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GO_ACTIONS_PERCEPTION_H_
# define LBCPP_SEQUENTIAL_DECISION_GO_ACTIONS_PERCEPTION_H_

# include "GoProblem.h"
# include <lbcpp/Core/CompositeFunction.h>

namespace lbcpp
{

///////////////////////////////
// Segment Matrix /////////////
///////////////////////////////

extern ClassPtr matrixRegionClass(TypePtr elementType);

class MatrixRegion : public Object
{
public:
  MatrixRegion(TypePtr elementType, size_t index)
    : Object(matrixRegionClass(elementType)), index(index), size(0) {}
  MatrixRegion() : index(0), size(0) {}

  size_t getIndex() const
    {return index;}

  void setValue(const Variable& value)
    {this->value = value;}

  const Variable& getValue() const
    {return value;}

  void addPosition(const std::pair<size_t, size_t>& position)
  {
    positions.insert(position);
    ++size;
  }

  void addNeighboringElement(const Variable& value, size_t count = 1)
    {neighboringElements[value] += count;}

  size_t getNumNeighboringElement(const Variable& value) const
  {
    std::map<Variable, size_t>::const_iterator it = neighboringElements.find(value);
    return it == neighboringElements.end() ? 0 : it->second;
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class MatrixRegionClass;

  size_t index;
  Variable value;
  size_t size;
  std::set<impl::PositiveIntegerPair> positions;
  std::map<Variable, size_t> neighboringElements; // value -> num connections
};

typedef ReferenceCountedObjectPtr<MatrixRegion> MatrixRegionPtr;

extern ClassPtr segmentedMatrixClass(TypePtr elementsType);

class GetMatrixRegionNumNeighboringElementsFunction : public SimpleBinaryFunction
{
public:
  GetMatrixRegionNumNeighboringElementsFunction()
    : SimpleBinaryFunction(matrixRegionClass(variableType), variableType, positiveIntegerType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const MatrixRegionPtr& region = inputs[0].getObjectAndCast<MatrixRegion>();
    const Variable& value = inputs[1];
    return Variable(region->getNumNeighboringElement(value), positiveIntegerType);
  }
};

class SegmentedMatrix : public BuiltinTypeMatrix<size_t>
{
public:
  typedef BuiltinTypeMatrix<size_t> BaseClass;

  SegmentedMatrix(TypePtr elementsType, size_t numRows, size_t numColumns)
    : BaseClass(segmentedMatrixClass(elementsType), numRows, numColumns, (size_t)-1), elementsType(elementsType)
  {
    BaseClass::elementsType = positiveIntegerType;
  }
  SegmentedMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BaseClass::setElement(row, column, (size_t)value.getInteger());}

  virtual void setElement(size_t index, const Variable& value)
    {BaseClass::setElement(index, (size_t)value.getInteger());}

  bool hasElement(size_t row, size_t column) const
    {return elements[makeIndex(row, column)] != (size_t)-1;}

  bool hasElement(const std::pair<size_t, size_t>& position) const
    {return elements[makeIndex(position.first, position.second)] != (size_t)-1;}

  MatrixRegionPtr startRegion(const Variable& value)
  {
    size_t index = regions.size();
    MatrixRegionPtr res = new MatrixRegion(elementsType, index);
    res->setValue(value);
    regions.push_back(res);
    return res;
  }

  void addToCurrentRegion(const std::pair<size_t, size_t>& position)
  {
    jassert(regions.size());
    MatrixRegionPtr region = regions.back();
    region->addPosition(position);
    BaseClass::setElement(position.first, position.second, region->getIndex());
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class SegmentedMatrixClass;

  TypePtr elementsType; // the input elementsType  /!\ this is different from BaseClass::elementsType whose value is "PositiveInteger"
  std::vector<MatrixRegionPtr> regions;
};

typedef ReferenceCountedObjectPtr<SegmentedMatrix> SegmentedMatrixPtr;

class SegmentedMatrixComponent : public MatrixComponent
{
public:
  SegmentedMatrixComponent(SegmentedMatrixPtr matrix, const String& name)
    : MatrixComponent(matrix) {}
 
  virtual juce::Colour selectColour(const Variable& element)
  {
    if (!element.exists())
      return Colours::lightgrey;

    size_t index = (size_t)element.getInteger();
    if (index >= colours.size())
    {
      size_t oldSize = colours.size();
      colours.resize(index + 1);
      for (size_t i = oldSize; i < colours.size(); ++i)
        colours[i] = randomColour();
    }
    return colours[index];
  }

  inline juce::Colour randomColour()
  {
    RandomGeneratorPtr random = RandomGenerator::getInstance();
    return juce::Colour(random->sampleByte(), random->sampleByte(), random->sampleByte(), (unsigned char)255);
  }

  lbcpp_UseDebuggingNewOperator

private:
  std::vector<juce::Colour> colours;
};

// Matrix<T> -> SegmentedMatrix<T>
class SegmentMatrixFunction : public SimpleUnaryFunction
{
public:
  SegmentMatrixFunction(bool use8Connexity = false)
    : SimpleUnaryFunction(matrixClass(), segmentedMatrixClass(anyType)), use8Connexity(use8Connexity) {}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    elementsType = Container::getTemplateParameter(inputVariables[0]->getType());
    jassert(elementsType);
    return segmentedMatrixClass(elementsType);
  }

  typedef std::pair<size_t, size_t> Position;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const MatrixPtr& matrix = input.getObjectAndCast<Matrix>();

    size_t rows = matrix->getNumRows();
    size_t columns = matrix->getNumColumns();

    SegmentedMatrixPtr res = new SegmentedMatrix(elementsType, rows, columns);

    for (size_t r = 0; r < rows; ++r)
      for (size_t c = 0; c < columns; ++c)
        if (!res->hasElement(r, c))
          makeMatrixSegment(matrix, res,  r, c, rows, columns);

    return Variable(res, getOutputType());
  }

  void makeMatrixSegment(const MatrixPtr& matrix, const SegmentedMatrixPtr& res, size_t row, size_t column, size_t numRows, size_t numColumns) const
  {
    Variable value = matrix->getElement(row, column);
    jassert(value.exists()); // missing values is not supported yet

    MatrixRegionPtr region = res->startRegion(value);
    
    std::set<Position> toExplore;
    std::set<Position> explored;
    toExplore.insert(Position(row, column));
    while (toExplore.size())
    {
      // add current position
      Position position = *toExplore.begin();
      toExplore.erase(toExplore.begin());
      if (explored.find(position) != explored.end())
        continue;
      explored.insert(position);

      res->addToCurrentRegion(position);
      
      // get neighbors
      Position neighbors[8];
      size_t numNeighbors = 0;
      if (position.first > 0)
        neighbors[numNeighbors++] = Position(position.first - 1, position.second);
      if (position.second > 0)
        neighbors[numNeighbors++] = Position(position.first, position.second - 1);
      if (position.first < numRows - 1)
        neighbors[numNeighbors++] = Position(position.first + 1, position.second);
      if (position.second < numColumns - 1)
        neighbors[numNeighbors++] = Position(position.first, position.second + 1);
      if (use8Connexity)
      {
        if (position.first > 0 && position.second > 0)
          neighbors[numNeighbors++] = Position(position.first - 1, position.second - 1);
        if (position.first > 0 && position.second < numColumns - 1)
          neighbors[numNeighbors++] = Position(position.first - 1, position.second + 1);
        if (position.first < numRows - 1 && position.second < numColumns - 1)
          neighbors[numNeighbors++] = Position(position.first + 1, position.second + 1);
        if (position.first < numRows - 1 && position.second > 0)
          neighbors[numNeighbors++] = Position(position.first + 1, position.second - 1);
      }

      size_t numMissingNeighbors = (use8Connexity ? 8 : 4) - numNeighbors;
      region->addNeighboringElement(Variable::missingValue(elementsType), numMissingNeighbors);

      // add neighbors in the toExplore list
      for (size_t i = 0; i < numNeighbors; ++i)
      {
        const Position& neighbor = neighbors[i];
        Variable neighborValue = matrix->getElement(neighbor.first, neighbor.second);
        if (neighborValue == value)
        {
          if (explored.find(neighbor) == explored.end())
            toExplore.insert(neighbor);
        }
        else
          region->addNeighboringElement(neighborValue);
      }
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SegmentMatrixFunctionClass;

  bool use8Connexity;
  TypePtr elementsType;
};

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



///////////////////////////////
// Go Action Features /////////
///////////////////////////////

// GoState -> GoBoard
class GetGoBoardWithCurrentPlayerAsBlack : public SimpleUnaryFunction
{
public:
  GetGoBoardWithCurrentPlayerAsBlack() : SimpleUnaryFunction(goStateClass, goBoardClass, T("Board")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GoStatePtr& state = input.getObjectAndCast<GoState>();
    return Variable(state->getBoardWithCurrentPlayerAsBlack(), outputType);
  }

  lbcpp_UseDebuggingNewOperator
};

class PositiveIntegerPairDistanceFeatureGenerator : public FeatureGenerator
{
public:
  PositiveIntegerPairDistanceFeatureGenerator(size_t firstMax = 0, size_t secondMax = 0, size_t numAxisDistanceIntervals = 10, size_t numDiagDistanceIntervals = 10)
    : firstMax(firstMax), secondMax(secondMax), numAxisDistanceIntervals(numAxisDistanceIntervals), numDiagDistanceIntervals(numDiagDistanceIntervals) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return positiveIntegerPairClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    axisDistanceFeatureGenerator = signedNumberFeatureGenerator(softDiscretizedLogNumberFeatureGenerator(0.0, log10(juce::jmax((double)firstMax, (double)secondMax) + 1.0), numAxisDistanceIntervals, false));
    diagDistanceFeatureGenerator = softDiscretizedLogNumberFeatureGenerator(0.0, log10(sqrt((double)(firstMax * firstMax + secondMax * secondMax)) + 1.0), numDiagDistanceIntervals, false);

    if (!axisDistanceFeatureGenerator->initialize(context, doubleType))
      return EnumerationPtr();
    if (!diagDistanceFeatureGenerator->initialize(context, doubleType))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("positiveIntegerPairDistanceFeatures"));
    res->addElementsWithPrefix(context, axisDistanceFeatureGenerator->getFeaturesEnumeration(), T("hor."), T("h."));
    i1 = res->getNumElements();
    res->addElementsWithPrefix(context, axisDistanceFeatureGenerator->getFeaturesEnumeration(), T("ver."), T("v."));
    i2 = res->getNumElements();
    res->addElementsWithPrefix(context, diagDistanceFeatureGenerator->getFeaturesEnumeration(), T("diag."), T("d."));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const PositiveIntegerPairPtr& pair1 = inputs[0].getObjectAndCast<PositiveIntegerPair>();
    const PositiveIntegerPairPtr& pair2 = inputs[1].getObjectAndCast<PositiveIntegerPair>();
    double x1 = (double)pair1->getFirst();
    double y1 = (double)pair1->getSecond();
    double x2 = (double)pair2->getFirst();
    double y2 = (double)pair2->getSecond();
    double dx = x2 - x1;
    double dy = y2 - y1;

    Variable h(dx);
    callback.sense(0, axisDistanceFeatureGenerator, &h, 1.0);

    Variable v(dy);
    callback.sense(i1, axisDistanceFeatureGenerator, &v, 1.0);

    Variable d(sqrt(dx * dx + dy * dy));
    callback.sense(i2, diagDistanceFeatureGenerator, &d, 1.0);
  }

  lbcpp_UseDebuggingNewOperator

private:
  size_t firstMax;
  size_t secondMax;
  size_t numAxisDistanceIntervals;
  size_t numDiagDistanceIntervals;

  FeatureGeneratorPtr axisDistanceFeatureGenerator;
  FeatureGeneratorPtr diagDistanceFeatureGenerator;
  size_t i1, i2;
};

class GoStatePreFeatures : public Object
{
public:
  GoStatePtr state;
  PositiveIntegerPairVectorPtr previousActions;
  GoBoardPtr board; // with current player as black
  DoubleVectorPtr globalPrimaryFeatures; // time
  SegmentedMatrixPtr fourConnexityGraph;
  VectorPtr fourConnexityGraphFeatures;   // region id -> features
  //SegmentedMatrixPtr eightConnexityGraph;
  //VectorPtr eightConnexityGraphFeatures;  // region id -> features
  MatrixPtr actionPrimaryFeatures;        // position -> features

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<GoStatePreFeatures> GoStatePreFeaturesPtr;

extern ClassPtr goStatePreFeaturesClass(TypePtr globalFeaturesEnumeration, TypePtr regionFeaturesEnumeration, TypePtr actionFeaturesEnumeration);
////////////////////

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


// GoState, Container[GoAction] -> Container[DoubleVector]
class GoActionsPerception : public CompositeFunction
{
public:
  GoActionsPerception(size_t boardSize = 19)
    : boardSize(boardSize)
  {
  }
 
  /*
  ** Cached features
  */
  bool isPassAction(const PositiveIntegerPairPtr& a) const
    {return a->getFirst() == boardSize && a->getSecond() == boardSize;}

  Variable getPositionFeatures(ExecutionContext& context, const Variable& action) const
  {
    const PositiveIntegerPairPtr& a = action.getObjectAndCast<PositiveIntegerPair>();
    if (isPassAction(a))
      return Variable::missingValue(positionFeatures->getElementsType());
    return positionFeatures->getElement(a->getFirst() * boardSize + a->getSecond());
  }

  Variable getRelativePositionFeatures(ExecutionContext& context, const Variable& previousAction, const Variable& currentAction) const
  {
    const PositiveIntegerPairPtr& a1 = previousAction.getObjectAndCast<PositiveIntegerPair>();
    const PositiveIntegerPairPtr& a2 = currentAction.getObjectAndCast<PositiveIntegerPair>();
    if (isPassAction(a1) || isPassAction(a2))
      return Variable::missingValue(relativePositionFeatures->getElementsType());

    size_t i = a1->getFirst() * boardSize + a1->getSecond();
    size_t j = a2->getFirst() * boardSize + a2->getSecond();
    return relativePositionFeatures->getElement(i, j);
  }

  /*
  ** State global features
  */
  void globalPrimaryFeatures(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(goStateClass, T("state"));
    size_t time = builder.addFunction(getVariableFunction(T("time")), state);

    // time
    FeatureGeneratorPtr timeFeatureGenerator = softDiscretizedLogNumberFeatureGenerator(0.0, log10(300.0), 15, true);
    timeFeatureGenerator->setLazy(false);
    builder.addFunction(timeFeatureGenerator, time, T("time"));
  }

  /*
  ** Region features
  */
  //MatrixRegion<Player> -> DoubleVector
  void regionFeatures(CompositeFunctionBuilder& builder)
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

  // SegmentedMatrix<Player> -> Vector<DoubleVector>
  void segmentedBoardFeatures(CompositeFunctionBuilder& builder)
  {
    size_t segmentedBoard = builder.addInput(segmentedMatrixClass(playerEnumeration), T("segmentedBoard"));
    size_t regions = builder.addFunction(getVariableFunction(T("regions")), segmentedBoard);
    builder.addFunction(mapContainerFunction(lbcppMemberCompositeFunction(GoActionsPerception, regionFeatures)), regions);
  }

  /*
  ** State related computation
  */
  void preFeaturesFunction(CompositeFunctionBuilder& builder)
  {
    std::vector<size_t> variables;
    size_t state = builder.addInput(goStateClass, T("state"));
    variables.push_back(state);

    // previous actions
    size_t previousActions = builder.addFunction(getVariableFunction(T("previousActions")), state, T("previousActions"));
    variables.push_back(previousActions);

    // board with black as current
    size_t board = builder.addFunction(new GetGoBoardWithCurrentPlayerAsBlack(), state, T("board"));
    variables.push_back(board);

    // global features
    size_t globalFeatures = builder.addFunction(lbcppMemberCompositeFunction(GoActionsPerception, globalPrimaryFeatures), state);
    EnumerationPtr globalFeaturesEnumeration = DoubleVector::getElementsEnumeration(builder.getOutputType());
    variables.push_back(globalFeatures);

    // region features
    size_t fourConnexityGraph = builder.addFunction(new SegmentMatrixFunction(true), board);
    variables.push_back(fourConnexityGraph);
    size_t fourConnexityGraphFeatures = builder.addFunction(lbcppMemberCompositeFunction(GoActionsPerception, segmentedBoardFeatures), fourConnexityGraph);
    variables.push_back(fourConnexityGraphFeatures);

    /*
    size_t eightConnexityGraph = builder.addFunction(new SegmentMatrixFunction(true), board);
    variables.push_back(eightConnexityGraph);
    size_t eightConnexityGraphFeatures = builder.addFunction(lbcppMemberCompositeFunction(GoActionsPerception, segmentedBoardFeatures), eightConnexityGraph);
    variables.push_back(eightConnexityGraphFeatures);*/
    EnumerationPtr regionFeaturesEnumeration = DoubleVector::getElementsEnumeration(Container::getTemplateParameter(builder.getOutputType()));

    // board primary features
    size_t boardPrimaryFeatures = builder.addFunction(mapContainerFunction(enumerationFeatureGenerator(false)), board);
    EnumerationPtr actionFeaturesEnumeration = DoubleVector::getElementsEnumeration(Container::getTemplateParameter(builder.getOutputType()));
    jassert(actionFeaturesEnumeration);
    variables.push_back(boardPrimaryFeatures);

    builder.addFunction(createObjectFunction(goStatePreFeaturesClass(globalFeaturesEnumeration, regionFeaturesEnumeration, actionFeaturesEnumeration)), variables, T("goPreFeatures"));
  }

  GoStatePreFeaturesPtr computePreFeatures(ExecutionContext& context, const GoStatePtr& state) const
  {
    FunctionPtr fun = lbcppMemberCompositeFunction(GoActionsPerception, preFeaturesFunction);
    return fun->compute(context, state).getObjectAndCast<GoStatePreFeatures>();
  }

  /*
  ** Per-action computations
  */
  void actionFeatures(CompositeFunctionBuilder& builder)
  {
    size_t action = builder.addInput(positiveIntegerPairClass, T("action"));
    size_t preFeatures = builder.addInput(goStatePreFeaturesClass(enumValueType, enumValueType, enumValueType), T("preFeatures"));

    size_t previousActions = builder.addFunction(getVariableFunction(T("previousActions")), preFeatures);
    size_t globalPrimaryFeatures = builder.addFunction(getVariableFunction(T("globalPrimaryFeatures")), preFeatures, T("globals"));

    // matrices:
    size_t region4 = builder.addFunction(getVariableFunction(T("fourConnexityGraph")), preFeatures);
    size_t region4Features = builder.addFunction(getVariableFunction(T("fourConnexityGraphFeatures")), preFeatures);
    //size_t region8 = builder.addFunction(getVariableFunction(T("eightConnexityGraph")), preFeatures);
    //size_t region8Features = builder.addFunction(getVariableFunction(T("eightConnexityGraphFeatures")), preFeatures);
    size_t actionPrimaryFeatures = builder.addFunction(getVariableFunction(T("actionPrimaryFeatures")), preFeatures);

    size_t row = builder.addFunction(getVariableFunction(1), action);
    size_t column = builder.addFunction(getVariableFunction(0), action);

    FunctionPtr fun = lbcppMemberBinaryFunction(GoActionsPerception, getRelativePositionFeatures, positiveIntegerPairClass, positiveIntegerPairClass,
                                                relativePositionFeatures->getElementsType());
    size_t previousActionRelationFeatures = builder.addFunction(mapContainerFunction(fun), previousActions, action);
    
    builder.startSelection();

      size_t i1 = builder.addFunction(matrixWindowFeatureGenerator(5, 5), actionPrimaryFeatures, row, column, T("window"));

      fun = lbcppMemberUnaryFunction(GoActionsPerception, getPositionFeatures, positiveIntegerPairClass, positionFeatures->getElementsType());
      size_t i2 = builder.addFunction(fun, action, T("position"));

      size_t i3 = builder.addFunction(fixedContainerWindowFeatureGenerator(0, 10), previousActionRelationFeatures, T("previousAction"));
      //size_t i32 = builder.addFunction(cartesianProductFeatureGenerator(), i3, i3, T("prev2"));

      FunctionPtr fun2 = new MatrixNeighborhoodFeatureGenerator(new MatrixConnectivityFeatureGenerator(), getElementFunction());
      size_t i4 = builder.addFunction(fun2, region4, row, column, region4Features, T("neighbors"));
      //size_t i42 = builder.addFunction(dynamicallyMappedFeatureGenerator(cartesianProductFeatureGenerator(), 1000000, true), i4, i4, T("neighbors2"));

    size_t features = builder.finishSelectionWithFunction(concatenateFeatureGenerator(false), T("f"));

    //size_t featuresAndTime = builder.addFunction(cartesianProductFeatureGenerator(true), features, globalPrimaryFeatures, T("wt"));
    //builder.addFunction(concatenateFeatureGenerator(true), features, featuresAndTime);
  }

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    ExecutionContext& context = builder.getContext();

    context.enterScope(T("Pre-calculating position features"));
    precalculatePositionFeatures(context);
    context.leaveScope();
    context.enterScope(T("Pre-calculating position pair features"));
    precalculateRelativePositionFeatures(context);
    context.leaveScope();

    size_t state = builder.addInput(goStateClass, T("state"));
    size_t availableActions = builder.addInput(containerClass(positiveIntegerPairClass), T("actions"));
    size_t preFeatures = builder.addFunction(lbcppMemberCompositeFunction(GoActionsPerception, preFeaturesFunction), state);
    
    builder.addFunction(mapContainerFunction(lbcppMemberCompositeFunction(GoActionsPerception, actionFeatures)), availableActions, preFeatures);
  }

private:
  size_t boardSize;

  /*
  ** Pre-calculated position features
  */
  ContainerPtr positionFeatures;

  void precalculatePositionFeaturesFunction(CompositeFunctionBuilder& builder)
  {
    size_t p = builder.addInput(positiveIntegerPairClass, T("position"));
    size_t x = builder.addFunction(getVariableFunction(T("first")), p);
    size_t y = builder.addFunction(getVariableFunction(T("second")), p);
    size_t fx = builder.addFunction(softDiscretizedNumberFeatureGenerator(0.0, (double)boardSize, 7, true), x);
    size_t fy = builder.addFunction(softDiscretizedNumberFeatureGenerator(0.0, (double)boardSize, 7, true), y);
    builder.addFunction(cartesianProductFeatureGenerator(false), fx, fy, T("positionFeatures"));
  }

  bool precalculatePositionFeatures(ExecutionContext& context)
  {
    FunctionPtr function = lbcppMemberCompositeFunction(GoActionsPerception, precalculatePositionFeaturesFunction);
    if (!function->initialize(context, (TypePtr)positiveIntegerPairClass))
      return false;

    size_t n = boardSize * boardSize;
    positionFeatures = vector(function->getOutputType(), n);
    for (size_t i = 0; i < n; ++i)
    {
      PositiveIntegerPairPtr pos(new PositiveIntegerPair(i / boardSize, i % boardSize));
      positionFeatures->setElement(i, function->compute(context, pos));
    }
    return true;
  }

  /*
  ** Pre-calculated relative position features
  */
  SymmetricMatrixPtr relativePositionFeatures;

  void precalculateRelativePositionFeaturesFunction(CompositeFunctionBuilder& builder)
  {
    size_t p1 = builder.addInput(positiveIntegerPairClass, T("position1"));
    size_t p2 = builder.addInput(positiveIntegerPairClass, T("position2"));

    builder.startSelection();

      builder.addFunction(new PositiveIntegerPairDistanceFeatureGenerator(boardSize, boardSize), p1, p2, T("dist"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false), T("relativePos"));
  }

  bool precalculateRelativePositionFeatures(ExecutionContext& context)
  {
    FunctionPtr function = lbcppMemberCompositeFunction(GoActionsPerception, precalculateRelativePositionFeaturesFunction);
    if (!function->initialize(context, positiveIntegerPairClass, positiveIntegerPairClass))
      return false;

    size_t n = boardSize * boardSize;
    relativePositionFeatures = symmetricMatrix(function->getOutputType(), boardSize * boardSize);
    for (size_t i = 0; i < n; ++i)
    {
      PositiveIntegerPairPtr a1(new PositiveIntegerPair(i / boardSize, i % boardSize));
      for (size_t j = i; j < n; ++j)
      {
        PositiveIntegerPairPtr a2(new PositiveIntegerPair(j / boardSize, j % boardSize));
        relativePositionFeatures->setElement(i, j, function->compute(context, a1, a2));
      }
    }
    return true;
  }
};

typedef ReferenceCountedObjectPtr<GoActionsPerception> GoActionsPerceptionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GO_ACTIONS_PERCEPTION_H_
