/*-----------------------------------------.---------------------------------.
| Filename: ROCScoreObjectComponent.h      | Components for ROC Score        |
| Author  : Julien Becker                  |                                 |
| Started : 12/05/2011 13:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_ROC_SCORE_OBJECT_H_
# define LBCPP_USER_INTERFACE_COMPONENT_ROC_SCORE_OBJECT_H_

# include "ContainerCurveEditor.h"
# include "../../Function/Evaluator/Utilities.h"

namespace lbcpp
{

class ROCScoreObjectComponent : public Component, public ComponentWithPreferedSize
{
public:
  ROCScoreObjectComponent(ROCScoreObjectPtr rocScore, const String& name)
  {
    addAndMakeVisible(curveEditorComponent = new ContainerCurveEditor(defaultExecutionContext(), rocScore->createROCCurveElements(), createCurveEditorConfiguration()));
    addAndMakeVisible(confusionMatrixLabel = new juce::Label(T("bestButtonLabel"), T("Best Confusion Matrix: ")));
    addAndMakeVisible(rocTextButton = new juce::TextButton(T("ROC Curve")));
    BinaryClassificationConfusionMatrixPtr confusionMatrix = rocScore->findBestSensitivitySpecificityTradeOff();
    String txt;
    if (confusionMatrix)
      txt = String(confusionMatrix->computeRecall(), 4) + T(" ") + String(confusionMatrix->computeSpecificity(), 4);
    addAndMakeVisible(ssTextButton = new juce::TextButton(T("Sensitivity/Specificity ") + txt));
  }
  
  virtual int getDefaultWidth() const
    {return ((ContainerCurveEditor*)curveEditorComponent)->getDefaultWidth();}

  virtual void resized()
  {
    const int width = getWidth();
    const int height = getHeight();

    const int buttonWidth = width / 3;
    const int buttonHeight = 20;

    curveEditorComponent->setBounds(0, 0, width, height - buttonHeight - 10);
    confusionMatrixLabel->setBounds(0, height - buttonHeight, buttonWidth, buttonHeight);
    rocTextButton->setBounds(buttonWidth, height - buttonHeight, buttonWidth, buttonHeight);
    ssTextButton->setBounds(buttonWidth * 2, height - buttonHeight, buttonWidth, buttonHeight);
  }

protected:
  Component* curveEditorComponent;
  juce::Label* confusionMatrixLabel;
  juce::TextButton* rocTextButton;
  juce::TextButton* ssTextButton;

  ContainerCurveEditorConfigurationPtr createCurveEditorConfiguration() const
  {
    ContainerCurveEditorConfigurationPtr res = new ContainerCurveEditorConfiguration(rocScoreObjectElementClass);
    res->setXAxis(new CurveAxisConfiguration(0.0, 1.0, T(""), false));
    res->setYAxis(new CurveAxisConfiguration(0.0, 1.0, T(""), false));
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_ROC_SCORE_OBJECT_H_
