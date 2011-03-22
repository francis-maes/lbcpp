/*-----------------------------------------.---------------------------------.
| Filename: GoSandBox.h                    | Go Sand Box                     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_

# include "../Problem/GoProblem.h"
# include "../Problem/LoadSGFFileFunction.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Core/CompositeFunction.h>

namespace lbcpp
{

///////////////////////////////
// More/less generic DP stuff /
///////////////////////////////

  // State -> Container[Action]
class GetAvailableActionsFunction : public SimpleUnaryFunction
{
public:
  GetAvailableActionsFunction(TypePtr actionType)
    : SimpleUnaryFunction(decisionProblemStateClass, containerClass(actionType), T("Actions")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DecisionProblemStatePtr& state = input.getObjectAndCast<DecisionProblemState>();
    return state->getAvailableActions();
  }

  lbcpp_UseDebuggingNewOperator
};

// State, Action -> DoubleVector
// TODO: transform into function FindElementInContainer: 
//     Container<T>, T -> PositiveInteger
// et gerer la supervision avec PositiveInteger dans le Ranking
class DecisionProblemStateActionsRankingCostsFunction : public SimpleBinaryFunction
{
public:
  DecisionProblemStateActionsRankingCostsFunction()
    : SimpleBinaryFunction(decisionProblemStateClass, variableType, denseDoubleVectorClass(positiveIntegerEnumerationEnumeration)) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DecisionProblemStatePtr& state = inputs[0].getObjectAndCast<DecisionProblemState>();
    const Variable& action = inputs[1];

    ContainerPtr availableActions = state->getAvailableActions();
    size_t n = availableActions->getNumElements();

    DenseDoubleVectorPtr res(new DenseDoubleVector(outputType, n, 0.0));
    bool actionFound = false;
    for (size_t i = 0; i < n; ++i)
      if (availableActions->getElement(i) == action)
      {
        res->setValue(i, -1);
        actionFound = true;
      }

    if (!actionFound)
      context.warningCallback(T("Could not find action ") + action.toShortString() + T(" in state ") + state->toShortString());
    return res;
  }

  lbcpp_UseDebuggingNewOperator
};

// State, Supervision Action -> Ranking Example
class DecisionProblemStateActionsRankingExample : public CompositeFunction
{
public:
  DecisionProblemStateActionsRankingExample(FunctionPtr actionsPerception = FunctionPtr())
    : actionsPerception(actionsPerception) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(decisionProblemStateClass, T("state"));
    size_t supervision = builder.addInput(anyType, T("supervision"));
    size_t perceptions = builder.addFunction(actionsPerception, state);
    if (actionsPerception->getOutputType())
    {
      size_t costs = builder.addFunction(new DecisionProblemStateActionsRankingCostsFunction(), state, supervision);
      builder.addFunction(createObjectFunction(pairClass(actionsPerception->getOutputType(), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration))), perceptions, costs);
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DecisionProblemStateActionsRankingExampleClass;

  FunctionPtr actionsPerception; // State -> Container[DoubleVector]
};


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

  void addNeighboringElement(const Variable& value)
    {neighboringElements[value]++;}

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

      // add neighbors in the toExplore list
      for (size_t i = 0; i < numNeighbors; ++i)
      {
        const Position& neighbor = neighbors[i];
        if (explored.find(neighbor) == explored.end())
        {
          Variable neighborValue = matrix->getElement(neighbor.first, neighbor.second);
          if (neighborValue == value)
            toExplore.insert(neighbor);
          else
            region->addNeighboringElement(neighborValue);
        }
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

class SumFeatureGenerator : public FeatureGenerator
{
public:
  SumFeatureGenerator(FeatureGeneratorPtr baseFeatureGenerator)
    : baseFeatureGenerator(baseFeatureGenerator) {}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return baseFeatureGenerator->getFeaturesEnumeration();}

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

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr matrixElementsType = Container::getTemplateParameter(inputVariables[0]->getType());
    std::vector<VariableSignaturePtr> relationVariables = inputVariables;
    relationVariables.back() = new VariableSignature(matrixElementsType, T("neighbor"));

    if (!relationFeatures->initialize(context, relationVariables))
      return false;
    if (!valueFeatures->initialize(context, inputVariables[3]->getType(), matrixElementsType))
      return false;

    if (!baseFeatureGenerator->initialize(context, relationFeatures->getOutputType(), valueFeatures->getOutputType()))
      return false;

    return SumFeatureGenerator::initializeFeatures(context, inputVariables, elementsType, outputName, outputShortName);
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
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
      return;

    ExecutionContext& context = defaultExecutionContext(); // fixme
    DenseDoubleVectorPtr res = new DenseDoubleVector(getFeaturesEnumeration(), doubleType);
    double weight = 1.0 / neighbors.size();
    for (std::set<Variable>::const_iterator it = neighbors.begin(); it != neighbors.end(); ++it)
    {
      DoubleVectorPtr rel = relationFeatures->compute(context, matrix, row, column, *it).getObjectAndCast<DoubleVector>();
      DoubleVectorPtr val = valueFeatures->compute(context, argument, *it).getObjectAndCast<DoubleVector>();
      DoubleVectorPtr relTimesVal = baseFeatureGenerator->compute(context, rel, val).getObjectAndCast<DoubleVector>();
      relTimesVal->addWeightedTo(res, 0, weight);
    }
    const std::vector<double>& values = res->getValues();
    for (size_t i = 0; i < values.size(); ++i)
      if (values[i])
        callback.sense(i, values[i]);
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

// PositiveIntegerPair -> DoubleVector
class PositiveIntegerPairPositionFeature : public FeatureGenerator
{
public:
  PositiveIntegerPairPositionFeature(size_t firstMax = 0, size_t secondMax = 0)
    : firstMax(firstMax), secondMax(secondMax) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return positiveIntegerPairClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("positions"));
    for (size_t i = 0; i < firstMax; ++i)
      for (size_t j = 0; j < secondMax; ++j)
        res->addElement(context, String((int)i) + T(", ") + String((int)j));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const PositiveIntegerPairPtr& action = inputs[0].getObjectAndCast<PositiveIntegerPair>();
    callback.sense(action->getFirst() * secondMax + action->getSecond(), 1.0);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  size_t firstMax;
  size_t secondMax;
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


// GoState -> Container[DoubleVector]
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
  Variable getPositionFeatures(ExecutionContext& context, const Variable& action) const
  {
    const PositiveIntegerPairPtr& a = action.getObjectAndCast<PositiveIntegerPair>();
    return positionFeatures->getElement(a->getFirst() * boardSize + a->getSecond());
  }

  Variable getRelativePositionFeatures(ExecutionContext& context, const Variable& previousAction, const Variable& currentAction) const
  {
    const PositiveIntegerPairPtr& a1 = previousAction.getObjectAndCast<PositiveIntegerPair>();
    const PositiveIntegerPairPtr& a2 = currentAction.getObjectAndCast<PositiveIntegerPair>();
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
    size_t opponentPlayer = builder.addConstant(Variable(2, playerEnumeration));
    size_t outside = builder.addConstant(Variable::missingValue(playerEnumeration));

    size_t noPlayersCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, noPlayers);
    size_t opponentPlayerCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, opponentPlayer);
    size_t outsideCount = builder.addFunction(new GetMatrixRegionNumNeighboringElementsFunction(), region, outside);

    size_t i1 = builder.addFunction(enumerationFeatureGenerator(false), regionPlayer);

    builder.startSelection();

      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, log10((double)(boardSize * 4)), 10, true), regionSize, T("regionSize"));
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, log10((double)(boardSize * 4)), 10, true), noPlayersCount, T("noPlayersCount"));
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, log10((double)(boardSize * 4)), 10, true), opponentPlayerCount, T("opponentCount"));
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, log10((double)(boardSize * 4)), 10, true), outsideCount, T("borderCount"));

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
    size_t fourConnexityGraph = builder.addFunction(new SegmentMatrixFunction(false), board);
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

      FeatureGeneratorPtr fun2 = new MatrixNeighborhoodFeatureGenerator(new MatrixConnectivityFeatureGenerator(), getElementFunction());
      fun2->setLazy(false);
      size_t i4 = builder.addFunction(fun2, region4, row, column, region4Features, T("neighbors"));
      size_t i42 = builder.addFunction(cartesianProductFeatureGenerator(), i4, i4, T("neighbors2"));

    size_t features = builder.finishSelectionWithFunction(concatenateFeatureGenerator(false), T("f"));

    //size_t i42 = builder.addFunction(cartesianProductFeatureGenerator(true), i4, i4);
//    size_t featuresAndTime = builder.addFunction(cartesianProductFeatureGenerator(true), features, globalPrimaryFeatures, T("fAndTime"));
//    builder.addFunction(concatenateFeatureGenerator(true), features, featuresAndTime, T("all"));

    /*size_t features = 
    size_t featuresAndTime = builder.addFunction(cartesianProductFeatureGenerator(true), features, globalPrimaryFeatures, T("wt"));
    builder.addFunction(concatenateFeatureGenerator(true), features, featuresAndTime);*/
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
    size_t preFeatures = builder.addFunction(lbcppMemberCompositeFunction(GoActionsPerception, preFeaturesFunction), state);
    size_t actions = builder.addFunction(new GetAvailableActionsFunction(positiveIntegerPairClass), state, T("actions"));
    
    builder.addFunction(mapContainerFunction(lbcppMemberCompositeFunction(GoActionsPerception, actionFeatures)), actions, preFeatures);
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

///////////////////////////////
/////// GoEpisodeFunction /////
///////////////////////////////

// InitialState, Trajectory -> Nil
class GoEpisodeFunction : public SimpleBinaryFunction
{
public:
  GoEpisodeFunction(LearnerParametersPtr learningParameters = LearnerParametersPtr(), FunctionPtr rankingExampleCreator = FunctionPtr(), FunctionPtr rankingMachine = FunctionPtr())
    : SimpleBinaryFunction(goStateClass, containerClass(positiveIntegerPairClass), objectVectorClass(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration))),
      learningParameters(learningParameters), rankingExampleCreator(rankingExampleCreator), rankingMachine(rankingMachine)
  {
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!rankingExampleCreator->initialize(context, goStateClass, positiveIntegerPairClass))
      return TypePtr();
    TypePtr rankingExampleType = rankingExampleCreator->getOutputType();
    if (!rankingMachine->initialize(context, rankingExampleType->getMemberVariableType(0), rankingExampleType->getMemberVariableType(1)))
      return TypePtr();
    return SimpleBinaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& initialState = inputs[0].getObjectAndCast<GoState>();
    const ContainerPtr& trajectory = inputs[1].getObjectAndCast<Container>();

    GoStatePtr state = initialState->cloneAndCast<GoState>();

    size_t n = trajectory->getNumElements();
    ObjectVectorPtr res = new ObjectVector(getOutputType());
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = trajectory->getElement(i);

      Variable rankingExample = rankingExampleCreator->compute(context, state, action);
      DenseDoubleVectorPtr scores = rankingMachine->computeWithInputsObject(context, rankingExample.getObject()).getObjectAndCast<DenseDoubleVector>();
      res->append(scores);

      double reward;
      state->performTransition(action, reward);
    }
    return res;
  }

  const FunctionPtr& getRankingMachine() const
    {return rankingMachine;}
 
protected:
  friend class GoEpisodeFunctionClass;

  LearnerParametersPtr learningParameters;
  FunctionPtr rankingExampleCreator;
  FunctionPtr rankingMachine;
};

typedef ReferenceCountedObjectPtr<GoEpisodeFunction> GoEpisodeFunctionPtr;

///////////////////////////////
/////// Evaluators ////////////
///////////////////////////////

class CallbackBasedEvaluator : public Evaluator
{
public:
  CallbackBasedEvaluator(EvaluatorPtr evaluator)
    : evaluator(evaluator), callback(NULL) {}

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const = 0;

  struct Callback : public FunctionCallback
  {
    Callback(const EvaluatorPtr& evaluator, const ScoreObjectPtr& scores)
      : evaluator(evaluator), scores(scores) {}

    EvaluatorPtr evaluator;
    ScoreObjectPtr scores;

    virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
    {
      ObjectPtr inputsObject = Object::create(function->getInputsClass());
      for (size_t i = 0; i < inputsObject->getNumVariables(); ++i)
        inputsObject->setVariable(i, inputs[i]);
      evaluator->updateScoreObject(context, scores, inputsObject, output);
    }
  };

  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
  {
    ScoreObjectPtr res = evaluator->createEmptyScoreObject(context, function);
    FunctionPtr functionToListen = getFunctionToListen(function);
    functionToListen->addPostCallback(const_cast<CallbackBasedEvaluator* >(this)->callback = new Callback(evaluator, res));
    return res;
  }

  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
    {return true;}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
  {
    evaluator->finalizeScoreObject(scores, function);
    getFunctionToListen(function)->removePostCallback(callback);
    deleteAndZero(const_cast<CallbackBasedEvaluator* >(this)->callback);
  }

protected:
  friend class CallbackBasedEvaluatorClass;

  EvaluatorPtr evaluator;
  Callback* callback; // pas bien: effet de bord
};

////

class PositiveIntegerPairScoringScoreObject : public ScoreObject
{
public:
  PositiveIntegerPairScoringScoreObject() 
    : predictionRate(new ScalarVariableMean(T("predictionRate"))), 
      rankOfAction(new ScalarVariableStatistics(T("rankOfAction"))),
      unsupervisedRate(new ScalarVariableMean(T("unsupervisedRate"))) {}

  static int getRank(const std::multimap<double, size_t>& sortedScores, size_t index)
  {
    int res = 0;
    for (std::multimap<double, size_t>::const_iterator it = sortedScores.begin(); it != sortedScores.end(); ++it, ++res)
      if (it->second == index)
        return res;
    return -1;
  }

  bool add(ExecutionContext& context, const DenseDoubleVectorPtr& scores, const DenseDoubleVectorPtr& costs)
  {
    std::multimap<double, size_t> sortedScores;
    for (size_t i = 0; i < scores->getNumElements(); ++i)
      sortedScores.insert(std::make_pair(-(scores->getValue(i)), i));
    
    if (sortedScores.empty())
    {
      context.errorCallback(T("No scores"));
      return false;
    }

    // prediction rate
    size_t selectedAction = sortedScores.begin()->second;
    predictionRate->push(costs->getValue(selectedAction) < 0 ? 1.0 : 0.0);

    // rank of selected action
    size_t index = costs->getIndexOfMinimumValue();
    if (index >= 0 && costs->getValue(index) < 0)
    {
      int rank = getRank(sortedScores, index);
      if (rank >= 0)
      {
        rankOfAction->push((double)rank);
        unsupervisedRate->push(0.0);
      }
      else
        unsupervisedRate->push(1.0);
    }
    else
      unsupervisedRate->push(1.0);
    return true;
  }

  virtual double getScoreToMinimize() const
    //{return 1.0 - predictionRate->getMean();} // prediction error
    {return rankOfAction->getMean();} // mean rank of best action

private:
  friend class PositiveIntegerPairScoringScoreObjectClass;

  ScalarVariableMeanPtr predictionRate;
  ScalarVariableStatisticsPtr rankOfAction;
  ScalarVariableMeanPtr unsupervisedRate;
};

class PositiveIntegerPairScoringEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual TypePtr getRequiredSupervisionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new PositiveIntegerPairScoringScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {result.staticCast<PositiveIntegerPairScoringScoreObject>()->add(context, prediction.getObjectAndCast<DenseDoubleVector>(), supervision.getObjectAndCast<DenseDoubleVector>());}
};

class GoEpisodeFunctionEvaluator : public CallbackBasedEvaluator
{
public:
  GoEpisodeFunctionEvaluator() : CallbackBasedEvaluator(new PositiveIntegerPairScoringEvaluator()) {}

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const
  {
    const GoEpisodeFunctionPtr& episodeFunction = evaluatedFunction.staticCast<GoEpisodeFunction>();
    return episodeFunction->getRankingMachine();
  }
};

///////////////////////////////
////////// SandBox ////////////
///////////////////////////////
GoStateComponent::GoStateComponent(GoStatePtr state, const String& name)
  : MatrixComponent(state->getBoard()), state(state), actionsPerceptionFunction(new GoActionsPerception())
{
}

class GoSandBox : public WorkUnit
{
public:
  GoSandBox() : maxCount(0), numFolds(7), learningParameters(new StochasticGDParameters(constantIterationFunction(1.0)))
  {
  }

  ContainerPtr loadGames(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    if (!gamesDirectory.isDirectory())
    {
      context.errorCallback(T("Invalid games directory"));
      return ContainerPtr();
    }

    return directoryFileStream(context, directory, T("*.sgf"))->load(maxCount, false)->apply(context, new LoadSGFFileFunction(), Container::parallelApply);
  }

  virtual Variable run(ExecutionContext& context)
  {
    double startTime = Time::getMillisecondCounterHiRes();

    // create problem
    DecisionProblemPtr problem = new GoProblem(0);

    // load games
    ContainerPtr games = loadGames(context, gamesDirectory, maxCount);
    if (!games)
      return false;
    ContainerPtr trainingGames = games->invFold(0, numFolds);
    ContainerPtr validationGames = games->fold(0, numFolds);
    context.informationCallback(String((int)trainingGames->getNumElements()) + T(" training games, ") +
                                String((int)validationGames->getNumElements()) + T(" validation games"));
    
#if 0
    // TMP
    PairPtr pair = trainingGames->getElement(0).getObjectAndCast<Pair>();
    DecisionProblemStatePtr state = pair->getFirst().getObjectAndCast<DecisionProblemState>();
    ContainerPtr trajectory  = pair->getSecond().getObjectAndCast<Container>();
    for (size_t i = 0; i < 151; ++i)
    {
      double r;
      state->performTransition(trajectory->getElement(i), r);
    }
    context.resultCallback(T("state"), state);

    GoActionsPerceptionPtr perception = new GoActionsPerception();
    perception->initialize(context, goStateClass);
    context.resultCallback(T("preFeatures"), perception->computePreFeatures(context, state));
    return true;
    // -
#endif // 0
    

    // create ranking machine
    if (!learningParameters)
    {
      context.errorCallback(T("No learning parameters"));
      return false;
    }
    FunctionPtr rankingExampleCreator = new DecisionProblemStateActionsRankingExample(new GoActionsPerception());
    StochasticGDParametersPtr sgdParameters = learningParameters.dynamicCast<StochasticGDParameters>();
    if (!sgdParameters)
    {
      context.errorCallback(T("Learning parameters type not supported"));
      return false;
    }
    FunctionPtr rankingMachine = linearRankingMachine(new StochasticGDParameters(sgdParameters->getLearningRate(), StoppingCriterionPtr(), 0,
                                                                                 sgdParameters->doPerEpisodeUpdates(), sgdParameters->doNormalizeLearningRate(),
                                                                                 false, true, false));
    //rankingMachine->setEvaluator(new PositiveIntegerPairScoringEvaluator());

    FunctionPtr goEpisodeFunction = new GoEpisodeFunction(learningParameters, rankingExampleCreator, rankingMachine);
    if (!goEpisodeFunction->initialize(context, goStateClass, containerClass(positiveIntegerPairClass)))
      return false;
    EvaluatorPtr evaluator = new GoEpisodeFunctionEvaluator();
    evaluator->setUseMultiThreading(true);
    goEpisodeFunction->setEvaluator(evaluator);
    goEpisodeFunction->setBatchLearner(learningParameters->createBatchLearner(context));
    goEpisodeFunction->setOnlineLearner(
      compositeOnlineLearner(evaluatorOnlineLearner(false, true), stoppingCriterionOnlineLearner(sgdParameters->getStoppingCriterion()), restoreBestParametersOnlineLearner()));
    //rankingMachine->setOnlineLearner(perEpisodeGDOnlineLearner(FunctionPtr(), constantIterationFunction(1.0), true));
    
    goEpisodeFunction->train(context, trainingGames, validationGames, T("Training"), true);

    //goEpisodeFunction->evaluate(context, trainingGames, EvaluatorPtr(), T("Evaluating on training examples"));
    //goEpisodeFunction->evaluate(context, validationGames, EvaluatorPtr(), T("Evaluating on validation examples"));

    return Variable((Time::getMillisecondCounterHiRes() - startTime) / 1000.0, timeType);

   // return learnOnline(context, rankingMachine, trainingExamples, validationExamples);

    /*
    // check validity
    context.enterScope(T("Check validity"));
    bool ok = true;
    for (size_t i = 0; i < games->getNumElements(); ++i)
    {
      context.progressCallback(new ProgressionState(i, games->getNumElements(), T("Games")));
      PairPtr stateAndTrajectory = games->getElement(i).getObjectAndCast<Pair>();
      if (stateAndTrajectory)
      {
        DecisionProblemStatePtr state = stateAndTrajectory->getFirst().getObject()->cloneAndCast<DecisionProblemState>();
        ok &= state->checkTrajectoryValidity(context, stateAndTrajectory->getSecond().getObjectAndCast<Container>());
      }
      
    }
    context.leaveScope(ok);
    return true;
    */

  }

private:
  friend class GoSandBoxClass;

  File gamesDirectory;
  size_t maxCount;
  size_t numFolds;
  LearnerParametersPtr learningParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
