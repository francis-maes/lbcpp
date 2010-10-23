/*-----------------------------------------.---------------------------------.
| Filename: SaveToDirectoryEvaluator.h     | An evaluator that saves         |
| Author  : Francis Maes                   |  predictions to a directory     |
| Started : 23/10/2010 12:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_FUNCTION_EVALUATOR_SAVE_TO_DIRECTORY_H_
# define LBCPP_FUNCTION_EVALUATOR_SAVE_TO_DIRECTORY_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class SaveToDirectoryEvaluator : public Evaluator
{
public:
  SaveToDirectoryEvaluator(const File& directory, const String& extension)
    : Evaluator(T("Save To ") + directory.getFileName()), directory(directory), extension(extension), savedCount(0) {}
  SaveToDirectoryEvaluator() : savedCount(0) {}

  virtual void addPrediction(const Variable& predicted, const Variable& correct)
  {
    if (predicted.exists())
    {
      ObjectPtr object = predicted.getObject();
      jassert(object);
      object->saveToFile(directory.getChildFile(object->getName() + extension));
    }
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {res.push_back(std::make_pair(T("SavedCount"), (double)savedCount));}

  virtual double getDefaultScore() const
    {return (double)savedCount;}

protected:
  friend class SaveToDirectoryEvaluatorClass;

  File directory;
  String extension;
  size_t savedCount;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_SAVE_TO_DIRECTORY_H_
