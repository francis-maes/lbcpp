/*-----------------------------------------.---------------------------------.
| Filename: ConfusionMatrixComponent.h     | Components for Confusion Matrix |
| Author  : Julien Becker                  |                                 |
| Started : 12/05/2011 13:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_CONFUSION_MATRIX_H_
# define LBCPP_USER_INTERFACE_COMPONENT_CONFUSION_MATRIX_H_

# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include "../../Function/Evaluator/Utilities.h"

namespace lbcpp
{

class ConfusionMatrixComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  ConfusionMatrixComponent(BinaryClassificationConfusionMatrixPtr confusionMatrix, const String& name)
    : confusionMatrix(confusionMatrix)
  {}

  virtual int getDefaultWidth() const
    {return 300;}

  virtual int getDefaultHeight() const
    {return 300;}

  virtual void paint(juce::Graphics& g)
  {
    const int width = getWidth();
    const int center = width / 2;
    const int cellWidth = 100;
    const int cellHeight = 50;
    
    const juce::Font defaultFont = g.getCurrentFont();
    
    size_t yCursor = 20;
    g.setFont(defaultFont.getHeight() + 4, juce::Font::bold);
    g.drawText(T("Confusion Matrix"), 0, yCursor, width, 20, juce::Justification::horizontallyCentred, true);
    g.setFont(defaultFont);
    yCursor += 30;

    g.drawText(T("Actual value"), center - cellWidth, yCursor, cellWidth * 2, 20, juce::Justification::horizontallyCentred, true);
    yCursor += 20;
    g.drawText(T("true"), center - cellWidth, yCursor, cellWidth, 20, juce::Justification::horizontallyCentred, true);
    g.drawText(T("false"), center, yCursor, cellWidth, 20, juce::Justification::horizontallyCentred, true);
    yCursor += 20;

    g.drawTextAsPath(T("Outcome"), juce::AffineTransform::rotation((float)-M_PI / 2.).translated(center - cellWidth - 10, yCursor + cellHeight * 1.5));
    g.drawTextAsPath(T("   true"), juce::AffineTransform::rotation((float)M_PI / 2.).translated(center + cellWidth + 10, yCursor));
    g.drawTextAsPath(T("   false"), juce::AffineTransform::rotation((float)M_PI / 2.).translated(center + cellWidth + 10, yCursor + cellHeight));

    g.drawRect(center - cellWidth, yCursor, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getTruePositives()), center - cellWidth, yCursor, cellWidth, cellHeight, juce::Justification::centred, true);
    g.drawRect(center - 1, yCursor, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getFalsePositives()), center, yCursor, cellWidth, cellHeight, juce::Justification::centred, true);
    yCursor += cellHeight;
    g.drawRect(center - cellWidth, yCursor - 1, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getFalseNegatives()), center - cellWidth, yCursor, cellWidth, cellHeight, juce::Justification::centred, true);
    g.drawRect(center - 1, yCursor - 1, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getTrueNegatives()), center, yCursor, cellWidth, cellHeight, juce::Justification::centred, true);
    yCursor += cellHeight;

    const int labelWidth = 150;
    const int valueWidth = 50;
    const int rowCenter = center + valueWidth;

    yCursor += 10;
    g.setFont(defaultFont.getHeight() + 4, juce::Font::bold);
    g.drawText(T("Measures"), 0, yCursor, width, 20, juce::Justification::horizontallyCentred, true);
    g.setFont(defaultFont);
    yCursor += 30;
    g.drawText(T("Accuracy "), rowCenter - labelWidth, yCursor, labelWidth, 20, juce::Justification::left, true);
    g.drawText(String(confusionMatrix->computeAccuracy(), 4), rowCenter, yCursor, valueWidth, 20, juce::Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("F1 Score"), rowCenter - labelWidth, yCursor, labelWidth, 20, juce::Justification::left, true);
    g.drawText(String(confusionMatrix->computeF1Score(), 4), rowCenter, yCursor, valueWidth, 20, juce::Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Precision"), rowCenter - labelWidth, yCursor, labelWidth, 20, juce::Justification::left, true);
    g.drawText(String(confusionMatrix->computePrecision(), 4), rowCenter, yCursor, valueWidth, 20, juce::Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Recall (Sensitivity)"), rowCenter - labelWidth, yCursor, labelWidth, 20, juce::Justification::left, true);
    g.drawText(String(confusionMatrix->computeRecall(), 4), rowCenter, yCursor, valueWidth, 20, juce::Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Specificity"), rowCenter - labelWidth, yCursor, labelWidth, 20, juce::Justification::left, true);
    g.drawText(String(confusionMatrix->computeSpecificity(), 4), rowCenter, yCursor, valueWidth, 20, juce::Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Matthews Correlation"), rowCenter - labelWidth, yCursor, labelWidth, 20, juce::Justification::left, true);
    g.drawText(String(confusionMatrix->computeMatthewsCorrelation(), 4), rowCenter, yCursor, valueWidth, 20, juce::Justification::horizontallyCentred, true);    
  }

protected:
  BinaryClassificationConfusionMatrixPtr confusionMatrix;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_CONFUSION_MATRIX_H_
