/*-----------------------------------------.---------------------------------.
| Filename: TwoDimensionalPlotDrawable.h   | 2-D Plot Drawable base class    |
| Author  : Francis Maes                   |                                 |
| Started : 11/09/2012 19:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_PLOT_TWO_DIMENSIONAL_DRAWABLE_H_
# define LBCPP_USER_INTERFACE_PLOT_TWO_DIMENSIONAL_DRAWABLE_H_

# include <lbcpp/UserInterface/Plot.h>
# include <lbcpp/Core.h>

namespace lbcpp
{

class TwoDimensionalPlotDrawable : public juce::Drawable
{
public:
  TwoDimensionalPlotDrawable();

  virtual PlotAxisPtr getXAxis() const = 0;
  virtual PlotAxisPtr getYAxis() const = 0;

  virtual void computeXAxisAutoRange(PlotAxisPtr axis) const
    {jassertfalse;}

  virtual void computeYAxisAutoRange(PlotAxisPtr axis) const
    {jassertfalse;}

  virtual String getXAxisLabel() const
    {return getXAxis()->getLabel();}

  virtual String getYAxisLabel() const
    {return getYAxis()->getLabel();}

  enum
  {
    pointCrossSize = 6,
    frameMarkerSize1 = 8,
    frameMarkerSize2 = 5,
    bottomValuesSize = 20,
  };

  virtual bool hitTest(float x, float y) const;
  virtual void getBounds(float& x, float& y, float& width, float& height) const;
  virtual void draw(juce::Graphics& g, const juce::AffineTransform& transform = juce::AffineTransform::identity) const;

protected:
  double boundsX, boundsY, boundsWidth, boundsHeight;

  void computeBounds();
  bool areBoundsValid() const;

  static void getMarkerValues(double minValue, double maxValue, double valuePerPixel,
                              std::vector<double>& main, std::vector<double>& secondary);

  static bool isNumberValid(double number)
    {return lbcpp::isNumberValid(number) && fabs(number) < 1e20;}

  juce::Rectangle getFrameRectangle(const juce::AffineTransform& transform) const;

  void drawFrameMarker(juce::Graphics& g, const juce::AffineTransform& transform, double value, bool isHorizontal, bool isMainMarker) const;
  void drawFrameMarkers(juce::Graphics& g, const juce::AffineTransform& transform, juce::Rectangle frameRectangle, bool isHorizontal) const;
  void drawFrame(juce::Graphics& g, const juce::AffineTransform& transform) const;
  void drawZeroAxis(juce::Graphics& g, const juce::AffineTransform& transform, PlotAxisPtr config, bool isHorizontalAxis) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_TWO_DIMENSIONAL_DRAWABLE_H_
