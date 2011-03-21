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

  // todo: addNeighboringRegion

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

private:
  friend class SegmentedMatrixClass;

  TypePtr elementsType; // the input elementsType  /!\ this is different from BaseClass::elementsType whose value is "PositiveInteger"
  std::vector<MatrixRegionPtr> regions;
};

typedef ReferenceCountedObjectPtr<SegmentedMatrix> SegmentedMatrixPtr;

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
    toExplore.insert(Position(row, column));
    while (toExplore.size())
    {
      // add current position
      Position position = *toExplore.begin();
      toExplore.erase(toExplore.begin());
      if (res->hasElement(position))
        continue;

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
        if (!res->hasElement(neighbor))
        {
          Variable neighborValue = matrix->getElement(neighbor.first, neighbor.second);
          if (neighborValue == value)
            toExplore.insert(neighbor);
          //else
          //  region->addNeighboringElements(neighborValue);
        }
      }
    }
  }

protected:
  friend class SegmentMatrixFunctionClass;

  bool use8Connexity;
  TypePtr elementsType;
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
  MatrixPtr fourConnexityGraphFeatures;
  MatrixPtr eightConnexityGraphFeatures;
  MatrixPtr actionPrimaryFeatures;
};

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

    size_t i1 = builder.addFunction(enumerationFeatureGenerator(false), regionPlayer);
    size_t i2 = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, log10((double)(boardSize * boardSize)), 10, false), regionSize);

    builder.addFunction(cartesianProductFeatureGenerator(false), i1, i2);
  }

  void selectionRegionFeaturesToPutInMatrix(CompositeFunctionBuilder& builder)
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t perRegionFeatures = builder.addInput(vectorClass(doubleVectorClass()));
    builder.addFunction(getElementFunction(), perRegionFeatures, position);
  }

  // SegmentedMatrix<Player> -> Matrix<DoubleVector>
  void segmentedBoardFeatures(CompositeFunctionBuilder& builder)
  {
    size_t segmentedBoard = builder.addInput(segmentedMatrixClass(playerEnumeration), T("segmentedBoard"));
    size_t regions = builder.addFunction(getVariableFunction(T("regions")), segmentedBoard);
    size_t perRegionFeatures = builder.addFunction(mapContainerFunction(lbcppMemberCompositeFunction(GoActionsPerception, regionFeatures)), regions);
    builder.addFunction(mapContainerFunction(lbcppMemberCompositeFunction(GoActionsPerception, selectionRegionFeaturesToPutInMatrix)), segmentedBoard, perRegionFeatures);
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
    FunctionPtr fun = lbcppMemberCompositeFunction(GoActionsPerception, segmentedBoardFeatures);
    size_t fourConnexityGraphFeatures = builder.addFunction(fun, fourConnexityGraph);
    variables.push_back(fourConnexityGraphFeatures);
    size_t eightConnexityGraph = builder.addFunction(new SegmentMatrixFunction(true), board);
    size_t eightConnexityGraphFeatures = builder.addFunction(fun, eightConnexityGraph);
    variables.push_back(eightConnexityGraphFeatures);
    EnumerationPtr regionFeaturesEnumeration = DoubleVector::getElementsEnumeration(Container::getTemplateParameter(builder.getOutputType()));

    // board primary features
    size_t boardPrimaryFeatures = builder.addFunction(mapContainerFunction(enumerationFeatureGenerator(false)), board);
    EnumerationPtr actionFeaturesEnumeration = DoubleVector::getElementsEnumeration(Container::getTemplateParameter(builder.getOutputType()));
    jassert(actionFeaturesEnumeration);
    variables.push_back(boardPrimaryFeatures);

    builder.addFunction(createObjectFunction(goStatePreFeaturesClass(globalFeaturesEnumeration, regionFeaturesEnumeration, actionFeaturesEnumeration)), variables, T("goPreFeatures"));
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
    size_t region4Features = builder.addFunction(getVariableFunction(T("fourConnexityGraphFeatures")), preFeatures);
    size_t region8Features = builder.addFunction(getVariableFunction(T("eightConnexityGraphFeatures")), preFeatures);
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

      size_t i3 = builder.addFunction(fixedContainerWindowFeatureGenerator(0, 5), previousActionRelationFeatures, T("previousAction"));

      size_t i4 = builder.addFunction(matrixWindowFeatureGenerator(3, 3), region4Features, row, column, T("reg4"));
      //size_t i5 = builder.addFunction(matrixWindowFeatureGenerator(3, 3), region8Features, row, column, T("reg8"));

      //builder.addInSelection(globalPrimaryFeatures);
      //builder.addFunction(cartesianProductFeatureGenerator(true), i3, i3, T("previousAction2"));
      //builder.addFunction(cartesianProductFeatureGenerator(true), i1, i2, T("posWin"));

    size_t features = builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));

    //size_t i42 = builder.addFunction(cartesianProductFeatureGenerator(true), i4, i4);
    size_t featuresAndTime = builder.addFunction(cartesianProductFeatureGenerator(true), features, globalPrimaryFeatures);
    builder.addFunction(concatenateFeatureGenerator(true), features, featuresAndTime);

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

juce::Component* GoStateComponent::createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
{
  const PairPtr& matrixAndPosition = variable.getObjectAndCast<Pair>();
  const PairPtr& position = matrixAndPosition->getSecond().getObjectAndCast<Pair>();
  Variable positiveIntegerPair(new PositiveIntegerPair(position->getSecond().getInteger(), position->getFirst().getInteger()));

  FunctionPtr perception = new GoActionsPerception();
  ContainerPtr actionPerceptions = perception->compute(context, state).getObjectAndCast<Container>();
  Variable actionPerception;
  if (actionPerceptions)
  {
    ContainerPtr actions = state->getAvailableActions();
    jassert(actions->getNumElements() == actionPerceptions->getNumElements());
    for (size_t i = 0; i < actions->getNumElements(); ++i)
      if (actions->getElement(i) == positiveIntegerPair)
      {
        actionPerception = actionPerceptions->getElement(i);
        break;
      }
  }

  if (actionPerception.exists())
    return userInterfaceManager().createVariableTreeView(context, actionPerception, name + T(" perception"), false);
  else
    return NULL;
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
    for (size_t i = 0; i < 150; ++i)
    {
      double r;
      state->performTransition(trajectory->getElement(i), r);
    }
    context.resultCallback(T("state"), state);

    FunctionPtr segmentFunction4 = new SegmentMatrixFunction(false);
    FunctionPtr segmentFunction8 = new SegmentMatrixFunction(true);
    if (!segmentFunction4->initialize(context, goBoardClass) || !segmentFunction8->initialize(context, goBoardClass))
      return false;

    GoBoardPtr board = state.staticCast<GoState>()->getBoard();
    context.resultCallback(T("segment4"), segmentFunction4->compute(context, board));
    context.resultCallback(T("segment8"), segmentFunction8->compute(context, board));

    FunctionPtr perception = new GoActionsPerception();
    context.resultCallback(T("actionFeatures"), perception->compute(context, state));
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
