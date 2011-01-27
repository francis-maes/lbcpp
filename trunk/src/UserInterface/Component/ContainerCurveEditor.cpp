/*-----------------------------------------.---------------------------------.
| Filename: ContainerCurveEditor.cpp       | Container Curve Editor          |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ContainerCurveEditor.h"
using namespace lbcpp;

using juce::Drawable;
using juce::AffineTransform;
using juce::Rectangle;
using juce::RectanglePlacement;

class ContainerCurveDrawable : public Drawable
{
public:
  ContainerCurveDrawable(const ContainerPtr& table, const ContainerCurveEditorConfigurationPtr& configuration)
    : table(table), configuration(configuration)
  {
    selectedCurves = configuration->getSelectedCurves();
  }

  virtual Drawable* createCopy() const
    {return new ContainerCurveDrawable(table, configuration);}

  virtual bool hitTest(float x, float y) const
    {return true;}

  virtual void getBounds(float& x, float& y, float& width, float& height) const
  {
     // FIXME
    x = -11;
    width = 22;
    y = 1.1f;
    height = -1.2f;
  }

  virtual void draw(Graphics& g, const AffineTransform& transform = AffineTransform::identity) const
  {
    drawAxisBack(g, transform, configuration->getYAxis(), false);
    drawAxisBack(g, transform, configuration->getXAxis(), true);
    for (size_t i = 0; i < selectedCurves.size(); ++i)
    {
      drawCurveLine(g, transform, selectedCurves[i], configuration->getCurve(selectedCurves[i]));
      drawCurvePoint(g, transform, selectedCurves[i], configuration->getCurve(selectedCurves[i])); 
    }
    drawAxisFront(g, transform, configuration->getYAxis(), false);
    drawAxisFront(g, transform, configuration->getXAxis(), true);
  }
  
  void drawAxisBack(Graphics& g, const AffineTransform& transform, CurveAxisConfigurationPtr config, bool isHorizontalAxis) const
  {
    float x0 = 0.f, y0 = 0.f;
    transform.transformPoint(x0, y0);

    g.setColour(Colours::black);
    Rectangle bounds = g.getClipBounds();
    if (isHorizontalAxis)
      g.drawLine((float)bounds.getX(), y0, (float)bounds.getWidth(), y0);
    else
      g.drawLine(x0, (float)bounds.getY(), x0, (float)bounds.getHeight());
  }

  void drawAxisFront(Graphics& g, const AffineTransform& transform, CurveAxisConfigurationPtr config, bool isHorizontalAxis) const
  {
  }

  void drawCurveLine(Graphics& g, const AffineTransform& transform, size_t index, CurveVariableConfigurationPtr config) const
  {
    size_t n = table->getNumElements();
    size_t keyVariableIndex = configuration->getKeyVariableIndex();
    float x0, y0;
    getPointPosition(0, keyVariableIndex, index, transform, x0, y0);

    g.setColour(config->getColour());
    for (size_t i = 1; i < n; ++i)
    {
      float x1, y1;
      getPointPosition(i, keyVariableIndex, index, transform, x1, y1);
      g.drawLine(x0, y0, x1, y1);
      x0 = x1;
      y0 = y1;
    }
  }

  void drawCurvePoint(Graphics& g, const AffineTransform& transform, size_t index, CurveVariableConfigurationPtr config) const
  {
    size_t n = table->getNumElements();
    size_t keyVariableIndex = configuration->getKeyVariableIndex();

    g.setColour(config->getColour());
    for (size_t i = 0; i < n; ++i)
    {
      float x, y;
      float crossHalfSize = 3.f;
      getPointPosition(i, keyVariableIndex, index, transform, x, y);
      g.drawLine(x - crossHalfSize, y, x + crossHalfSize, y);
      g.drawLine(x, y - crossHalfSize, x, y + crossHalfSize);
    }
  }

protected:
  ContainerPtr table;
  ContainerCurveEditorConfigurationPtr configuration;

  std::vector<size_t> selectedCurves;

  void getPointPosition(size_t row, size_t columnX, size_t columnY, const AffineTransform& transform, float& x, float& y) const
  {
    ObjectPtr rowObject = table->getElement(row).getObject();
    x = (float)getTableValue(rowObject, columnX);
    y = (float)getTableValue(rowObject, columnY);
    transform.transformPoint(x, y);
  }

  double getTableValue(const ObjectPtr& row, size_t column) const
  {
    Variable value = row->getVariable(column);
    if (!value.exists())
      return 0.0;
    if (value.isInteger())
      return (double)value.getInteger();
    else if (value.isDouble())
      return value.getDouble();
    else
    {
      jassert(false);
      return 0.0;
    }
  }
};

class ContainerCurveEditorContentComponent : public Component
{
public:
  ContainerCurveEditorContentComponent(const ContainerPtr& table, const ContainerCurveEditorConfigurationPtr& configuration)
  {
    drawable = new ContainerCurveDrawable(table, configuration);
  }
  virtual ~ContainerCurveEditorContentComponent()
    {delete drawable;}

  virtual void paint(Graphics& g)
    {drawable->drawWithin(g, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);}

protected:
  ContainerCurveDrawable* drawable;
};

/*
** ContainerCurveEditorConfiguration
*/
ContainerCurveEditorConfiguration::ContainerCurveEditorConfiguration(ClassPtr type)
  : xAxis(new CurveAxisConfiguration(0.0, 1000.0, T("X Axis"))),
    yAxis(new CurveAxisConfiguration(0.0, 1.0, T("Y Axis"))),
    variables(type->getObjectNumVariables())
{
  for (size_t i = 0; i < variables.size(); ++i)
  {
    TypePtr variableType = type->getObjectVariableType(i);
    if (variableType->inheritsFrom(integerType) || variableType->inheritsFrom(doubleType))
      variables[i] = new CurveVariableConfiguration(i == 1, Colours::red, type->getObjectVariableName(i));
  }
  keyVariableIndex = 0;
}

std::vector<size_t> ContainerCurveEditorConfiguration::getSelectedCurves() const
{
  std::vector<size_t> res;
  for (size_t i = 0; i < variables.size(); ++i)
    if (i != keyVariableIndex && variables[i] && variables[i]->isSelected())
      res.push_back(i);
  return res;
}

/*
** ContainerCurveEditor
*/
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
  return new ContainerCurveEditorContentComponent(object.staticCast<Container>(), configuration.staticCast<ContainerCurveEditorConfiguration>());
}
