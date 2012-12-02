/*-----------------------------------------.---------------------------------.
| Filename: PlotConfigurationComponent.h   | Plot Configuration Component    |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 12:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_PLOT_CONFIGURATION_COMPONENT_H_
# define LBCPP_USER_INTERFACE_PLOT_CONFIGURATION_COMPONENT_H_

# include <lbcpp/UserInterface/ObjectComponent.h>
# include <lbcpp/UserInterface/Plot.h>

namespace lbcpp
{

class PlotVariableListBoxModel : public juce::ListBoxModel
{
public:
  PlotVariableListBoxModel(const PlotPtr& plot, juce::ChangeBroadcaster* callback)
    : plot(plot), callback(callback) {}

  virtual int getNumRows()
    {return plot->getNumPlotVariables();}

  virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
  {
    PlotVariablePtr plotVariable = plot->getPlotVariable(rowNumber);
    if (plotVariable->isSelected())
    {
      g.setColour(juce::Colours::lightblue);
      g.fillRect(0, 0, width, height);
    }

    g.setColour(plotVariable->getColour());
    g.drawText(plotVariable->getName(), 0, 0, width, height, juce::Justification(juce::Justification::left), true);
  }

  virtual void listBoxItemClicked(int row, const juce::MouseEvent& e)
  {
    PlotVariablePtr plotVariable = plot->getPlotVariable(row);
    plotVariable->setSelected(!plotVariable->isSelected());
    listBox->deselectRow(row);
    callback->sendSynchronousChangeMessage(this);
  }

  void setListBox(juce::ListBox* listBox)
    {this->listBox = listBox;}

protected:
  PlotPtr plot;
  juce::ChangeBroadcaster* callback;
  juce::ListBox* listBox;
};

class PlotConfigurationComponent : public Component, public juce::ChangeBroadcaster,
                                    public juce::ComboBoxListener, public juce::ChangeListener,
                                    public juce::ButtonListener, public juce::TextEditorListener
{
public:
  PlotConfigurationComponent(PlotPtr plot)
    : plot(plot)
  {
    addAndMakeVisible(xAxisLabel = new juce::Label(T("xaxis"), T("X-Axis")));
    addAndMakeVisible(keyComboBox = createVariableIndexComboBox(plot->getKeyVariableIndex()));
    addAndMakeVisible(yAxisLabel = new juce::Label(T("yaxis"), T("Y-Axis")));
    // FIXME: Who will destroy curveListBox ?
    PlotVariableListBoxModel* curveListBox = new PlotVariableListBoxModel(plot, this);
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

  virtual ~PlotConfigurationComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
  {
    if (button == selectAllButton || button == noneButton)
    {
      const size_t n = plot->getNumPlotVariables();
      for (size_t i = 0; i < n; ++i)
      {
        plot->getPlotVariable(i)->setSelected(button == selectAllButton);
        yListBox->selectRow(i, true);
      }
      plot->getPlotVariable(plot->getKeyVariableIndex())->setSelected(false);
      sendSynchronousChangeMessage(this);
    }
  }

  virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == keyComboBox)
      plot->setKeyVariableIndex(keyComboBox->getSelectedId() - 1);
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
      plot->getXAxis()->setAutoRange(false);
      plot->getXAxis()->setRangeMin(value);
    }
    else if (&editor == xMaxScaleTextEditor)
    {
      plot->getXAxis()->setAutoRange(false);
      plot->getXAxis()->setRangeMax(value);
    }
    else if (&editor == yMinScaleTextEditor)
    {
      plot->getYAxis()->setAutoRange(false);
      plot->getYAxis()->setRangeMin(value);
    }
    else if (&editor == yMaxScaleTextEditor)
    {
      plot->getYAxis()->setAutoRange(false);
      plot->getYAxis()->setRangeMax(value);
    }
    sendSynchronousChangeMessage(this);
  }

  virtual void changeListenerCallback(void* objectThatHasChanged)
    {updateAxisRanges();}

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
    xMinScaleTextEditor->setText(string(plot->getXAxis()->getRangeMin()));
    xMaxScaleTextEditor->setText(string(plot->getXAxis()->getRangeMax()));
    yMinScaleTextEditor->setText(string(plot->getYAxis()->getRangeMin()));
    yMaxScaleTextEditor->setText(string(plot->getYAxis()->getRangeMax()));
  }

protected:
  PlotPtr plot;

  juce::Label* xAxisLabel;
  juce::ComboBox* keyComboBox;
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
    juce::ComboBox* res = new juce::ComboBox(T("combo"));
    for (size_t i = 0; i < plot->getNumPlotVariables(); ++i)
    {
      res->addItem(plot->getPlotVariable(i)->getName(), i + 1);
      if (selectedIndex == i)
        res->setSelectedId(i + 1);
    }
    res->addListener(this);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_CONFIGURATION_COMPONENT_H_
