/*-----------------------------------------.---------------------------------.
| Filename: MatrixComponent.h              | Matrix Component Base class     |
| Author  : Francis Maes                   |                                 |
| Started : 15/03/2011 11:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_MATRIX_COMPONENT_H_
# define LBCPP_USER_INTERFACE_MATRIX_COMPONENT_H_

# include "ObjectEditor.h"
# include "ComponentWithPreferedSize.h"
# include "VariableSelector.h"

namespace lbcpp
{

class MatrixComponent : public juce::Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  MatrixComponent(MatrixPtr matrix)
    : matrix(matrix), selectedRow(-1), selectedColumn(-1)
  {
  }
  
  int computePixelsPerEntry(int availableWidth, int availableHeight) const
  {
    int availableSize = juce::jmin(availableWidth, availableHeight);
    int n = juce::jmax((int)matrix->getNumRows(), (int)matrix->getNumColumns());
    return n ? juce::jmax(1, availableSize / (int)n) : 4;
  }
  
  virtual int getDefaultWidth() const
    {return 400;}

  virtual int getDefaultHeight() const
    {return 400;}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    int pixelsPerEntry = computePixelsPerEntry(availableWidth, availableHeight);
    return juce::jmax(pixelsPerEntry * (int)matrix->getNumColumns(), availableWidth);
  }

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    int pixelsPerEntry = computePixelsPerEntry(availableWidth, availableHeight);
    return juce::jmax(pixelsPerEntry * (int)matrix->getNumRows(), availableHeight);
  }

  virtual void paint(juce::Graphics& g)
  {
    int pixelsPerRow, pixelsPerColumn, x1, y1;
    getPaintCoordinates(pixelsPerRow, pixelsPerColumn, x1, y1);
  
    size_t numRows = matrix->getNumRows();
    size_t numColumns = matrix->getNumColumns();

    for (size_t row = 0; row < numRows; ++row)
      for (size_t column = 0; column < numColumns; ++column)
        paintEntry(g, x1 + column * pixelsPerColumn, y1 + row * pixelsPerRow, pixelsPerColumn, pixelsPerRow, matrix->getElement(row, column));
      
    if (selectedRow >= 0 && selectedColumn >= 0)
    {
      g.setColour(Colours::lightblue.withAlpha(0.7f));
      g.fillRect(x1 + (selectedColumn - 4) * pixelsPerColumn, y1 + selectedRow * pixelsPerRow, 9 * pixelsPerColumn - 1, pixelsPerRow - 1);
      g.fillRect(x1 + selectedColumn * pixelsPerColumn, y1 + (selectedRow - 4) * pixelsPerRow, pixelsPerColumn - 1, 9 * pixelsPerRow - 1);
    }

    g.setColour(Colours::black);
    g.drawRect(x1, y1, (int)numColumns * pixelsPerColumn, (int)numRows * pixelsPerRow);
  }

  virtual void mouseUp(const juce::MouseEvent& e)
  {
    int pixelsPerRow, pixelsPerColumn, x1, y1;
    getPaintCoordinates(pixelsPerRow, pixelsPerColumn, x1, y1);
    jassert(pixelsPerRow && pixelsPerColumn);
    int x = (e.getMouseDownX() - x1) / pixelsPerColumn;
    int y = (e.getMouseDownY() - y1) / pixelsPerRow;
    if (x >= 0 && y >= 0 && x < (int)matrix->getNumColumns() && y < (int)matrix->getNumRows())
    {
      selectedRow = y;
      selectedColumn = x;
      sendSelectionChanged(matrix->getElement(selectedRow, selectedColumn),
        T("Element (") + String((int)selectedRow) + T(", ") + String((int)selectedColumn) + T(")"));
      repaint();
      return;
    }

    selectedRow = selectedColumn = -1;
    sendSelectionChanged(std::vector<Variable>(), String::empty);
    repaint();
  }
  
  // overidable
  virtual juce::Colour selectColour(const Variable& element) const = 0;
  virtual void paintEntry(juce::Graphics& g, int x1, int y1, int width, int height, const Variable& element)
  {
    if (matrix)
    {
      g.setColour(selectColour(element));
      g.fillRect(x1, y1, width, height);
    }
  }

private:
  MatrixPtr matrix;
  int selectedRow, selectedColumn;

  void getPaintCoordinates(int& pixelsPerRow, int& pixelsPerColumn, int& x1, int& y1) const
  {
    pixelsPerRow = pixelsPerColumn = computePixelsPerEntry(getWidth(), getHeight());
    x1 = (getWidth() - pixelsPerColumn * (int)matrix->getNumColumns()) / 2;
    y1 = (getHeight() - pixelsPerRow * (int)matrix->getNumRows()) / 2;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_MATRIX_COMPONENT_H_
