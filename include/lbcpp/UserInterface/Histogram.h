/*-----------------------------------------.---------------------------------.
| Filename: Histogram.h                    | Histogram                       |
| Author  : Julien Becker                  |                                 |
| Started : 09/06/2011 14:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_HISTOGRAM_H_
# define LBCPP_USER_INTERFACE_HISTOGRAM_H_

namespace lbcpp
{

class Histogram : public NameableObject
{
public:
  Histogram(double stepSize, double minValue, double maxValue, bool includeOutOfBound = true, const String& name = String::empty)
    : NameableObject(name), stepSize(stepSize), minValue(minValue), maxValue(maxValue), includeOutOfBound(includeOutOfBound)
    {jassert(minValue <= maxValue);}
  Histogram() {}

  void addData(double value)
  {
    values.push_back(value);
    bins.clear();
  }

  size_t getNumBins() const
    {return (size_t)(ceil((maxValue - minValue) / stepSize));}
  
  size_t getBin(size_t index) const
  {
    jassert(index < getNumBins());
    const_cast<Histogram* >(this)->ensureBinsAreComputed();
    return bins[index];
  }

  bool drawOutOfBound() const
    {return includeOutOfBound;}

  size_t getMaximumBinValue() const
  {
    const_cast<Histogram* >(this)->ensureBinsAreComputed();
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
    const_cast<Histogram* >(this)->ensureBinsAreComputed();
    return leftOutOfBound;
  }
  
  size_t getRightOutOfBound() const
  {
    const_cast<Histogram* >(this)->ensureBinsAreComputed();
    return rightOutOfBound;
  }

protected:
  friend class HistogramClass;

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
        bins[(size_t)((values[i] - minValue) / stepSize)]++;
    }
  }
};

typedef ReferenceCountedObjectPtr<Histogram> HistogramPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_HISTOGRAM_H_
