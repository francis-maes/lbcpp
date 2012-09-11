/*-----------------------------------------.---------------------------------.
| Filename: MOOParetoFrontComponent.h      | User Interface for Pareto Fronts|
| Author  : Francis Maes                   |                                 |
| Started : 11/09/2012 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PARETO_FRONT_COMPONENT_H_
# define LBCPP_MOO_PARETO_FRONT_COMPONENT_H_

# include "MOOCore.h"
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include "../../../src/UserInterface/Plot/TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

class MOOParetoFrontDrawable : public TwoDimensionalPlotDrawable
{
public:
  MOOParetoFrontDrawable(MOOParetoFrontPtr front) : front(front)
  {
    MOOFitnessLimitsPtr theoreticalLimits = front->getTheoreticalLimits();
    MOOFitnessLimitsPtr empiricalLimits = front->getEmpiricalLimits();
    xAxis = makeAxis(theoreticalLimits, empiricalLimits, 0);
    yAxis = makeAxis(theoreticalLimits, empiricalLimits, 1);
    computeBounds();
  }

  virtual Drawable* createCopy() const
    {return new MOOParetoFrontDrawable(front);}

  virtual PlotAxisPtr getXAxis() const
    {return xAxis;}

  virtual PlotAxisPtr getYAxis() const
    {return yAxis;}

  virtual void draw(juce::Graphics& g, const juce::AffineTransform& transform = juce::AffineTransform::identity) const
  {
    TwoDimensionalPlotDrawable::draw(g, transform);
    if (!areBoundsValid())
      return;

    const MOOParetoFront::ParetoMap& m = front->getParetoMap();
    g.setColour(juce::Colours::black);
    int baseSize = juce::jmin(g.getClipBounds().getWidth() / 10, g.getClipBounds().getHeight() / 10);
    for (MOOParetoFront::ParetoMap::const_iterator it = m.begin(); it != m.end(); ++it)
    {
      double x = it->first->getObjective(0);
      double y = it->first->getObjective(1);
      transform.transformPoint(x, y);
      paintPoint(g, (int)x, (int)y, baseSize);
    }
  }

protected:
  MOOParetoFrontPtr front;
  PlotAxisPtr xAxis, yAxis;

  void paintPoint(juce::Graphics& g, int x, int y, int baseSize) const
  {
    enum {s = 5};
    int pointHalfSize = baseSize / 10;
    float x1 = (float)(x - pointHalfSize);
    float y1 = (float)(y - pointHalfSize);
    float x2 = (float)(x + pointHalfSize);
    float y2 = (float)(y + pointHalfSize);
    float lineWidth = juce::jmax(1.f, baseSize / 20.f);
    g.drawLine(x1, y1, x2 + 1.f, y2 + 1.f, lineWidth);
    g.drawLine(x1, y2, x2 + 1.f, y1 + 1.f, lineWidth);
  }

  PlotAxisPtr makeAxis(MOOFitnessLimitsPtr theoreticalLimits, MOOFitnessLimitsPtr empiricalLimits, size_t index)
  {
    double lower = theoreticalLimits->getLowerLimit(index);
    double upper = theoreticalLimits->getUpperLimit(index);
    if (lower > upper)
      {double tmp = lower; lower = upper; upper = tmp;}
    double empiricalRange = empiricalLimits->getUpperLimit(index) - empiricalLimits->getLowerLimit(index);
    if (!isNumberValid(lower))
      lower = empiricalLimits->getLowerLimit(index) - empiricalRange / 10.0;
    if (!isNumberValid(upper))
      upper = empiricalLimits->getUpperLimit(index) + empiricalRange / 10.0;
    return new PlotAxis(lower, upper, "F" + String((int)index + 1), false);
  }
};

class MOOParetoFrontComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  MOOParetoFrontComponent(MOOParetoFrontPtr front, const String& name)
    : front(front)
  {
  }

  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getDefaultHeight() const
    {return 600;}

  virtual void paint(juce::Graphics& g)
  {
    const MOOParetoFront::ParetoMap& m = front->getParetoMap();
    if (m.empty())
    {
      paintText(g, "Empty Pareto Front");
      return;
    }
    if (front->getNumObjectives() != 2)
    {
      paintText(g, "Only the display of bi-objective pareto fronts is supported yet");
      return;
    }

    enum
    {
      leftMargin = 60,
      rightMargin = 20,
      topMargin = 20,
      bottomMargin = 40
    };

    MOOParetoFrontDrawable drawable(front);
    drawable.drawWithin(g, leftMargin, topMargin, getWidth() - leftMargin - rightMargin, getHeight() - topMargin - bottomMargin, juce::RectanglePlacement::stretchToFit);
  }

protected:
  MOOParetoFrontPtr front;

  void paintText(juce::Graphics& g, const String& text)
  {
    g.setColour(juce::Colours::grey);
    g.drawText(text, 0, 0, getWidth(), getHeight(), juce::Justification::centred, true);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PARETO_FRONT_COMPONENT_H_
