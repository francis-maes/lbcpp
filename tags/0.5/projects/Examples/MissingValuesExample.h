/*-----------------------------------------.---------------------------------.
| Filename: MissingValuesExample.h         | Illustrates missing values      |
| Author  : Francis Maes                   |                                 |
| Started : 08/07/2010 12:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_MISSING_VALUES_EXAMPLE_H_
# define LBCPP_EXAMPLES_MISSING_VALUES_EXAMPLE_H_

# include <lbcpp/Core/Variable.h>

namespace lbcpp
{

class VerySimpleClass : public Object
{
public:
  virtual String toString() const
    {return T("VerySimpleClass");}
};

class MissingValuesExample : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    Variable myBoolean(true);
    Variable myMissingBoolean = Variable::missingValue(booleanType);
    Variable myInt(51);
    Variable myMissingInt = Variable::missingValue(integerType);
    Variable myDouble(8.6);
    Variable myMissingDouble = Variable::missingValue(doubleType);
    Variable myString(T("Hello"));
    Variable myMissingString = Variable::missingValue(stringType);
    Variable myObject(new VerySimpleClass());
    Variable myMissingObject = Variable::missingValue(getType(T("VerySimpleClass")));
    Variable myPair = Variable::pair(myBoolean, myObject);
    Variable myMissingPair = Variable::missingValue(pairClass(booleanType, getType(T("VerySimpleClass"))));
    DenseDoubleVectorPtr myVector = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, probabilityType, 3);
    myVector->setValue(0, 0.0);
    myVector->setValue(1, 1.0);
    myVector->setValue(2, doubleType->getMissingValue().getDouble());
    
    // FIXME: use informationCallback and resultCallback
    std::cout << myBoolean << " " << myMissingBoolean << std::endl
              << myInt << " " << myMissingInt << std::endl
              << myDouble << " " << myMissingDouble << std::endl
              << myString << " " << myMissingString << std::endl
              << myObject << " " << myMissingObject << std::endl
              << myPair << " " << myMissingPair << std::endl
              << myVector->toString() << std::endl;
        
    std::cout << myBoolean.isMissingValue() << " " << myMissingBoolean.isMissingValue() << std::endl
              << myInt.isMissingValue() << " " << myMissingInt.isMissingValue() << std::endl
              << myDouble.isMissingValue() << " " << myMissingDouble.isMissingValue() << std::endl
              << myString.isMissingValue() << " " << myMissingString.isMissingValue() << std::endl
              << myObject.isMissingValue() << " " << myMissingObject.isMissingValue() << std::endl
              << myPair.isMissingValue() << " " << myMissingPair.isMissingValue() << std::endl;

    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_MISSING_VALUES_EXAMPLE_H_

