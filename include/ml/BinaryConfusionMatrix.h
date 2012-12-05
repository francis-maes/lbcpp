/*-----------------------------------------.---------------------------------.
| Filename: BinaryConfusionMatrix.h        | Binary Confusion Matrix         |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_BINARY_CONFUSION_MATRIX_H_
# define LBCPP_DATA_BINARY_CONFUSION_MATRIX_H_

# include <oil/Core/Object.h>

namespace lbcpp
{

class BinaryConfusionMatrix : public Object
{
public:
  BinaryConfusionMatrix(const BinaryConfusionMatrix& otherMatrix);
  BinaryConfusionMatrix();

  virtual string toString() const;

  void clear();
  void set(size_t truePositive, size_t falsePositive, size_t falseNegative, size_t trueNegative);
  void addPrediction(bool predicted, bool correct, size_t count = 1);
  void removePrediction(bool predicted, bool correct, size_t count = 1);

  void addPredictionIfExists(ExecutionContext& context, const ObjectPtr& predicted, const ObjectPtr& correct, size_t count = 1);

  double computeAccuracy() const;
  double computeF1Score() const;
  double computePrecision() const;
  double computeRecall() const;
  double computeMatthewsCorrelation() const;
  double computeSensitivity() const;
  double computeSpecificity() const;
  double computeSensitivityAndSpecificity() const;
  
  void computePrecisionRecallAndF1(double& precision, double& recall, double& f1score) const;

  size_t getSampleCount() const
    {return totalCount;}

  size_t getTruePositives() const
    {return truePositive;}

  size_t getFalsePositives() const
    {return falsePositive;}

  size_t getFalseNegatives() const
    {return falseNegative;}

  size_t getTrueNegatives() const
    {return trueNegative;}

  size_t getCount(bool predicted, bool correct) const;

  size_t getPositives() const
    {return truePositive + falseNegative;}

  size_t getNegatives() const
    {return trueNegative + falsePositive;}
  
  /* Object */
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

  bool operator ==(const BinaryConfusionMatrix& other) const;
  bool operator !=(const BinaryConfusionMatrix& other) const
    {return !(*this == other);}

protected:
  friend class BinaryConfusionMatrixClass;

 // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;

  bool convertToBoolean(ExecutionContext& context, const ObjectPtr& object, bool& res);
};

typedef ReferenceCountedObjectPtr<BinaryConfusionMatrix> BinaryConfusionMatrixPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_BINARY_CONFUSION_MATRIX_H_
