/*-----------------------------------------.---------------------------------.
| Filename: SaveScoresToGnuPlotFileOnli...h| Export scores in a File readable|
| Author  : Francis Maes                   |  by gnuplot                     |
| Started : 01/11/2010 19:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_SAVE_SCORES_TO_GNUPLOT_FILE_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_SAVE_SCORES_TO_GNUPLOT_FILE_H_

# include <lbcpp/Inference/InferenceOnlineLearner.h>

namespace lbcpp
{

class SaveScoresToGnuPlotFileOnlineLearner : public UpdatableOnlineLearner
{
public:
  SaveScoresToGnuPlotFileOnlineLearner(const File& outputFile, LearnerUpdateFrequency updateFrequency)
    : UpdatableOnlineLearner(updateFrequency), numLines(0)
  {
    outputFile.deleteFile();
    ostr = outputFile.createOutputStream();
  }
  SaveScoresToGnuPlotFileOnlineLearner() : ostr(NULL), numLines(0) {}

  ~SaveScoresToGnuPlotFileOnlineLearner()
    {if (ostr) delete ostr;}

  virtual void update(InferenceContextWeakPtr context, const InferencePtr& inference)
  {
    std::vector< std::pair<String, double> > scores;
    getScores(scores);
    scores.insert(scores.begin(), std::make_pair(T("defaultScore"), getDefaultScore()));
    
    if (numLines == 0)
    {
      *ostr << "# GnuPlot File Generated on " << Time::getCurrentTime().toString(true, true, true, true) << "\n";
      *ostr << "# iteration";
      for (size_t i = 0; i < scores.size(); ++i)
        *ostr << " " << scores[i].first;
      *ostr << "\n";
    }

    ++numLines;

    *ostr << String((int)numLines);
    for (size_t i = 0; i < scores.size(); ++i)
      *ostr << " " << scores[i].second;
    *ostr << "\n";

    ostr->flush();
  }

protected:
  juce::OutputStream* ostr;
  size_t numLines;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_SAVE_SCORES_TO_GNUPLOT_FILE_H_
