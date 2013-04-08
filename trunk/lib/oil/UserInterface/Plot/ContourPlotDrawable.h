/*-----------------------------------------.---------------------------------.
| Filename: ContourPlot.h                  | Contour Plot                    |
| Author  : Denny Verbeeck                 |                                 |
| Started : 05/04/2013 15:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _CONTOURPLOT_H_

# include "TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

class ColourScale
{
public:
  virtual juce::Colour getColour(double value) const = 0;
  /**
   * Create an image with the current colormap and scale
   * \param width The width of the image in pixels
   * \param height The height of the image in pixels (must be greater than 12)
   * \param numTicks The number of annotated ticks on the colorbar (default = 4)
   * \return A juce::Image* containing the colormap
   */
  virtual juce::Image* getColourBar(size_t width, size_t height, int numTicks = 4) const;
  virtual void initialize(TablePtr data);
  enum Type
  {
    linear = 0,
    percentile
  };
protected:
  double rangeMin, rangeMax;
  double normalize(double value, double lower, double upper, double l, double u) const;
  juce::Colour getColourFromScale(double normalizedValue) const;
};

class LinearColourScale : public ColourScale
{
public:
  virtual juce::Colour getColour(double value) const;
};

class PercentileBasedColourScale : public ColourScale
{
public:
  virtual void initialize(TablePtr data);
  virtual juce::Colour getColour(double value) const;
private:
  double p0, p25, p50, p75, p100;
};

class ContourPlotDrawable : public TwoDimensionalPlotDrawable
{
public:
  ContourPlotDrawable() : data(NULL), xAxis(NULL), yAxis(NULL), colourScale(NULL),
                          colourBarHeight(30), colourBarMargin(10) {}
  ~ContourPlotDrawable()
    {delete colourScale;}

  void setData(TablePtr data);
  void setColourScale(ColourScale* scale)
    {colourScale = scale;}
  void makeAxes(double xmin, double xmax, double ymin, double ymax);
  juce::AffineTransform getContourPlotTransform(const juce::AffineTransform& transform) const;

  virtual Drawable* createCopy() const;

  virtual PlotAxisPtr getXAxis() const
    {return xAxis;}

  virtual PlotAxisPtr getYAxis() const
    {return yAxis;}

  virtual void draw(juce::Graphics& g, const juce::AffineTransform& transform = juce::AffineTransform::identity) const;

protected:
  TablePtr data;
  PlotAxisPtr xAxis, yAxis;
  ColourScale* colourScale;
  const int colourBarHeight;
  const int colourBarMargin;
};

} /* namespace lbcpp */

#endif // !_CONTOURPLOT_H_
