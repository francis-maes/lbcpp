/*-----------------------------------------.---------------------------------.
| Filename: MultiSequenceComponent.h       | Components to display multiple  |
| Author  : Francis Maes                   |  aligned sequences              |
| Started : 14/06/2010 18:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_
# define EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_

# include "../Components/common.h"
# include "../Utilities/ComponentWithPreferedSize.h"
# include "../../projects/Proteins/InferenceData/LabelSequence.h"
# include "../../projects/Proteins/InferenceData/ScalarSequence.h"
# include "../../projects/Proteins/InferenceData/ScoreVectorSequence.h"

namespace lbcpp
{

class MultiSequenceComponent : public Component, public ComponentWithPreferedSize
{
public:
  MultiSequenceComponent() : numSequences(0), longestSequence(0) {}
  
  void clearSequences()
    {sequences.clear();}

  void addSequenceGroup(const String& friendlyName, const std::vector< std::pair<String, ObjectContainerPtr> >& sequences)
  {
    this->sequences.push_back(std::make_pair(friendlyName, sequences));
    numSequences += sequences.size();
    for (size_t i = 0; i < sequences.size(); ++i)
    {
      size_t length = sequences[i].second->size();
      if (length > longestSequence)
        longestSequence = length;
    }
  }

  enum
  {
    marginLeft = 100,
    marginRight = 5,
    elementWidth = 20,
    elementHeight = 20,
    marginVertical = 20,
  };

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    int numElementsPerLine = (availableWidth - marginLeft - marginRight) / elementWidth;
    if (numElementsPerLine < 1)
      return availableHeight;
    int numParts = (int)ceil(longestSequence / (double)numElementsPerLine);
    return numParts * (numSequences * elementHeight + marginVertical);
  }

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::white);
    int numElementsPerLine = (getWidth() - marginLeft - marginRight) / elementWidth;
    int numParts = (int)ceil(longestSequence / (double)numElementsPerLine);

    int y = marginVertical / 2;
    size_t begin = 0;
    for (int i = 0; i < numParts; ++i)
    {
      size_t end = begin + numElementsPerLine;
      if (end > longestSequence)
        end = longestSequence;
      paintSequencesInterval(g, begin, end, marginLeft, y);
      begin = end;
      y += elementHeight * numSequences + marginVertical;
    }
  }

private:
  std::vector< std::pair<String, std::vector< std::pair<String, ObjectContainerPtr> > > > sequences;
  size_t numSequences, longestSequence;
  
  void paintSequencesInterval(Graphics& g, size_t begin, size_t end, int x1, int y1)
  {
    g.setFont(10);
    g.setColour(Colours::grey);
    for (size_t i = begin; i < end; ++i)
      if (i == begin || i == end - 1 || ((i + 1) % 5) == 0)
        g.drawText(String((int)(i + 1)), x1 + (i - begin) * elementWidth, y1 - 10, elementWidth, 10, Justification::centred, true);

    bool hasMultiSequences = false;
    for (size_t i = 0; i < sequences.size(); ++i)
      if (sequences[i].second.size() > 1)
      {
        hasMultiSequences = true;
        break;
      }
    
    int y = y1;
    for (size_t i = 0; i < sequences.size(); ++i)
    {
      std::vector< std::pair<String, ObjectContainerPtr> >& seq = sequences[i].second;
      for (size_t j = 0; j < seq.size(); ++j)
      {
        g.setFont(9);
        g.setColour(Colours::black);
        if (hasMultiSequences)
        {
          g.drawText(sequences[i].first, 0, y + elementHeight / 2, marginLeft - 5, elementHeight / 2, Justification::centredRight, true);
          g.setColour(Colours::grey);
          g.drawText(seq[j].first, 0, y, marginLeft - 5, elementHeight / 2, Justification::centredRight, true);
        }
        else
          g.drawText(sequences[i].first, 0, y, marginLeft - 5, elementHeight, Justification::centredRight, true);
        paintSequenceInterval(g, begin, end, x1, y, seq[j].first, seq[j].second);
        y += elementHeight;
      }
    }
  }
  
  void paintSequenceInterval(Graphics& g, size_t begin, size_t end, int x1, int y1, const String& versionName, ObjectContainerPtr sequence)
  {
    int y2 = y1 + elementHeight;
    
    g.setColour(Colours::black);
    g.drawLine((float)x1, (float)y1, (float)(x1 + (end - begin) * elementWidth), (float)y1);
    g.drawLine((float)x1, (float)y2, (float)(x1 + (end - begin) * elementWidth), (float)y2);
    
    int x = x1;
    for (size_t i = begin; i < end; ++i)
    {
      g.setColour(Colours::black);
      g.drawLine((float)x, (float)y1, (float)x, (float)(y2 + 1));
      paintSequenceElement(g, x + 1, y1 + 1, elementHeight - 1, elementWidth - 1, sequence, i);
      x += elementWidth;
    }
    g.setColour(Colours::black);
    g.drawLine((float)x, (float)y1, (float)x, (float)(y2 + 1));
  }

  void paintSequenceElement(Graphics& g, int x, int y, int w, int h, ObjectContainerPtr sequence, size_t index)
  {
    LabelSequencePtr labelSequence = sequence.dynamicCast<LabelSequence>();
    if (labelSequence)
    {
      g.setFont(12);
      g.drawText(labelSequence->getString(index), x, y, w, h, Justification::centred, true);
      return;
    }
    
    ScalarSequencePtr scalarSequence = sequence.dynamicCast<ScalarSequence>();
    if (scalarSequence)
    {
      String str = T("?");
      if (scalarSequence->hasValue(index))
      {
        g.setColour(Colours::red);
        double value = juce::jlimit(0.0, 1.0, scalarSequence->getValue(index));
        g.fillRect(x, y + (int)(h * (1 - value)), w, (int)(h * value));
        str = String(scalarSequence->getValue(index), 2);
      }
      g.setColour(Colours::black);
      g.setFont(8);
      g.drawText(str, x, y, w, h, Justification::centred, true);
    }
    
    ScoreVectorSequencePtr scoreVectorSequence = sequence.dynamicCast<ScoreVectorSequence>();
    if (scoreVectorSequence)
    {
      for (size_t i = 0; i < scoreVectorSequence->getNumScores(); ++i)
      {
        juce::uint8 level = (juce::uint8)(255 * juce::jlimit(0.0, 1.0, 1.0  - scoreVectorSequence->getScore(index, i)));
        g.setColour(Colour(255, level, level));
        int y1 = y + i * h / scoreVectorSequence->getNumScores();
        int y2 = y + (i + 1) * h / scoreVectorSequence->getNumScores();
        g.fillRect(x, y1, w, y2 - y1);
      }
    }
  }
  
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_

