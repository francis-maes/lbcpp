/*-----------------------------------------.---------------------------------.
| Filename: HistogramComponent.h           | Components for Histogram        |
| Author  : Julien Becker                  |                                 |
| Started : 09/06/2011 14:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_HISTOGRAM_H_
# define LBCPP_USER_INTERFACE_COMPONENT_HISTOGRAM_H_

# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>

namespace lbcpp
{

class HistogramConfiguration : public NameableObject
{
public:
  HistogramConfiguration(double stepSize, double minValue, double maxValue, bool includeOutOfBound = true, const String& name = String::empty)
    : NameableObject(name), stepSize(stepSize), minValue(minValue), maxValue(maxValue), includeOutOfBound(includeOutOfBound)
    {jassert(minValue <= maxValue);}
  HistogramConfiguration() {}

  void addData(double value)
  {
    values.push_back(value);
    bins.clear();
  }

  size_t getNumBins() const
    {return ceil((maxValue - minValue) / stepSize);}
  
  size_t getBin(size_t index) const
  {
    jassert(index < getNumBins());
    const_cast<HistogramConfiguration* >(this)->ensureBinsAreComputed();
    return bins[index];
  }

  bool drawOutOfBound() const
    {return includeOutOfBound;}

  size_t getMaximumBinValue() const
  {
    const_cast<HistogramConfiguration* >(this)->ensureBinsAreComputed();
    size_t max = 0;
    for (size_t i = 0; i < bins.size(); ++i)
      if (bins[i] > max)
        max = bins[i];
    if (leftOutOfBound > max && includeOutOfBound)
      max = leftOutOfBound;
    if (rightOutOfBound > max && includeOutOfBound)
      max = rightOutOfBound;
    return max;
  }
  
  size_t getLeftOutOfBound() const
  {
    const_cast<HistogramConfiguration* >(this)->ensureBinsAreComputed();
    return leftOutOfBound;
  }
  
  size_t getRightOutOfBound() const
  {
    const_cast<HistogramConfiguration* >(this)->ensureBinsAreComputed();
    return rightOutOfBound;
  }

protected:
  friend class HistogramConfigurationClass;

  double stepSize;
  double minValue;
  double maxValue;
  bool includeOutOfBound;
  std::vector<double> values;

private:
  std::vector<size_t> bins;
  size_t leftOutOfBound;
  size_t rightOutOfBound;
  
  void ensureBinsAreComputed()
  {
    if (bins.size() == getNumBins())
      return;

    bins.clear();
    bins.resize(getNumBins(), 0);
    leftOutOfBound = 0;
    rightOutOfBound = 0;

    for (size_t i = 0; i < values.size(); ++i)
    {
      if (values[i] < minValue)
        ++leftOutOfBound;
      else if (values[i] > maxValue)
        ++rightOutOfBound;
      else
        bins[(values[i] - minValue) / stepSize]++;
    }
  }
};

typedef ReferenceCountedObjectPtr<HistogramConfiguration> HistogramConfigurationPtr;

class HistogramComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  enum {minimumBinWidth = 15};

  HistogramComponent(HistogramConfigurationPtr configuration, const String& name)
    : configuration(configuration) {}

  virtual int getDefaultWidth() const
    {return (configuration->getNumBins() + (configuration->drawOutOfBound() ? 2 : 0)) * minimumBinWidth;}
  
  virtual int getDefaultHeight() const
    {return 300;}
  
  virtual void paint(juce::Graphics& g)
  {
    const int width = getWidth();
    const int height = getHeight();
    double currentY = 0.;

    juce::Font f = g.getCurrentFont();
    g.setFont(24, juce::Font::bold);
    g.drawText(configuration->getName(), 0, 0, width, 30, juce::Justification::centred, false);
    g.setFont(f);
    currentY += 40;

    drawHistogram(g, 20, currentY, width - 40, juce::jmin(300., height - currentY - 10));
  }

protected:
  HistogramConfigurationPtr configuration;

  void drawHistogram(juce::Graphics& g, float x, float y, float width, float height)
  {
    const float binWidth = width / ((double)configuration->getNumBins() + (configuration->drawOutOfBound() ? 2 : 0));
    const double binUnitHeight = height / (double)configuration->getMaximumBinValue();
    const size_t n = configuration->getNumBins();
    // draw bar
    for (size_t i = 0; i < n; ++i)
    {
      juce::Colour c = juce::Colour(i / (double)configuration->getNumBins(), 1.f, 0.75f, (juce::uint8)255);
      g.setColour(c);
      g.fillRect(x + (i + (configuration->drawOutOfBound() ? 1 : 0)) * binWidth, y + height - binUnitHeight * configuration->getBin(i), binWidth, binUnitHeight * configuration->getBin(i));
    }
    // draw border
    g.setColour(juce::Colours::black);
    for (size_t i = 0; i < n; ++i)
      g.drawRect(x + (i + (configuration->drawOutOfBound() ? 1 : 0)) * binWidth, y + height - binUnitHeight * configuration->getBin(i), binWidth, binUnitHeight * configuration->getBin(i), 1.);
    // draw out-of-bound
    if (configuration->drawOutOfBound())
    {
      g.fillRect(x, y + height - binUnitHeight * configuration->getLeftOutOfBound(), binWidth, binUnitHeight * configuration->getLeftOutOfBound());
      g.fillRect(x + n * binWidth, y + height - binUnitHeight * configuration->getRightOutOfBound(), binWidth, binUnitHeight * configuration->getRightOutOfBound());
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_HISTOGRAM_H_
