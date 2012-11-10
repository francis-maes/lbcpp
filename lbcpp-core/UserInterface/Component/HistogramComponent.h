/*-----------------------------------------.---------------------------------.
| Filename: HistogramComponent.h           | Component for Histograms        |
| Author  : Julien Becker                  |                                 |
| Started : 09/06/2011 14:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_HISTOGRAM_H_
# define LBCPP_USER_INTERFACE_COMPONENT_HISTOGRAM_H_

# include <lbcpp/UserInterface/Histogram.h>
# include <lbcpp/UserInterface/ObjectComponent.h>

namespace lbcpp
{

class HistogramComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  enum {minimumBinWidth = 15};

  HistogramComponent(HistogramPtr histogram, const String& name)
    : histogram(histogram) {}

  virtual int getDefaultWidth() const
    {return (histogram->getNumBins() + (histogram->drawOutOfBound() ? 2 : 0)) * minimumBinWidth;}
  
  virtual int getDefaultHeight() const
    {return 300;}
  
  virtual void paint(juce::Graphics& g)
  {
    const int width = getWidth();
    const int height = getHeight();
    float currentY = 0.f;

    juce::Font f = g.getCurrentFont();
    g.setFont(24, juce::Font::bold);
    g.drawText(histogram->getName(), 0, 0, width, 30, juce::Justification::centred, false);
    g.setFont(f);
    currentY += 40.f;

    drawHistogram(g, 20.f, currentY, width - 40.f, juce::jmin(300.f, height - currentY - 10.f));
  }

protected:
  HistogramPtr histogram;

  void drawHistogram(juce::Graphics& g, float x, float y, float width, float height)
  {
    const float binWidth = width / ((float)histogram->getNumBins() + (histogram->drawOutOfBound() ? 2 : 0));
    const float binUnitHeight = height / (float)histogram->getMaximumBinValue();
    const size_t n = histogram->getNumBins();
    // draw bar
    for (size_t i = 0; i < n; ++i)
    {
      juce::Colour c = juce::Colour(i / (float)histogram->getNumBins(), 1.f, 0.75f, (juce::uint8)255);
      g.setColour(c);
      g.fillRect(x + (i + (histogram->drawOutOfBound() ? 1 : 0)) * binWidth, y + height - binUnitHeight * histogram->getBin(i), binWidth, binUnitHeight * histogram->getBin(i));
    }
    // draw border
    g.setColour(juce::Colours::black);
    for (size_t i = 0; i < n; ++i)
      g.drawRect(x + (i + (histogram->drawOutOfBound() ? 1 : 0)) * binWidth, y + height - binUnitHeight * histogram->getBin(i), binWidth, binUnitHeight * histogram->getBin(i), 1.);
    // draw out-of-bound
    if (histogram->drawOutOfBound())
    {
      g.fillRect(x, y + height - binUnitHeight * histogram->getLeftOutOfBound(), binWidth, binUnitHeight * histogram->getLeftOutOfBound());
      g.fillRect(x + n * binWidth, y + height - binUnitHeight * histogram->getRightOutOfBound(), binWidth, binUnitHeight * histogram->getRightOutOfBound());
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_HISTOGRAM_H_
