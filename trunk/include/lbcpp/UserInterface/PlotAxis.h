/*-----------------------------------------.---------------------------------.
| Filename: PlotAxis.h                     | Plot Axis                       |
| Author  : Francis Maes                   |                                 |
| Started : 11/09/2012 19:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_PLOT_AXIS_H_
# define LBCPP_USER_INTERFACE_PLOT_AXIS_H_

# include "../Core/Object.h"

namespace lbcpp
{

class PlotAxis : public Object
{
public:
  PlotAxis(double rangeMin, double rangeMax, const String& label = String::empty, bool autoRange = true)
    : rangeMin(rangeMin), rangeMax(rangeMax), autoRange(autoRange), logScale(false), label(label) {}
  PlotAxis() : rangeMin(0.0), rangeMax(0.0), autoRange(true), logScale(false) {}

  bool hasAutoRange() const
    {return autoRange;}

  void setAutoRange(bool value)
    {autoRange = value;}

  bool hasLogScale() const
    {return logScale;}

  double getRangeMin() const
    {return rangeMin;}

  double getRangeMax() const
    {return rangeMax;}

  String getLabel() const
    {return label;}

  void setRangeMin(double value)
    {rangeMin = value;}

  void setRangeMax(double value)
    {rangeMax = value;}

  void setRange(double minValue, double maxValue)
    {rangeMin = minValue; rangeMax = maxValue;}

  void increaseRange(double percentAdded)
  {
    double i = (rangeMax - rangeMin) * percentAdded / 2.0;
    rangeMin -= i;
    rangeMax += i;
  }

private:
  friend class PlotAxisClass;

  double rangeMin, rangeMax;
  bool autoRange;
  bool logScale;
  String unit;
  String label;
};

typedef ReferenceCountedObjectPtr<PlotAxis> PlotAxisPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_AXIS_H_
