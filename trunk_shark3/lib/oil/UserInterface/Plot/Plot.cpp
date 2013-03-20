/*-----------------------------------------.---------------------------------.
| Filename: Plot.cpp                       | Plot                            |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 11:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <oil/Execution/ExecutionContext.h>
#include <oil/Core/RandomGenerator.h>
#include <oil/UserInterface/Plot.h>
using namespace lbcpp;

using juce::Colour;
using juce::Colours;

inline Colour randomColour()
{
  RandomGeneratorPtr random = defaultExecutionContext().getRandomGenerator();
  return Colour(random->sampleByte(), random->sampleByte(), random->sampleByte(), (unsigned char)255);
}

Plot::Plot(TablePtr data)
  : data(data), 
    xAxis(new PlotAxis(0.0, 1000.0)),
    yAxis(new PlotAxis(0.0, 1.0)),
    keyVariableIndex(0)
{
  const juce::Colour defaultColours[] = {
      Colours::red, Colours::green, Colours::blue, Colours::black,
      Colours::yellow.withBrightness(0.5f), Colours::cyan, Colours::violet, Colours::grey,
      Colours::darkred, Colours::darkgreen, Colours::darkblue,
  };
  const size_t numDefaultColours = sizeof (defaultColours) / sizeof (juce::Colour);

  for (size_t i = 0; i < data->getNumColumns(); ++i)
  {
    ClassPtr columnType = data->getType(i);
    if (columnType->isConvertibleToDouble())
    {
      size_t index = variables.size();
      variables.push_back(new PlotVariable(data->getKey(i), index > 0 && data->getKey(i)->toShortString() != JUCE_T("returnValue"), index < numDefaultColours ? defaultColours[index] : randomColour()));
    }
  }
}

std::vector<size_t> Plot::getSelectedCurves() const
{
  std::vector<size_t> res;
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i]->isSelected())
      res.push_back(i);
  return res;
}
