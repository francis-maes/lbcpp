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
# include <lbcpp/UserInterface/PlotAxis.h>
# include <lbcpp/Execution/ExecutionTrace.h>

namespace lbcpp
{

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

  String getLabel() const
    {return label;}

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

  PlotAxisPtr getXAxis() const
    {return xAxis;}

  PlotAxisPtr getYAxis() const
    {return yAxis;}
  
  void setXAxis(PlotAxisPtr axis)
    {xAxis = axis;}

  void setYAxis(PlotAxisPtr axis)
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
  PlotAxisPtr xAxis;
  PlotAxisPtr yAxis;
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
