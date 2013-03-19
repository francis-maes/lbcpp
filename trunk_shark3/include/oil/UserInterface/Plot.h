/*-----------------------------------------.---------------------------------.
| Filename: Plot.h                         | Plot                            |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_PLOT_H_
# define LBCPP_USER_INTERFACE_PLOT_H_

# include "../Core/Object.h"
# include "../Core/Table.h"

namespace lbcpp
{

class PlotVariable : public Object
{
public:
  PlotVariable(const ObjectPtr& key, bool selected, const juce::Colour& colour)
    : key(key), selected(selected), colour(colour) {}
  PlotVariable() : selected(false) {}

  const ObjectPtr& getKey() const
    {return key;}

  string getName() const
    {return key->toShortString();}

  bool& isSelected()
    {return selected;}

  bool isSelected() const
    {return selected;}

  void setSelected(bool selected)
    {this->selected = selected;}

  const juce::Colour& getColour() const
    {return colour;}

private:
  friend class PlotVariableClass;

  ObjectPtr key;
  bool selected;
  juce::Colour colour;
};

typedef ReferenceCountedObjectPtr<PlotVariable> PlotVariablePtr;

class PlotAxis : public Object
{
public:
  PlotAxis(double rangeMin, double rangeMax, const string& label = string::empty, bool autoRange = true)
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

  string getLabel() const
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
  string unit;
  string label;
};

typedef ReferenceCountedObjectPtr<PlotAxis> PlotAxisPtr;

class Plot : public Object
{
public:
  Plot(TablePtr data);
  Plot() {}
  
  std::vector<size_t> getSelectedCurves() const;

  TablePtr getData() const
    {return data;}

  PlotAxisPtr getXAxis() const
    {return xAxis;}

  PlotAxisPtr getYAxis() const
    {return yAxis;}
  
  void setXAxis(PlotAxisPtr axis)
    {xAxis = axis;}

  void setYAxis(PlotAxisPtr axis)
    {yAxis = axis;}

  size_t getNumPlotVariables() const
    {return variables.size();}

  PlotVariablePtr getPlotVariable(size_t index) const
    {jassert(index < variables.size()); return variables[index];}

  size_t getKeyVariableIndex() const
    {return keyVariableIndex;}

  void setKeyVariableIndex(size_t index)
    {keyVariableIndex = index;}

protected:
  friend class PlotClass;

  TablePtr data;
  PlotAxisPtr xAxis;
  PlotAxisPtr yAxis;
  std::vector<PlotVariablePtr> variables;
  size_t keyVariableIndex;
};

typedef ReferenceCountedObjectPtr<Plot> PlotPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_H_
