/*-----------------------------------------.---------------------------------.
| Filename: PieChartComponent.h            | Components for Pie Chart        |
| Author  : Julien Becker                  |                                 |
| Started : 08/06/2011 13:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_PIE_CHART_H_
# define LBCPP_USER_INTERFACE_COMPONENT_PIE_CHART_H_

# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>

namespace lbcpp
{

class PieChartConfiguration : public NameableObject
{
public:
  PieChartConfiguration(const String& name = String::empty, const String& description = String::empty)
    : NameableObject(name), description(description) {}

  size_t getNumElements() const
    {return values.size();}

  void appendElement(const String& name, double value)
    {values.push_back(std::make_pair(name, value));}

  String getElementName(size_t index) const
    {jassert(index < values.size()); return values[index].first;}

  double getElementValue(size_t index) const
    {jassert(index < values.size()); return values[index].second;}

  String getDescription() const
    {return description;}

protected:
  friend class PieChartConfigurationClass;

  String description;
  std::vector<std::pair<String, double> > values;
};

typedef ReferenceCountedObjectPtr<PieChartConfiguration> PieChartConfigurationPtr;

class PieChartComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  PieChartComponent(PieChartConfigurationPtr configuration, const String& name)
    : configuration(configuration) {}

  virtual int getDefaultWidth() const
    {return 300;}

  virtual int getDefaultHeight() const
    {return 300;}

  virtual void paint(juce::Graphics& g)
  {
    const int width = getWidth();
    //const int height = getHeight();
    double currentY = 0.;

    juce::Font f = g.getCurrentFont();
    g.setFont(24, juce::Font::bold);
    g.drawText(configuration->getName(), 0, 0, width, 30, juce::Justification::centred, false);
    g.setFont(f);
    currentY += 40;
    
    const int radius = width / 3.;
    drawPieChart(g, width / 2., currentY + radius, radius);
    currentY += 2 * radius + 10;
    
    if (configuration->getDescription() != String::empty)
    {
      g.drawText(configuration->getDescription(), 20, currentY, width - 20, 50, juce::Justification::centred, true);
      currentY += 60;
    }
    
    drawPieLegend(g, width / 2. - 125, currentY);
  }

protected:
  PieChartConfigurationPtr configuration;
  std::vector<juce::Colour> colors;

  void drawPieChart(juce::Graphics& g, float x, float y, int radius)
  {
    const int diameter = radius * 2;
    const double innerCircle = 1/2.;
    colors.clear();

    std::vector<double> angles;
    angles.push_back(0.0);

    double currentAngle = 0.0;
    for (size_t i = 0; i < configuration->getNumElements(); ++i)
    {
      double nextAngle = currentAngle + M_2_TIMES_PI * configuration->getElementValue(i);
      angles.push_back(nextAngle);

      juce::Path path;
      path.addPieSegment(x - radius, y - radius, diameter, diameter, currentAngle + M_PI / 2, nextAngle + M_PI / 2, 1/2.);
      juce::Colour c = juce::Colour(i / (double)configuration->getNumElements(), 1.f, 0.75f, (juce::uint8)255);
      colors.push_back(c);
      g.setColour(c);
      g.fillPath(path);
      g.setColour(juce::Colours::black);
      
      currentAngle = nextAngle;
    }
    
    const double innerRadius = radius * innerCircle;
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

    const size_t n = configuration->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const double currentY = y + i * lineHeight;
      g.setColour(colors[i]);
      g.fillRect((float)x, (float)currentY + (lineHeight - squareSize) / 2., (float)squareSize, (float)squareSize);
      g.setColour(juce::Colours::black);
      g.drawRect((float)x, (float)currentY + (lineHeight - squareSize) / 2., (float)squareSize, (float)squareSize);

      g.drawText(String(configuration->getElementValue(i) * 100, 2) + T(" %"), x + squareSize + 10, currentY, percentValueWidth, lineHeight, juce::Justification::centredRight, false);
      g.drawText(configuration->getElementName(i), x + squareSize + percentValueWidth + 20, currentY, textWidth, lineHeight, juce::Justification::centredLeft, true);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_PIE_CHART_H_