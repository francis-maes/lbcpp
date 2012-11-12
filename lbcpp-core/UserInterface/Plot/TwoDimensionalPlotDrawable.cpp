/*-----------------------------------------.---------------------------------.
| Filename: TwoDimensionalPlotDrawable.cpp | 2-D Plot Drawable base class    |
| Author  : Francis Maes                   |                                 |
| Started : 11/09/2012 19:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "TwoDimensionalPlotDrawable.h"
using namespace lbcpp;
using juce::Drawable;
using juce::AffineTransform;
using juce::Rectangle;
using juce::Graphics;
using juce::Justification;

TwoDimensionalPlotDrawable::TwoDimensionalPlotDrawable()
  : boundsX(0.0), boundsY(0.0), boundsWidth(0.0), boundsHeight(0.0)
{
}

bool TwoDimensionalPlotDrawable::hitTest(float x, float y) const
  {return true;}

void TwoDimensionalPlotDrawable::getBounds(float& x, float& y, float& width, float& height) const
{
  x = (float)boundsX;
  y = (float)(boundsY + boundsHeight);
  width = (float)boundsWidth;
  height = (float)-boundsHeight;
}

void TwoDimensionalPlotDrawable::draw(Graphics& g, const AffineTransform& transform) const
{
  if (areBoundsValid())
  {
    drawZeroAxis(g, transform, getYAxis(), false);
    drawZeroAxis(g, transform, getXAxis(), true);
    drawFrame(g, transform);
  }
}

void TwoDimensionalPlotDrawable::computeBounds()
{
  // x-axis
  PlotAxisPtr axis = getXAxis();
  if (axis->hasAutoRange())
    computeXAxisAutoRange(axis);
  boundsX = axis->getRangeMin();
  boundsWidth = axis->getRangeMax() - axis->getRangeMin();

  // y-axis
  axis = getYAxis();
  if (axis->hasAutoRange())
    computeYAxisAutoRange(axis);
  boundsY = axis->getRangeMin();
  boundsHeight = axis->getRangeMax() - axis->getRangeMin();
}

bool TwoDimensionalPlotDrawable::areBoundsValid() const
{
  static const double epsilon = 1e-9;
  return isNumberValid(boundsX) && isNumberValid(boundsY) &&
          isNumberValid(boundsWidth) && isNumberValid(boundsHeight) && 
          fabs(boundsWidth) > epsilon && fabs(boundsHeight) > epsilon;
}

void TwoDimensionalPlotDrawable::getMarkerValues(double minValue, double maxValue, double valuePerPixel,
                            std::vector<double>& main, std::vector<double>& secondary)
{
  int powerOfTen = (int)(log10(valuePerPixel) + 0.9);
  while (main.size() < 2)
  {
    main.clear();
    secondary.clear();

    double step = pow(10.0, (double)powerOfTen);
    int ibegin = (int)(minValue / step);
    int iend = (int)(maxValue / step);
    for (int i = ibegin; i <= iend; ++i)
    {
      double time = i * step;
      if (time >= minValue && time <= maxValue)
      {
        if (i % 10 == 0)
          main.push_back(time);
        else
          secondary.push_back(time);
      }
    }

    --powerOfTen; // if there are not enough main markers, retry with a more finner scale
  }
}

juce::Rectangle TwoDimensionalPlotDrawable::getFrameRectangle(const AffineTransform& transform) const
{
  float x, y, width, height;
  getBounds(x, y, width, height);
  float x2 = x + width, y2 = y + height;
  transform.transformPoint(x, y);
  transform.transformPoint(x2, y2);
  return juce::Rectangle((int)(x + 0.5), (int)(y + 0.5), (int)(x2 - x + 0.5f), (int)(y2 - y + 0.5f));
}

void TwoDimensionalPlotDrawable::drawFrameMarker(Graphics& g, const AffineTransform& transform, double value, bool isHorizontal, bool isMainMarker) const
{
  int dirh = isHorizontal ? 1 : 0;
  int dirv = isHorizontal ? 0 : 1;
  float frameMarkerHalfSize = (isMainMarker ? frameMarkerSize1 : frameMarkerSize2) / 2.f;

  float x, y;
  if (isHorizontal)
    x = (float)value, y = (float)(boundsY + boundsHeight);
  else
    x = (float)(boundsX + boundsWidth), y = (float)value;
  transform.transformPoint(x, y);

  g.drawLine(x - dirv * frameMarkerHalfSize, y - dirh * frameMarkerHalfSize, x + dirv * frameMarkerHalfSize, y + dirh * frameMarkerHalfSize);

  if (isHorizontal)
    x = (float)value, y = (float)boundsY;
  else
    x = (float)boundsX, y = (float)value;
  transform.transformPoint(x, y);
  g.drawLine(x - dirv * frameMarkerHalfSize, y - dirh * frameMarkerHalfSize, x + dirv * frameMarkerHalfSize, y + dirh * frameMarkerHalfSize);

  if (isMainMarker)
  {
    g.setFont(11);
    string str = ObjectPtr(new Double(value))->toShortString();
    if (isHorizontal)
      g.drawText(str, (int)(x - 40), (int)(y + 5), 80, bottomValuesSize - 5, Justification::centred, false);
    else
      g.drawText(str, (int)(x - 60), (int)(y - 10), 80, 20, Justification::centred, false);
  }
}

void TwoDimensionalPlotDrawable::drawFrameMarkers(Graphics& g, const AffineTransform& transform, juce::Rectangle frameRectangle, bool isHorizontal) const
{
  double minValue, maxValue, valuePerPixel;
  if (isHorizontal)
    minValue = boundsX, maxValue = boundsX + boundsWidth, valuePerPixel = boundsWidth / frameRectangle.getWidth();
  else
    minValue = boundsY, maxValue = boundsY + boundsHeight, valuePerPixel = boundsHeight / frameRectangle.getHeight();
  
  std::vector<double> mainMarkers, secondaryMarkers;
  getMarkerValues(minValue, maxValue, valuePerPixel, mainMarkers, secondaryMarkers);

  g.setColour(juce::Colours::lightgrey);
  for (size_t i = 0; i < secondaryMarkers.size(); ++i)
    drawFrameMarker(g, transform, secondaryMarkers[i], isHorizontal, false);
  g.setColour(juce::Colours::black);
  for (size_t i = 0; i < mainMarkers.size(); ++i)
    drawFrameMarker(g, transform, mainMarkers[i], isHorizontal, true);
}

void TwoDimensionalPlotDrawable::drawFrame(Graphics& g, const AffineTransform& transform) const
{
  juce::Rectangle rect = getFrameRectangle(transform);
  
  drawFrameMarkers(g, transform, rect, true);
  drawFrameMarkers(g, transform, rect, false);

  g.setColour(juce::Colours::black);
  g.drawRect(rect.getX(), rect.getY(), rect.getWidth() + 1, rect.getHeight() + 1, 1);

  g.setFont(16);
  g.drawText(getXAxisLabel(), rect.getX(), rect.getBottom() + bottomValuesSize, rect.getWidth(), 20, Justification::centred, false);

  string yAxisLabel = getYAxisLabel();
  float transx = (float)rect.getX() - 40.f;
  float transy = (float)rect.getY() + rect.getHeight() * (0.75f + juce::jmin(1.f, yAxisLabel.length() / 50.f)) / 2.f;
  g.drawTextAsPath(yAxisLabel, AffineTransform::rotation(-(float)(M_PI / 2.f)).translated(transx, transy));
}

void TwoDimensionalPlotDrawable::drawZeroAxis(Graphics& g, const AffineTransform& transform, PlotAxisPtr config, bool isHorizontalAxis) const
{
  float x0 = 0.f, y0 = 0.f;
  transform.transformPoint(x0, y0);

  juce::Rectangle frameRect = getFrameRectangle(transform);

  g.setColour(juce::Colours::lightgrey);
  if (isHorizontalAxis)
  {
    if (0 >= boundsY && 0 <= boundsY + boundsHeight)
      g.drawLine((float)frameRect.getX(), y0, (float)frameRect.getRight(), y0);
  }
  else
  {
    if (0 >= boundsX && 0 <= boundsX + boundsWidth)
      g.drawLine(x0, (float)frameRect.getY(), x0, (float)frameRect.getBottom());
  }
}
