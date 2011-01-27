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
  CurveAxisConfiguration(double rangeMin, double rangeMax, const String& label)
    : rangeMin(rangeMin), rangeMax(rangeMax), autoRange(true), logScale(false), label(label) {}
  CurveAxisConfiguration() : rangeMin(0.0), rangeMax(0.0), autoRange(true), logScale(false) {}

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
  ContainerCurveEditorConfiguration(ClassPtr type)
    : xAxis(new CurveAxisConfiguration(0.0, 1000.0, T("X Axis"))),
      yAxis(new CurveAxisConfiguration(0.0, 1.0, T("Y Axis"))),
      variables(type->getObjectNumVariables())
  {
    for (size_t i = 0; i < variables.size(); ++i)
      variables[i] = new CurveVariableConfiguration(true, Colours::black, type->getObjectVariableName(i));
    keyVariableIndex = 0;
  }

  ContainerCurveEditorConfiguration() {}
  
protected:
  friend class ContainerCurveEditorConfigurationClass;

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

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_CURVE_EDITOR_H_
