/*-----------------------------------------.---------------------------------.
| Filename: ContainerCurveEditor.cpp       | Container Curve Editor          |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ContainerCurveEditor.h"
#include <lbcpp/Data/RandomGenerator.h>
#include "../Plot/TwoDimensionalPlotDrawable.h"
using namespace lbcpp;

using juce::ComboBox;
using juce::RectanglePlacement;
using juce::Justification;
using juce::AffineTransform;

class ContainerCurveDrawable : public TwoDimensionalPlotDrawable
{
public:
  ContainerCurveDrawable(const ContainerPtr& table, const ContainerCurveEditorConfigurationPtr& configuration)
    : table(table), configuration(configuration)
  {
    selectedCurves = configuration->getSelectedCurves();
    size_t keyVariableIndex = configuration->getKeyVariableIndex();
    jassertfalse; // broken
    //table->makeOrder(keyVariableIndex, true, order);
    computeBounds();
    if (!areBoundsValid())
    {
      boundsX = -1000.0;
      boundsWidth = 2000.0;
      boundsY = -1.0;
      boundsHeight = 2.0;
    }
  }

  enum {pointCrossSize = 6};

  virtual Drawable* createCopy() const
    {return new ContainerCurveDrawable(table, configuration);}

  virtual void draw(Graphics& g, const AffineTransform& transform = AffineTransform::identity) const
  {
    TwoDimensionalPlotDrawable::draw(g, transform);
    if (areBoundsValid() && table->getNumElements() > 0)
      for (size_t i = 0; i < selectedCurves.size(); ++i)
      {
        drawCurveLine(g, transform, selectedCurves[i], configuration->getCurve(selectedCurves[i]));
        drawCurvePoint(g, transform, selectedCurves[i], configuration->getCurve(selectedCurves[i])); 
      }
  }

  virtual PlotAxisPtr getXAxis() const
    {return configuration->getXAxis();}

  virtual PlotAxisPtr getYAxis() const
    {return configuration->getYAxis();}

  virtual String getXAxisLabel() const
  {
    String res = configuration->getXAxis()->getLabel();
    if (res.isEmpty())
      res = table->getElementsType()->getMemberVariableName(configuration->getKeyVariableIndex());
    return res;
  }

  virtual String getYAxisLabel() const
  {
    String res = configuration->getYAxis()->getLabel();
    if (res.isEmpty())
    {
      for (size_t i = 0; i < selectedCurves.size(); ++i)
      {
        if (i > 0)
          res += T(", ");
        res += table->getElementsType()->getMemberVariableName(selectedCurves[i]);
      }
    }
    return res;
  }

  virtual void computeXAxisAutoRange(PlotAxisPtr axis) const
  {
    double minValue = DBL_MAX, maxValue = -DBL_MAX;
    getTableValueRange(configuration->getKeyVariableIndex(), minValue, maxValue);
    if (maxValue > minValue)
    {
      axis->setRange(minValue, maxValue);
      axis->increaseRange(0.2);
    }
    else if (maxValue == minValue)
      axis->setRange(minValue - 1.1, maxValue + 1.1);
  }

  virtual void computeYAxisAutoRange(PlotAxisPtr axis) const
  {
    double minValue = DBL_MAX, maxValue = -DBL_MAX;
    for (size_t i = 0; i < selectedCurves.size(); ++i)
      getTableValueRange(selectedCurves[i], minValue, maxValue);
    if (maxValue > minValue)
    {
      axis->setRange(minValue, maxValue);
      axis->increaseRange(0.2);
    }
    else if (maxValue == minValue)
      axis->setRange(minValue - 1.1, maxValue + 1.1);
  }

protected:
  ContainerPtr table;
  ContainerCurveEditorConfigurationPtr configuration;

  std::vector<size_t> selectedCurves;
  std::vector<size_t> order;

  bool getPointPosition(size_t row, size_t columnX, size_t columnY, const AffineTransform& transform, float& x, float& y) const
  {
    ObjectPtr rowObject = table->getElement(order[row]).getObject();
    double dx = 0.0, dy = 0.0;
    if (!getTableScalarValue(rowObject, columnX, dx) || !getTableScalarValue(rowObject, columnY, dy))
      return false;

    x = (float)dx;
    y = (float)dy;
    transform.transformPoint(x, y);
    return true;
  }

  bool getTableScalarValue(const ObjectPtr& row, size_t column, double& scalarValue) const
  {
    Variable v = row->getVariable(column);
    if (!v.exists() || !v.isConvertibleToDouble())
      return false;
    scalarValue = v.toDouble();
    return true;
  }

  void getTableValueRange(size_t column, double& minValue, double& maxValue) const
  {
    double value = 0.0;
    for (size_t i = 0; i < table->getNumElements(); ++i)
      if (getTableScalarValue(table->getElement(i).getObject(), column, value))
      {
        if (value > maxValue)
          maxValue = value;
        if (value < minValue)
          minValue = value;
      }
  }

  void drawCurveLine(Graphics& g, const AffineTransform& transform, size_t index, CurveVariableConfigurationPtr config) const
  {
    size_t n = table->getNumElements();
    size_t keyVariableIndex = configuration->getKeyVariableIndex();
    float x0, y0;
    bool isP0Valid = getPointPosition(0, keyVariableIndex, index, transform, x0, y0) && isNumberValid(x0) && isNumberValid(y0);

    g.setColour(config->getColour());
    for (size_t i = 1; i < n; ++i)
    {
      float x1, y1;
      bool isP1Valid = getPointPosition(i, keyVariableIndex, index, transform, x1, y1) && isNumberValid(x1) && isNumberValid(y1);
      if (isP0Valid && isP1Valid && fabs(y0) < 1e16 && fabs(y1) < 1e16)
        g.drawLine(x0, y0, x1, y1); // juce has a problem with very very large values, so we restrict y-values to 1e16
      x0 = x1;
      y0 = y1;
      isP0Valid = isP1Valid;
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
      float crossHalfSize = pointCrossSize / 2.f;
      if (getPointPosition(i, keyVariableIndex, index, transform, x, y) && isNumberValid(x) && isNumberValid(y))
      {
        g.drawLine(x - crossHalfSize, y, x + crossHalfSize, y);
        g.drawLine(x, y - crossHalfSize, x, y + crossHalfSize);
      }
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

  enum
  {
    leftMargin = 60,
    rightMargin = 20,
    topMargin = 20,
    bottomMargin = 40
  };

  virtual void paint(Graphics& g)
    {drawable->drawWithin(g, leftMargin, topMargin, getWidth() - leftMargin - rightMargin, getHeight() - topMargin - bottomMargin, juce::RectanglePlacement::stretchToFit);}

protected:
  ContainerCurveDrawable* drawable;
};
//////////////////////////////////////////

class ContainerCurveSelectorConfigurationComponent : public BooleanButtonsComponent
{
public:
  ContainerCurveSelectorConfigurationComponent(ContainerCurveEditorConfigurationPtr configuration)
  {
    std::vector<ConfigurationButton* > buttonsColumn;

    TypePtr rowType = configuration->getRowType();
    for (size_t i = 0; i < rowType->getNumMemberVariables(); ++i)
    {
      CurveVariableConfigurationPtr curve = configuration->getCurve(i);
      if (curve)
        addToggleButton(buttonsColumn, rowType->getMemberVariableName(i), curve->isSelected(), 4, curve->getColour());
    }
    flushButtons(buttonsColumn);

    initialize();
  }
};

class CurveListBoxModel : public juce::ListBoxModel
{
public:
  CurveListBoxModel(const ContainerCurveEditorConfigurationPtr& configuration, juce::ChangeBroadcaster* callback)
    : configuration(configuration), callback(callback)
  {
    const size_t n = configuration->getNumCurves();
    for (size_t i = 0; i < n; ++i)
    {
      CurveVariableConfigurationPtr curve = configuration->getCurve(i);
      if (curve)
        curves.push_back(curve);
    }
  }

  virtual int getNumRows()
    {return curves.size();}

  virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
  {
    if (curves[rowNumber]->isSelected())
    {
      g.setColour(juce::Colours::lightblue);
      g.fillRect(0, 0, width, height);
    }

    g.setColour(curves[rowNumber]->getColour());
    g.drawText(curves[rowNumber]->getLabel(), 0, 0, width, height, juce::Justification(juce::Justification::left), true);
  }

  virtual void listBoxItemClicked(int row, const juce::MouseEvent& e)
  {
    curves[row]->setSelected(!curves[row]->isSelected());
    listBox->deselectRow(row);
    callback->sendSynchronousChangeMessage(this);
  }

  void setListBox(juce::ListBox* listBox)
    {this->listBox = listBox;}

protected:
  ContainerCurveEditorConfigurationPtr configuration;
  juce::ChangeBroadcaster* callback;
  juce::ListBox* listBox;
  std::vector<CurveVariableConfigurationPtr> curves;
};

class ContainerCurveEditorConfigurationComponent : public Component, public juce::ChangeBroadcaster,
                                                   public juce::ComboBoxListener, public juce::ChangeListener,
                                                   public juce::ButtonListener, public juce::TextEditorListener
{
public:
  ContainerCurveEditorConfigurationComponent(ContainerCurveEditorConfigurationPtr configuration)
    : configuration(configuration)
  {
    addAndMakeVisible(xAxisLabel = new juce::Label(T("xaxis"), T("X-Axis")));
    addAndMakeVisible(keyComboBox = createVariableIndexComboBox(configuration->getKeyVariableIndex()));
    addAndMakeVisible(yAxisLabel = new juce::Label(T("yaxis"), T("Y-Axis")));
    // FIXME: Who will destroy curveListBox ?
    CurveListBoxModel* curveListBox = new CurveListBoxModel(configuration, this);
    addAndMakeVisible(yListBox = new juce::ListBox(T("yListBox"), curveListBox));
    curveListBox->setListBox(yListBox);
    yListBox->setOutlineThickness(1);
    yListBox->setRowHeight(15);
    yListBox->setMultipleSelectionEnabled(false);

    addAndMakeVisible(selectAllButton = new juce::TextButton(T("Select All")));
    selectAllButton->addButtonListener(this);
    addAndMakeVisible(noneButton = new juce::TextButton(T("None")));
    noneButton->addButtonListener(this);

    addAndMakeVisible(scalesLabel = new juce::Label(T("scales"), T("Scales")));
    addAndMakeVisible(xMinScaleLabel = new juce::Label(T("xMinScaleLabel"), T("x-Min")));
    addAndMakeVisible(xMaxScaleLabel = new juce::Label(T("xMaxScaleLabel"), T("x-Max")));
    addAndMakeVisible(yMinScaleLabel = new juce::Label(T("yMinScaleLabel"), T("y-Min")));
    addAndMakeVisible(yMaxScaleLabel = new juce::Label(T("yMaxScaleLabel"), T("y-Max")));

    addAndMakeVisible(xMinScaleTextEditor = new juce::TextEditor(T("xMinScaleTextEditor")));
    addAndMakeVisible(xMaxScaleTextEditor = new juce::TextEditor(T("xMaxScaleTextEditor")));
    addAndMakeVisible(yMinScaleTextEditor = new juce::TextEditor(T("yMinScaleTextEditor")));
    addAndMakeVisible(yMaxScaleTextEditor = new juce::TextEditor(T("yMaxScaleTextEditor")));

    xMinScaleTextEditor->addListener(this);
    xMaxScaleTextEditor->addListener(this);
    yMinScaleTextEditor->addListener(this);
    yMaxScaleTextEditor->addListener(this);

    addChangeListener(this);
  }

  virtual ~ContainerCurveEditorConfigurationComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
  {
    if (button == selectAllButton || button == noneButton)
    {
      const size_t n = configuration->getNumCurves();
      size_t index = 0;
      for (size_t i = 0; i < n; ++i)
      {
        if (configuration->getCurve(i))
        {
          configuration->getCurve(i)->setSelected(button == selectAllButton);
          yListBox->selectRow(index, true);
          ++index;
        }
      }
      configuration->getCurve(configuration->getKeyVariableIndex())->setSelected(false);
      sendSynchronousChangeMessage(this);
    }
  }

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == keyComboBox)
      configuration->setKeyVariableIndex(keyComboBox->getSelectedId() - 1);
    sendSynchronousChangeMessage(this);
  }
  
  virtual void textEditorEscapeKeyPressed(juce::TextEditor& editor) {}
  virtual void textEditorTextChanged(juce::TextEditor &editor) {}
  virtual void textEditorFocusLost(juce::TextEditor& editor) {}

  virtual void textEditorReturnKeyPressed(juce::TextEditor& editor)
  {
    double value = editor.getText().getDoubleValue();
    if (&editor == xMinScaleTextEditor)
    {
        configuration->getXAxis()->setAutoRange(false);
        configuration->getXAxis()->setRangeMin(value);
    }
    else if (&editor == xMaxScaleTextEditor)
    {
        configuration->getXAxis()->setAutoRange(false);
        configuration->getXAxis()->setRangeMax(value);
    }
    else if (&editor == yMinScaleTextEditor)
    {
        configuration->getYAxis()->setAutoRange(false);
        configuration->getYAxis()->setRangeMin(value);
    }
    else if (&editor == yMaxScaleTextEditor)
    {
        configuration->getYAxis()->setAutoRange(false);
        configuration->getYAxis()->setRangeMax(value);
    }
    sendSynchronousChangeMessage(this);
  }

  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    updateAxisRanges();
  }

  virtual void resized()
  {
    xAxisLabel->setBoundsRelative(0, 0, 0.3f, 0.5f);
    xAxisLabel->setSize(xAxisLabel->getWidth(), 20);
    keyComboBox->setBoundsRelative(0, 0.22f, 0.3f, 0.5f);
    keyComboBox->setSize(keyComboBox->getWidth(), 20);

    yAxisLabel->setBoundsRelative(0.35f, 0, 0.3f, 0.5f);
    yAxisLabel->setSize(yAxisLabel->getWidth(), 20);
    yListBox->setBoundsRelative(0.35f, 0.22f, 0.3f, 1.f);
    yListBox->setSize(yListBox->getWidth(), (int)(yListBox->getHeight() - 0.22f * getHeight()));

    //selectedCurves->setBoundsRelative(0.4f, 0, 0.6f, 1.f);
    selectAllButton->setBoundsRelative(0.7f, 0.22f, 0.3f, 0.5f);
    selectAllButton->setSize(selectAllButton->getWidth(), 20);
    noneButton->setBoundsRelative(0.7f, 0.44f, 0.3f, 0.5f);
    noneButton->setSize(noneButton->getWidth(), 20);

    scalesLabel->setBoundsRelative(0, 0.44f, 0.3f, 0.5f);
    scalesLabel->setSize(scalesLabel->getWidth(), 20);
    xMinScaleLabel->setBoundsRelative(0.01f, 0.60f, 0.07f, 0.5f);
    xMinScaleLabel->setSize(xMinScaleLabel->getWidth(), 20);
    xMaxScaleLabel->setBoundsRelative(0.16f, 0.60f, 0.07f, 0.5f);
    xMaxScaleLabel->setSize(xMaxScaleLabel->getWidth(), 20);
    yMinScaleLabel->setBoundsRelative(0.01f, 0.80f, 0.07f, 0.5f);
    yMinScaleLabel->setSize(yMinScaleLabel->getWidth(), 20);
    yMaxScaleLabel->setBoundsRelative(0.16f, 0.80f, 0.07f, 0.5f);
    yMaxScaleLabel->setSize(yMaxScaleLabel->getWidth(), 20);

    xMinScaleTextEditor->setBoundsRelative(0.07f, 0.60f, 0.08f, 0.5f);
    xMinScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
    xMaxScaleTextEditor->setBoundsRelative(0.22f, 0.60f, 0.08f, 0.5f);
    xMaxScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
    yMinScaleTextEditor->setBoundsRelative(0.07f, 0.80f, 0.08f, 0.5f);
    yMinScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
    yMaxScaleTextEditor->setBoundsRelative(0.22f, 0.80f, 0.08f, 0.5f);
    yMaxScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
  }

  void updateAxisRanges()
  {
    xMinScaleTextEditor->setText(String(configuration->getXAxis()->getRangeMin()));
    xMaxScaleTextEditor->setText(String(configuration->getXAxis()->getRangeMax()));
    yMinScaleTextEditor->setText(String(configuration->getYAxis()->getRangeMin()));
    yMaxScaleTextEditor->setText(String(configuration->getYAxis()->getRangeMax()));
  }

protected:
  ContainerCurveEditorConfigurationPtr configuration;

  juce::Label* xAxisLabel;
  ComboBox* keyComboBox;
  juce::Label* yAxisLabel;
  juce::ListBox* yListBox;
  juce::Button* selectAllButton;
  juce::Button* noneButton;

  juce::Label* scalesLabel;
  juce::Label* xMinScaleLabel;
  juce::Label* xMaxScaleLabel;
  juce::Label* yMinScaleLabel;
  juce::Label* yMaxScaleLabel;

  juce::TextEditor* xMinScaleTextEditor;
  juce::TextEditor* xMaxScaleTextEditor;
  juce::TextEditor* yMinScaleTextEditor;
  juce::TextEditor* yMaxScaleTextEditor;

  juce::ComboBox* createVariableIndexComboBox(size_t selectedIndex)
  {
    TypePtr type = configuration->getRowType();
    ComboBox* res = new ComboBox(T("combo"));
    for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
      if (configuration->getCurve(i))
      {
        res->addItem(type->getMemberVariableName(i), i + 1);
        if (selectedIndex == i)
          res->setSelectedId(i + 1);
      }
    res->addListener(this);
    return res;
  }
};

//////////////////////////////////////////
/*
** ContainerCurveEditorConfiguration
*/

using juce::Colour;
using juce::Colours;
inline Colour randomColour()
{
  RandomGeneratorPtr random = defaultExecutionContext().getRandomGenerator();
  return Colour(random->sampleByte(), random->sampleByte(), random->sampleByte(), (unsigned char)255);
}

ContainerCurveEditorConfiguration::ContainerCurveEditorConfiguration(ClassPtr rowType)
  : rowType(rowType), 
    xAxis(new PlotAxis(0.0, 1000.0)),
    yAxis(new PlotAxis(0.0, 1.0)),
    variables(rowType->getNumMemberVariables())
{
  const juce::Colour defaultColours[] = {
      Colours::red, Colours::green, Colours::blue, Colours::black,
      Colours::yellow.withBrightness(0.5f), Colours::cyan, Colours::violet, Colours::grey,
      Colours::darkred, Colours::darkgreen, Colours::darkblue,
  };
  const size_t numDefaultColours = sizeof (defaultColours) / sizeof (juce::Colour);

  keyVariableIndex = (size_t)-1;
  for (size_t i = 0; i < variables.size(); ++i)
  {
    TypePtr variableType = rowType->getMemberVariableType(i);
    if (variableType->isConvertibleToDouble())
    {
      if (keyVariableIndex == (size_t)-1)
        keyVariableIndex = i; // default x-variable
      variables[i] = new CurveVariableConfiguration(false, i < numDefaultColours ? defaultColours[i] : randomColour(), rowType->getMemberVariableName(i));
    }
  }
  if (keyVariableIndex == (size_t)-1)
    keyVariableIndex = 0;
  // select y-variable
  for (size_t i = keyVariableIndex + 1; i < variables.size(); ++i)
    if (variables[i])
    {
      variables[i]->setSelected(true);
      break;
    }
}

std::vector<size_t> ContainerCurveEditorConfiguration::getSelectedCurves() const
{
  std::vector<size_t> res;
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i] && variables[i]->isSelected())
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
  return new ContainerCurveEditorConfigurationComponent(configuration.staticCast<ContainerCurveEditorConfiguration>());
}

Component* ContainerCurveEditor::createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration)
{
  return new ContainerCurveEditorContentComponent(object.staticCast<Container>(), configuration.staticCast<ContainerCurveEditorConfiguration>());
}
