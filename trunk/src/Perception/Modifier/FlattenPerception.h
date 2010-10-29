/*-----------------------------------------.---------------------------------.
| Filename: FlattenPerception.h            | A decorator to flatten a        |
| Author  : Francis Maes                   | Perception                      |
| Started : 12/07/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_
# define LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class FlattenPerception : public Perception
{
public:
  FlattenPerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated)
    {computeOutputType();}

  virtual bool isSparse() const
    {return decorated->isSparse();}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual String toString() const
    {return decorated->toString() + T(" flattened");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    FlattenCallback callback(decorated, targetCallback, &offsets);
    decorated->computePerception(input, &callback);
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class FlattenPerceptionClass;

  PerceptionPtr decorated;

  struct OffsetInfo
  {
    OffsetInfo() : offset(0), subInfo(NULL) {}
    ~OffsetInfo()
      {if (subInfo) delete [] subInfo;}

    size_t offset;
    OffsetInfo* subInfo;

    void set(size_t offset, size_t numSubInfos)
    {
      jassert(!subInfo);
      this->offset = offset;
      subInfo = new OffsetInfo[numSubInfos]();
    }
  };

  OffsetInfo offsets;

  virtual void computeOutputType()
  {
    precompute(decorated, String::empty, offsets);
    Perception::computeOutputType();
  }

  void precompute(PerceptionPtr perception, const String& fullName, OffsetInfo& offsets)
  {
    TypePtr perceptionOutputType = perception->getOutputType();
    size_t n = perceptionOutputType->getObjectNumVariables();

    offsets.set(outputVariables.size(), n);

    for (size_t i = 0; i < n; ++i)
    {
      String name = fullName;
      if (name.isNotEmpty())
        name += '.';
      name += perception->getOutputVariableName(i);

      PerceptionPtr subPerception = perception->getOutputVariableSubPerception(i);
      if (subPerception)
        precompute(subPerception, name, offsets.subInfo[i]);
      else
        addOutputVariable(name, perception->getOutputVariableType(i));
    }
  }

  struct FlattenCallback : public PerceptionCallback
  {
    FlattenCallback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const OffsetInfo* offsets)
      : targetCallback(targetCallback), currentOffset(offsets)
      {}

    virtual void sense(size_t variableNumber, const Variable& value)
      {targetCallback->sense(variableNumber + currentOffset->offset, value);}

    virtual void sense(size_t variableNumber, double value)
      {targetCallback->sense(variableNumber + currentOffset->offset, value);}

    virtual void sense(size_t variableNumber, const ObjectPtr& value)
      {targetCallback->sense(variableNumber + currentOffset->offset, value);}

    virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
    {
      const OffsetInfo* offsetBackup = currentOffset;
      jassert(currentOffset->subInfo);
      currentOffset = currentOffset->subInfo + variableNumber;
      subPerception->computePerception(input, this);
      currentOffset = offsetBackup;
    }

  private:
    PerceptionCallbackPtr targetCallback;
    const OffsetInfo* currentOffset;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FLATTEN_H_
