/*-----------------------------------------.---------------------------------.
| Filename: ContourPlot.cpp                | Contour Plot                    |
| Author  : Denny Verbeeck                 |                                 |
| Started : 05/04/2013 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ContourPlotDrawable.h"
using namespace lbcpp;

void ColourScale::initialize(TablePtr data)
{
  rangeMin = DBL_MAX;
  rangeMax = -DBL_MAX;
  for (size_t r = 0; r < data->getNumRows(); ++r)
    for (size_t c = 0; c < data->getNumColumns(); ++c)
    {
      double v = Double::get(data->getElement(r,c));
      if (rangeMin > v)
        rangeMin = v;
      if (rangeMax < v)
        rangeMax = v;
    }
}

double ColourScale::normalize(double value, double lower, double upper, double l, double u) const
{
  if (upper == lower)
    return l;
  return l + (value - lower) * (u - l) / (upper - lower);
}

juce::Image* ColourScale::getColourBar(size_t width, size_t height, int numTicks) const
{
  jassert(height > 12); // we need at least 13 pix
  // draw the colour bar
  double step = (rangeMax - rangeMin) / width;
  juce::Image* res = new juce::Image(juce::Image::RGB, width, height, false);
  juce::Graphics g(*res);
  g.fillAll(juce::Colour(255, 255, 255));
  for (size_t i = 0; i < width; ++i)
  {
    g.setColour(getColour(rangeMin + i * step));
    g.drawVerticalLine(i, 12.0, (float)height);
  }
  g.drawVerticalLine(0, 12.0, (float)height);
  g.drawVerticalLine(width, 12.0, (float)height);
  g.drawHorizontalLine(12, 0.0, (float)width);
  g.drawHorizontalLine(height, 0.0, (float)width);

  // draw the scale
  char* number = new char[8];
  double div = (rangeMax - rangeMin) / numTicks;
  
  g.setFont(juce::Font(10.0f));
  g.setColour(juce::Colour(0, 0, 0));
  
  sprintf(number, "%8.3e", rangeMin);
  g.drawText(string(number), 0, 0, 75, 10, juce::Justification::left, false);
  
  for (int i = 1; i < numTicks; ++i)
  {
    sprintf(number, "%8.3e", rangeMin + i * div);
    g.drawText(string(number), i * width / numTicks, 0, 75, 10, juce::Justification::left, false);
    g.drawVerticalLine(i * width / numTicks, 12.0, (float)height);
  }
  sprintf(number, "%8.3e", rangeMax);
  g.drawText(string(number), width - 75, 0, 75, 0, juce::Justification::right, false);
  return res;
}

juce::Colour ColourScale::getColourFromScale(double normalizedValue) const
{
  if (normalizedValue < 0.01f)
    return juce::Colour((float)normalizedValue, 1.f, 1.f - (float)normalizedValue, 1.f);
  else
    return juce::Colour((float)normalizedValue, 1.f, (1.f - (float)normalizedValue) * 0.66f, 1.f);
}

juce::Colour LinearColourScale::getColour(double value) const
  {return getColourFromScale(normalize(value, rangeMin, rangeMax, 0.0, 1.0));}

void PercentileBasedColourScale::initialize(TablePtr data)
{
  ColourScale::initialize(data);
  std::multiset<double> values;
  for (size_t r = 0; r < data->getNumRows(); ++r)
    for (size_t c = 0; c < data->getNumColumns(); ++c)
      values.insert(Double::get(data->getElement(r,c)));

  size_t numValues = values.size();
  jassert(values.size() > 0);
  p0 = *values.begin();
  p100 = *values.rbegin();

  size_t i25 = numValues / 4;
  size_t i50 = numValues / 2;
  size_t i75 = 3 * numValues / 4;

  size_t i = 0;
  for (std::multiset<double>::const_iterator it = values.begin(); it != values.end(); ++it, ++i)
    if (i == i25)
      p25 = *it;
    else if (i == i50)
      p50 = *it;
    else if (i == i75)
      p75 = *it;
}

juce::Colour PercentileBasedColourScale::getColour(double value) const
{
  double normalizedValue = 1.0;
  if (value < p0)
    normalizedValue = 0.0;
  else if (value < p25)
    normalizedValue = normalize(value, p0, p25, 0.0, 0.25);
  else if (value < p50)
    normalizedValue = normalize(value, p25, p50, 0.25, 0.5);
  else if (value < p75)
    normalizedValue = normalize(value, p50, p75, 0.5, 0.75);
  else if (value < p100)
    normalizedValue = normalize(value, p75, p100, 0.75, 1.0);
  return getColourFromScale(normalizedValue);
}

void ContourPlotDrawable::makeAxes(double xmin, double xmax, double ymin, double ymax)
{
  xAxis = new PlotAxis(xmin, xmax, T("x0"), false);
  yAxis = new PlotAxis(ymin, ymax, T("x1"), false);
  computeBounds();
}

juce::Drawable* ContourPlotDrawable::createCopy() const
{
  ContourPlotDrawable* res = new ContourPlotDrawable();
  res->data = data;
  res->xAxis = xAxis;
  res->yAxis = yAxis;
  res->colourScale = colourScale;
  return res;
}

void ContourPlotDrawable::setData(TablePtr data)
{
  jassert(data->getNumRows() >= 1);
  jassert(data->getNumColumns() >= 1);
  this->data = data;
}

juce::AffineTransform ContourPlotDrawable::getContourPlotTransform(const juce::AffineTransform& transform) const
{
  juce::Rectangle rect = getFrameRectangle(transform);
  int contourPlotYPos = rect.getY() + colourBarHeight + colourBarMargin;
  int contourPlotHeight = rect.getHeight() - colourBarHeight - colourBarMargin;
  return transform.scaled(1.0, (float)contourPlotHeight / rect.getHeight()).translated(0.0, (float)(colourBarHeight + colourBarMargin));
}

void ContourPlotDrawable::draw(juce::Graphics& g, const juce::AffineTransform& transform) const
{
  jassert(data);
  jassert(colourScale);
  colourScale->initialize(data);
  if (!areBoundsValid())
    return;

  juce::Rectangle rect = getFrameRectangle(transform);
    
  int w = data->getNumColumns();
  int h = data->getNumRows();

  juce::Image* imgLayer = new juce::Image(juce::Image::RGB, w, h, true);
  juce::Graphics gLayer(*imgLayer);
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
    {
      gLayer.setColour(colourScale->getColour(Double::get(data->getElement(y, x))));
      gLayer.setPixel(x, y);
    }

  int contourPlotYPos = rect.getY() + colourBarHeight + colourBarMargin;
  int contourPlotHeight = rect.getHeight() - colourBarHeight - colourBarMargin;
  
  g.drawImage(imgLayer, rect.getX(), contourPlotYPos, rect.getWidth(), contourPlotHeight, 0, 0, w, h);
  g.drawImage(colourScale->getColourBar(rect.getWidth(), colourBarHeight), rect.getX(), rect.getY(), rect.getWidth(), colourBarHeight, 0, 0, rect.getWidth(), colourBarHeight);
  TwoDimensionalPlotDrawable::draw(g, getContourPlotTransform(transform));
  delete imgLayer;
}
