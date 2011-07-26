/*-----------------------------------------.---------------------------------.
| Filename: SegmentMatrixFunction.h        | Segment Matrix Function         |
| Author  : Francis Maes                   |                                 |
| Started : 06/04/2011 01:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_MATRIX_SEGMENT_FUNCTION_H_
# define LBCPP_DATA_MATRIX_SEGMENT_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/Matrix.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/UserInterface/MatrixComponent.h>

namespace lbcpp
{

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
    RandomGeneratorPtr random = defaultExecutionContext().getRandomGenerator();
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
    res->setSourceMatrix(matrix);

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

      //size_t numMissingNeighbors = (use8Connexity ? 8 : 4) - numNeighbors;
      //region->addNeighboringElement(Variable::missingValue(elementsType), numMissingNeighbors);

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
          region->addNeighbor(neighbor);
      }
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SegmentMatrixFunctionClass;

  bool use8Connexity;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_MATRIX_SEGMENT_FUNCTION_H_
