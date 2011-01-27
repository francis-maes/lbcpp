/*-----------------------------------------.---------------------------------.
| Filename: ContainerCurveEditor.cpp       | Container Curve Editor          |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ContainerCurveEditor.h"
using namespace lbcpp;

ContainerCurveEditor::ContainerCurveEditor(ExecutionContext& context, ContainerPtr table, ContainerCurveEditorConfigurationPtr configuration)
  : ObjectEditor(table, configuration, false, false), table(table)
{
  initialize();
}

Component* ContainerCurveEditor::createConfigurationComponent(const ObjectPtr& configuration)
{
  return new Component();
}

Component* ContainerCurveEditor::createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration)
{
  return new Component();
}
