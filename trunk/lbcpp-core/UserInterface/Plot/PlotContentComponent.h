/*-----------------------------------------.---------------------------------.
| Filename: PlotContentComponent.h         | Plot Content Component          |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2011 12:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_USER_INTERFACE_PLOT_CONTENT_COMPONENT_H_
# define LBCPP_USER_INTERFACE_PLOT_CONTENT_COMPONENT_H_

# include <lbcpp/UserInterface/Plot.h>
# include <lbcpp/UserInterface/ObjectComponent.h>
# include "TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

using juce::RectanglePlacement;
using juce::Justification;
using juce::AffineTransform;

class PlotDrawable : public TwoDimensionalPlotDrawable
{
public:
  PlotDrawable(const PlotPtr& plot) : plot(plot)
  {
    selectedCurves = plot->getSelectedCurves();
    size_t keyVariableIndex = plot->getKeyVariableIndex();
    int columnIndex = plot->getData()->findColumnByKey(plot->getPlotVariable(keyVariableIndex)->getKey());
    jassert(columnIndex >= 0);
    plot->getData()->makeOrder((size_t)columnIndex, true, order);
    computeBounds();
    if (!areBoundsValid())
    {
      boundsX = -1000.0;
      boundsWidth = 2000.0;
      boundsY = -1.0;
      boundsHeight = 2.0;
    }
  }

  enum {pointCrossSize = 6};

  virtual Drawable* createCopy() const
    {return new PlotDrawable(plot);}

  virtual void draw(Graphics& g, const AffineTransform& transform = AffineTransform::identity) const
  {
    TwoDimensionalPlotDrawable::draw(g, transform);
    if (areBoundsValid() && plot->getData()->getNumRows() > 0)
      for (size_t i = 0; i < selectedCurves.size(); ++i)
      {
        drawCurveLine(g, transform, selectedCurves[i], plot->getPlotVariable(selectedCurves[i]));
        drawCurvePoint(g, transform, selectedCurves[i], plot->getPlotVariable(selectedCurves[i])); 
      }
  }

  virtual PlotAxisPtr getXAxis() const
    {return plot->getXAxis();}

  virtual PlotAxisPtr getYAxis() const
    {return plot->getYAxis();}

  virtual String getXAxisLabel() const
  {
    String res = plot->getXAxis()->getLabel();
    if (res.isEmpty())
      res = plot->getPlotVariable(plot->getKeyVariableIndex())->getName();
    return res;
  }

  virtual String getYAxisLabel() const
  {
    String res = plot->getYAxis()->getLabel();
    if (res.isEmpty())
    {
      for (size_t i = 0; i < selectedCurves.size(); ++i)
      {
        if (i > 0)
          res += T(", ");
        res += plot->getPlotVariable(selectedCurves[i])->getName();
      }
    }
    return res;
  }

  virtual void computeXAxisAutoRange(PlotAxisPtr axis) const
  {
    double minValue = DBL_MAX, maxValue = -DBL_MAX;
    getTableValueRange(plot->getKeyVariableIndex(), minValue, maxValue);
    if (maxValue > minValue)
    {
      axis->setRange(minValue, maxValue);
      axis->increaseRange(0.2);
    }
    else if (maxValue == minValue)
      axis->setRange(minValue - 1.1, maxValue + 1.1);
  }

  virtual void computeYAxisAutoRange(PlotAxisPtr axis) const
  {
    double minValue = DBL_MAX, maxValue = -DBL_MAX;
    for (size_t i = 0; i < selectedCurves.size(); ++i)
      getTableValueRange(selectedCurves[i], minValue, maxValue);
    if (maxValue > minValue)
    {
      axis->setRange(minValue, maxValue);
      axis->increaseRange(0.2);
    }
    else if (maxValue == minValue)
      axis->setRange(minValue - 1.1, maxValue + 1.1);
  }

protected:
  PlotPtr plot;

  std::vector<size_t> selectedCurves;
  std::vector<size_t> order;

  bool getPointPosition(size_t row, size_t columnX, size_t columnY, const AffineTransform& transform, float& x, float& y) const
  {
    int cx = plot->getData()->findColumnByKey(plot->getPlotVariable(columnX)->getKey());
    int cy = plot->getData()->findColumnByKey(plot->getPlotVariable(columnY)->getKey());
    jassert(cx >= 0 && cy >= 0);

    x = (float)Variable(plot->getData()->getElement(order[row], (size_t)cx)).toDouble();
    y = (float)Variable(plot->getData()->getElement(order[row], (size_t)cy)).toDouble();
    transform.transformPoint(x, y);
    return true;
  }

  void getTableValueRange(size_t column, double& minValue, double& maxValue) const
  {
    int c = plot->getData()->findColumnByKey(plot->getPlotVariable(column)->getKey());
    jassert(c >= 0);
    for (size_t i = 0; i < plot->getData()->getNumRows(); ++i)
    {
      double value = Variable(plot->getData()->getElement(i, c)).toDouble();
      if (value > maxValue)
        maxValue = value;
      if (value < minValue)
        minValue = value;
    }
  }

  void drawCurveLine(Graphics& g, const AffineTransform& transform, size_t index, PlotVariablePtr plotVariable) const
  {
    size_t n = plot->getData()->getNumRows();
    size_t keyVariableIndex = plot->getKeyVariableIndex();
    float x0, y0;
    bool isP0Valid = getPointPosition(0, keyVariableIndex, index, transform, x0, y0) && isNumberValid(x0) && isNumberValid(y0);

    g.setColour(plotVariable->getColour());
    for (size_t i = 1; i < n; ++i)
    {
      float x1, y1;
      bool isP1Valid = getPointPosition(i, keyVariableIndex, index, transform, x1, y1) && isNumberValid(x1) && isNumberValid(y1);
      if (isP0Valid && isP1Valid && fabs(y0) < 1e16 && fabs(y1) < 1e16)
        g.drawLine(x0, y0, x1, y1); // juce has a problem with very very large values, so we restrict y-values to 1e16
      x0 = x1;
      y0 = y1;
      isP0Valid = isP1Valid;
    }
  }

  void drawCurvePoint(Graphics& g, const AffineTransform& transform, size_t index, PlotVariablePtr plotVariable) const
  {
    size_t n = plot->getData()->getNumRows();
    size_t keyVariableIndex = plot->getKeyVariableIndex();

    g.setColour(plotVariable->getColour());
    for (size_t i = 0; i < n; ++i)
    {
      float x, y;
      float crossHalfSize = pointCrossSize / 2.f;
      if (getPointPosition(i, keyVariableIndex, index, transform, x, y) && isNumberValid(x) && isNumberValid(y))
      {
        g.drawLine(x - crossHalfSize, y, x + crossHalfSize, y);
        g.drawLine(x, y - crossHalfSize, x, y + crossHalfSize);
      }
    }
  }
};

class PlotContentComponent : public Component
{
public:
  PlotContentComponent(const PlotPtr& plot)
    {drawable = new PlotDrawable(plot);}

  virtual ~PlotContentComponent()
    {delete drawable;}

  enum
  {
    leftMargin = 60,
    rightMargin = 20,
    topMargin = 20,
    bottomMargin = 40
  };

  virtual void paint(Graphics& g)
    {drawable->drawWithin(g, leftMargin, topMargin, getWidth() - leftMargin - rightMargin, getHeight() - topMargin - bottomMargin, juce::RectanglePlacement::stretchToFit);}

protected:
  PlotDrawable* drawable;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_CONTENT_COMPONENT_H_
