/*-----------------------------------------.---------------------------------.
| Filename: PieChartComponent.h            | Components for Pie Chart        |
| Author  : Julien Becker                  |                                 |
| Started : 08/06/2011 13:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_PIE_CHART_H_
# define LBCPP_USER_INTERFACE_COMPONENT_PIE_CHART_H_

# include <oil/UserInterface/ObjectComponent.h>
# include <oil/UserInterface/PieChart.h>

namespace lbcpp
{

class PieChartComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  PieChartComponent(PieChartPtr pieChart, const string& name)
    : pieChart(pieChart) {}

  virtual int getDefaultWidth() const
    {return 300;}

  virtual int getDefaultHeight() const
    {return 300;}

  virtual void paint(juce::Graphics& g)
  {
    const int width = getWidth();
    //const int height = getHeight();
    float currentY = 0.f;

    juce::Font f = g.getCurrentFont();
    g.setFont(24, juce::Font::bold);
    g.drawText(pieChart->getName(), 0, 0, width, 30, juce::Justification::centred, false);
    g.setFont(f);
    currentY += 40;
    
    const float radius = width / 3.f;
    drawPieChart(g, width / 2.f, currentY + radius, (int)radius);
    currentY += 2 * radius + 10.f;
    
    drawPieLegend(g, width / 2.f - 125.f, currentY);
  }

protected:
  PieChartPtr pieChart;
  std::vector<juce::Colour> colors;

  void drawPieChart(juce::Graphics& g, float x, float y, int radius)
  {
    const float diameter = radius * 2.f;
    const float innerCircle = 0.5f;
    colors.clear();

    std::vector<float> angles;
    angles.push_back(0.f);

    float currentAngle = 0.0;
    for (size_t i = 0; i < pieChart->getNumElements(); ++i)
    {
      float nextAngle = currentAngle + (float)(M_2_TIMES_PI * pieChart->getElementValue(i));
      angles.push_back(nextAngle);

      juce::Path path;
      path.addPieSegment(x - radius, y - radius, diameter, diameter, currentAngle + (float)(M_PI / 2), nextAngle + (float)(M_PI / 2), 0.5f);
      juce::Colour c = juce::Colour(i / (float)pieChart->getNumElements(), 1.f, 0.75f, (juce::uint8)255);
      colors.push_back(c);
      g.setColour(c);
      g.fillPath(path);
      g.setColour(juce::Colours::black);
      
      currentAngle = nextAngle;
    }
    
    const float innerRadius = radius * innerCircle;
    g.drawEllipse(x - radius, y - radius, diameter, diameter, 1);
    g.drawEllipse(x - innerRadius, y - innerRadius, diameter * innerCircle, diameter * innerCircle, 1);
    
    for (size_t i = 0; i < angles.size(); ++i)
    {
      float cosAngle = cos(angles[i]);
      float sinAngle = sin(angles[i]);
      g.drawLine(x + cosAngle * innerRadius, y + sinAngle * innerRadius, x + cosAngle * radius, y + sinAngle * radius);
    }    
  }

  void drawPieLegend(juce::Graphics& g, float x, float y)
  {
    enum {lineHeight = 20};
    enum {squareSize = 10};
    enum {percentValueWidth = 50};
    enum {textWidth = 170};

    const size_t n = pieChart->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const float currentY = y + i * lineHeight;
      g.setColour(colors[i]);
      g.fillRect(x, currentY + (lineHeight - squareSize) / 2.f, (float)squareSize, (float)squareSize);
      g.setColour(juce::Colours::black);
      g.drawRect(x, currentY + (lineHeight - squareSize) / 2.f, (float)squareSize, (float)squareSize);

      g.drawText(string(pieChart->getElementValue(i) * 100, 2) + JUCE_T(" %"), (int)x + squareSize + 10, (int)currentY, percentValueWidth, lineHeight, juce::Justification::centredRight, false);
      g.drawText(pieChart->getElementName(i), (int)x + squareSize + percentValueWidth + 20, (int)currentY, textWidth, lineHeight, juce::Justification::centredLeft, true);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_PIE_CHART_H_
