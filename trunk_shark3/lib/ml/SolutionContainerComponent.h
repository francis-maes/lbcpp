/*-----------------------------------------.---------------------------------.
| Filename: SolutionContainerComponent.h   | User Interface for Solution sets|
| Author  : Francis Maes                   |                                 |
| Started : 11/09/2012 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLUTION_SET_COMPONENT_H_
# define ML_SOLUTION_SET_COMPONENT_H_

# include <ml/SolutionComparator.h>
# include <ml/SolutionContainer.h>
# include <oil/UserInterface/ObjectComponent.h>
# include "../oil/UserInterface/Plot/TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

class SolutionVectorDrawable : public TwoDimensionalPlotDrawable
{
public:
  SolutionVectorDrawable(SolutionVectorPtr solutions) : solutions(solutions)
  {
    FitnessLimitsPtr fitnessLimits = solutions->getFitnessLimits();
    FitnessLimitsPtr empiricalLimits = solutions->getEmpiricalFitnessLimits();
    xAxis = makeAxis(fitnessLimits, empiricalLimits, 0);
    yAxis = makeAxis(fitnessLimits, empiricalLimits, 1);
    computeBounds();
  }

  virtual Drawable* createCopy() const
    {return new SolutionVectorDrawable(solutions);}

  virtual PlotAxisPtr getXAxis() const
    {return xAxis;}

  virtual PlotAxisPtr getYAxis() const
    {return yAxis;}

  virtual void draw(juce::Graphics& g, const juce::AffineTransform& transform = juce::AffineTransform::identity) const
  {
    TwoDimensionalPlotDrawable::draw(g, transform);
    if (!areBoundsValid())
      return;

    std::vector<ParetoFrontPtr> fronts = solutions->nonDominatedSort();
    for (size_t i = 0; i < fronts.size(); ++i)
    {
      ParetoFrontPtr front = fronts[i];

      g.setColour(juce::Colour(i / (float)fronts.size(), 0.5f, 1.f, (juce::uint8)255));
      for (size_t j = 0; j < front->getNumSolutions() - 1; ++j)
      {
        int x1, y1, x2, y2, x3, y3;
        getPixelPosition(front->getFitness(j), x1, y1, transform);
        getPixelPosition(Fitness::makeWorstCombination(front->getFitness(j), front->getFitness(j + 1)), x2, y2, transform);
        getPixelPosition(front->getFitness(j + 1), x3, y3, transform);
        g.drawLine((float)x1, (float)y1, (float)x2, (float)y2);
        g.drawLine((float)x2, (float)y2, (float)x3, (float)y3);
      }
    }

    for (size_t i = 0; i < solutions->getNumSolutions(); ++i)
    {
      FitnessPtr fitness = solutions->getFitness(i);
      int x, y;
      getPixelPosition(fitness, x, y, transform);
      g.setColour(fitness == currentFitness ? juce::Colours::red : juce::Colours::black);
      paintPoint(g, x, y);
    }
  }

  int hittest(int x, int y, const juce::AffineTransform& transform) const
  {
    double minDistance = DBL_MAX;
    int res = -1;

    for (size_t i = 0; i < solutions->getNumSolutions(); ++i)
    {
      FitnessPtr fitness = solutions->getFitness(i);
      int ox, oy;
      getPixelPosition(fitness, ox, oy, transform);
      double distance = sqrt((double)((x - ox) * (x - ox) + (y - oy) * (y - oy)));
      if (distance < minDistance)
      {
        minDistance = distance;
        res = (int)i;
      }
    }

    const int hittestRadius = 7;
    return minDistance < hittestRadius ? res : -1;
  }

  void setCurrentFitness(const FitnessPtr& fitness)
    {currentFitness = fitness;}

  const FitnessPtr& getCurrentFitness() const
    {return currentFitness;}

protected:
  SolutionVectorPtr solutions;
  PlotAxisPtr xAxis, yAxis;
  FitnessPtr currentFitness;

  void getPixelPosition(FitnessPtr fitness, int& x, int& y, const juce::AffineTransform& transform) const
  {
    double dx = fitness->getValue(0);
    double dy = fitness->getValue(1);
    transform.transformPoint(dx, dy);
    x = (int)dx;
    y = (int)dy;
  }

  void paintPoint(juce::Graphics& g, int x, int y) const
  {
    if (solutions->getNumSolutions() < 200)
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
    else
      g.setPixel(x, y);
  }

  PlotAxisPtr makeAxis(FitnessLimitsPtr theoreticalLimits, FitnessLimitsPtr empiricalLimits, size_t index)
  {
    /*
    double lower = theoreticalLimits->getLowerLimit(index);
    double upper = theoreticalLimits->getUpperLimit(index);
    if (lower > upper)
      {double tmp = lower; lower = upper; upper = tmp;}

    double empiricalRange = empiricalLimits->getUpperLimit(index) - empiricalLimits->getLowerLimit(index);
    if (empiricalLimits->getLowerLimit(index) < lower || !isNumberValid(lower))
      lower = empiricalLimits->getLowerLimit(index) - empiricalRange / 10.0;
    if (empiricalLimits->getUpperLimit(index) > upper || !isNumberValid(upper))
      upper = empiricalLimits->getUpperLimit(index) + empiricalRange / 10.0;
      */

    double lower = empiricalLimits->getLowerLimit(index);
    double upper = empiricalLimits->getUpperLimit(index);
    double range = upper - lower;
    lower -= range / 10.0;
    upper += range / 10.0;
    return new PlotAxis(lower, upper, "F" + string((int)index + 1), false);
  }
};

class SolutionVectorComponent : public juce::Component, public ComponentWithPreferedSize, public ObjectSelector
{
public:
  SolutionVectorComponent(SolutionVectorPtr solutions, const string& name)
    : solutions(solutions->sort(lexicographicComparator())), drawable(NULL), selectedIndex(-1)
  {
    setWantsKeyboardFocus(true);
    if (solutions->getNumObjectives() == 2)
      drawable = new SolutionVectorDrawable(this->solutions);
  }
  virtual ~SolutionVectorComponent()
    {if (drawable) delete drawable;}

  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getDefaultHeight() const
    {return 600;}

  virtual void paint(juce::Graphics& g)
  {
    if (solutions->isEmpty())
    {
      paintText(g, "Empty Pareto Front");
      return;
    }
    if (!drawable)
    {
      paintText(g, "Only the display of bi-objective pareto solutionss is supported yet");
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
  SolutionVectorPtr solutions;
  SolutionVectorDrawable* drawable;
  int selectedIndex;

  void paintText(juce::Graphics& g, const string& text)
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

  void select(int index)
  {
    if (index >= 0)
      sendSelectionChanged(solutions->getSolution(index), T("solution"));
    else
      sendSelectionChanged(std::vector<ObjectPtr>(), string::empty);

    selectedIndex = index;
    if (drawable)
      drawable->setCurrentFitness(selectedIndex >= 0 ? solutions->getFitness(selectedIndex) : FitnessPtr());
    repaint();
  }

  void changeSelection(bool gotoRight)
  {
    if (!drawable || selectedIndex < 0)
      return;
    
    if (gotoRight)
    {
      if (selectedIndex < (int)solutions->getNumSolutions() - 1)
        select(selectedIndex + 1);
    }
    else
    {
      if (selectedIndex > 0)
        select(selectedIndex - 1);
    }
  }
};

}; /* namespace lbcpp */

#endif // !ML_SOLUTION_SET_COMPONENT_H_
