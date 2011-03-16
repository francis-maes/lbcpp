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
    : directory(directory), extension(extension) {}
  SaveToDirectoryEvaluator() {}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
  {
    if (!directory.exists())
    {
      if (!directory.createDirectory())
      {
        context.errorCallback(T("Could not create directory ") + directory.getFullPathName());
        return ScoreObjectPtr();
      }
    }
    else if (!directory.isDirectory())
    {
      context.errorCallback(directory.getFullPathName() + T(" is not a directory"));
      return ScoreObjectPtr();
    }
    return new DummyScoreObject();
  }

  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {
    ObjectPtr object = output.getObject();
    if (!object)
    {
      context.errorCallback(T("Ouput is not an object"));
      return false;
    }
    object->saveToFile(context, directory.getChildFile(object->getName() + extension));
    return true;
  }

protected:
  friend class SaveToDirectoryEvaluatorClass;

  File directory;
  String extension;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_SAVE_TO_DIRECTORY_H_
