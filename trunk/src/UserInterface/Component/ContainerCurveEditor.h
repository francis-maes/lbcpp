/*-----------------------------------------.---------------------------------.
| Filename: ContainerCurveEditor.h         | Container Curve Editor          |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_CURVE_EDITOR_H_
# define LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_CURVE_EDITOR_H_

# include <lbcpp/UserInterface/ObjectEditor.h>
# include <lbcpp/Execution/ExecutionTrace.h>

namespace lbcpp
{
class CurveAxisConfiguration : public Object
{
public:
  CurveAxisConfiguration(double rangeMin, double rangeMax, const String& label = String::empty, bool autoRange = true)
    : rangeMin(rangeMin), rangeMax(rangeMax), autoRange(autoRange), logScale(false), label(label) {}
  CurveAxisConfiguration() : rangeMin(0.0), rangeMax(0.0), autoRange(true), logScale(false) {}

  bool hasAutoRange() const
    {return autoRange;}

  bool hasLogScale() const
    {return logScale;}

  double getRangeMin() const
    {return rangeMin;}

  double getRangeMax() const
    {return rangeMax;}

  String getLabel() const
    {return label;}

private:
  friend class CurveAxisConfigurationClass;

  double rangeMin, rangeMax;
  bool autoRange;
  bool logScale;
  String unit;
  String label;
};

typedef ReferenceCountedObjectPtr<CurveAxisConfiguration> CurveAxisConfigurationPtr;

class CurveVariableConfiguration : public Object
{
public:
  CurveVariableConfiguration(bool selected, const juce::Colour& colour, const String& label)
    : selected(selected), colour(colour), label(label) {}
  CurveVariableConfiguration() : selected(false) {}

  bool& isSelected()
    {return selected;}

  bool isSelected() const
    {return selected;}

  void setSelected(bool selected)
    {this->selected = selected;}

  const juce::Colour& getColour() const
    {return colour;}

private:
  friend class CurveVariableConfigurationClass;

  bool selected;
  juce::Colour colour;
  String label;
};

typedef ReferenceCountedObjectPtr<CurveVariableConfiguration> CurveVariableConfigurationPtr;

class ContainerCurveEditorConfiguration : public Object
{
public:
  ContainerCurveEditorConfiguration(ClassPtr rowType);
  ContainerCurveEditorConfiguration() {}
  
  std::vector<size_t> getSelectedCurves() const;

  TypePtr getRowType() const
    {return rowType;}

  CurveAxisConfigurationPtr getXAxis() const
    {return xAxis;}

  CurveAxisConfigurationPtr getYAxis() const
    {return yAxis;}
  
  void setXAxis(CurveAxisConfigurationPtr axis)
    {xAxis = axis;}

  void setYAxis(CurveAxisConfigurationPtr axis)
    {yAxis = axis;}

  size_t getNumCurves() const
    {return rowType->getNumMemberVariables();}

  CurveVariableConfigurationPtr getCurve(size_t index) const
    {jassert(index < variables.size()); return variables[index];}

  size_t getKeyVariableIndex() const
    {return keyVariableIndex;}

  void setKeyVariableIndex(size_t index)
    {keyVariableIndex = index;}

protected:
  friend class ContainerCurveEditorConfigurationClass;

  TypePtr rowType;
  CurveAxisConfigurationPtr xAxis;
  CurveAxisConfigurationPtr yAxis;
  std::vector<CurveVariableConfigurationPtr> variables;
  size_t keyVariableIndex;
};

typedef ReferenceCountedObjectPtr<ContainerCurveEditorConfiguration> ContainerCurveEditorConfigurationPtr;

class ContainerCurveEditor : public ObjectEditor
{
public:
  ContainerCurveEditor(ExecutionContext& context, ContainerPtr table, ContainerCurveEditorConfigurationPtr configuration);

protected:
  ContainerPtr table;

  virtual Component* createConfigurationComponent(const ObjectPtr& configuration);
  virtual Component* createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration);
};

typedef ReferenceCountedObjectPtr<ContainerCurveEditor> ContainerCurveEditorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_CURVE_EDITOR_H_
