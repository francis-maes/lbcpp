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
# include <lbcpp/UserInterface/VariableSelector.h>
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

    const MOOParetoFront::ParetoMap& m = front->getMap();
    g.setColour(juce::Colours::lightgrey);
    MOOParetoFront::ParetoMap::const_iterator it, nxt;
    for (it = m.begin(); it != m.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (nxt == m.end())
        break;
      int x1, y1, x2, y2, x3, y3;
      getPixelPosition(it->first, x1, y1, transform);
      getPixelPosition(MOOFitness::makeWorstCombination(it->first, nxt->first), x2, y2, transform);
      getPixelPosition(nxt->first, x3, y3, transform);
      g.drawLine((float)x1, (float)y1, (float)x2, (float)y2);
      g.drawLine((float)x2, (float)y2, (float)x3, (float)y3);
    }

    for (it = m.begin(); it != m.end(); ++it)
    {
      int x, y;
      getPixelPosition(it->first, x, y, transform);
      g.setColour(it->first == currentFitness ? juce::Colours::red : juce::Colours::black);
      paintPoint(g, x, y);
    }
  }

  MOOFitnessPtr hittest(int x, int y, const juce::AffineTransform& transform) const
  {
    double minDistance = DBL_MAX;
    MOOFitnessPtr res;

    const MOOParetoFront::ParetoMap& m = front->getMap();
    for (MOOParetoFront::ParetoMap::const_iterator it = m.begin(); it != m.end(); ++it)
    {
      int ox, oy;
      getPixelPosition(it->first, ox, oy, transform);
      double distance = sqrt((double)((x - ox) * (x - ox) + (y - oy) * (y - oy)));
      if (distance < minDistance)
      {
        minDistance = distance;
        res = it->first;
      }
    }

    const int hittestRadius = 7;
    return minDistance < hittestRadius ? res : MOOFitnessPtr();
  }

  void setCurrentFitness(const MOOFitnessPtr& fitness)
    {currentFitness = fitness;}

  const MOOFitnessPtr& getCurrentFitness() const
    {return currentFitness;}

protected:
  MOOParetoFrontPtr front;
  PlotAxisPtr xAxis, yAxis;
  MOOFitnessPtr currentFitness;

  void getPixelPosition(MOOFitnessPtr fitness, int& x, int& y, const juce::AffineTransform& transform) const
  {
    double dx = fitness->getValue(0);
    double dy = fitness->getValue(1);
    transform.transformPoint(dx, dy);
    x = (int)dx;
    y = (int)dy;
  }

  void paintPoint(juce::Graphics& g, int x, int y) const
  {
    const int pointHalfSize = 4;
    const float lineWidth = 2.f;

    float x1 = (float)(x - pointHalfSize);
    float y1 = (float)(y - pointHalfSize);
    float x2 = (float)(x + pointHalfSize);
    float y2 = (float)(y + pointHalfSize);
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
    if (empiricalLimits->getLowerLimit(index) < lower || !isNumberValid(lower))
      lower = empiricalLimits->getLowerLimit(index) - empiricalRange / 10.0;
    if (empiricalLimits->getUpperLimit(index) > upper || !isNumberValid(upper))
      upper = empiricalLimits->getUpperLimit(index) + empiricalRange / 10.0;

    return new PlotAxis(lower, upper, "F" + String((int)index + 1), false);
  }
};

class MOOParetoFrontComponent : public juce::Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  MOOParetoFrontComponent(MOOParetoFrontPtr front, const String& name)
    : front(front), drawable(NULL)
  {
    setWantsKeyboardFocus(true);
    if (front->getNumObjectives() == 2)
      drawable = new MOOParetoFrontDrawable(front);
  }
  virtual ~MOOParetoFrontComponent()
    {if (drawable) delete drawable;}

  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getDefaultHeight() const
    {return 600;}

  virtual void paint(juce::Graphics& g)
  {
    const MOOParetoFront::ParetoMap& m = front->getMap();
    if (m.empty())
    {
      paintText(g, "Empty Pareto Front");
      return;
    }
    if (!drawable)
    {
      paintText(g, "Only the display of bi-objective pareto fronts is supported yet");
      return;
    }

    if (getWidth() > leftMargin + rightMargin && getHeight() > topMargin + bottomMargin)
      drawable->draw(g, getTransform());
  }

  virtual bool keyPressed(const juce::KeyPress& key)
  {
    if (key.getKeyCode() == juce::KeyPress::leftKey)
    {
      changeSelection(false);
      return true;
    }
    if (key.getKeyCode() == juce::KeyPress::rightKey)
    {
      changeSelection(true);
      return true;
    }
    return false;
  }

  virtual void mouseUp(const juce::MouseEvent& e)
  {
    if (drawable)
      select(drawable->hittest(e.getMouseDownX(), e.getMouseDownY(), getTransform()));
  }

protected:
  MOOParetoFrontPtr front;
  MOOParetoFrontDrawable* drawable;

  void paintText(juce::Graphics& g, const String& text)
  {
    g.setColour(juce::Colours::grey);
    g.drawText(text, 0, 0, getWidth(), getHeight(), juce::Justification::centred, true);
  }

  enum
  {
    leftMargin = 60,
    rightMargin = 20,
    topMargin = 20,
    bottomMargin = 40
  };

  juce::AffineTransform getTransform() const
  {
    const int destX = leftMargin;
    const int destY = topMargin;
    const int destW = getWidth() - leftMargin - rightMargin;
    const int destH = getHeight() - topMargin - bottomMargin;

    float x, y, w, h;
    drawable->getBounds(x, y, w, h);
    juce::RectanglePlacement placement(juce::RectanglePlacement::stretchToFit);
    return placement.getTransformToFit(x, y, w, h, (float)destX, (float)destY, (float)destW, (float)destH);
  }

  void select(const MOOFitnessPtr& fitness)
  {
    if (fitness)
    {
      std::vector<ObjectPtr> solutions;
      front->getSolutionsByFitness(fitness, solutions);

      Variable res;
      if (solutions.size() == 1)
        res = Variable::pair(fitness->getVariable(0), solutions[0]);
      else
      {
        ObjectVectorPtr sol(new ObjectVector(objectClass, 0));
        for (size_t i = 0; i < solutions.size(); ++i)
          sol->append(solutions[i]);
        res = Variable::pair(fitness->getVariable(0), sol);
      }
      sendSelectionChanged(res, T("pareto point"));
    }
    else
      sendSelectionChanged(std::vector<Variable>(), String::empty);

    if (drawable)
      drawable->setCurrentFitness(fitness);
    repaint();
  }

  void changeSelection(bool gotoRight)
  {
    if (!drawable)
      return;
    const MOOFitnessPtr& current = drawable->getCurrentFitness();
    if (!current)
      return;
    const MOOParetoFront::ParetoMap& m = front->getMap();
    MOOParetoFront::ParetoMap::const_iterator it = m.find(current);
    if (it == m.end())
      return;
    if (gotoRight)
    {
      ++it;
      if (it != m.end())
        select(it->first);
    }
    else
    {
      if (it != m.begin())
      {
        --it;
        select(it->first);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PARETO_FRONT_COMPONENT_H_
