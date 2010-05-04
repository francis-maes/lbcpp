/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Evaluator.h"
#include "ObjectContainerEvaluator.h"
#include "ClassificationAccuracyEvaluator.h"
#include "BinaryClassificationConfusionEvaluator.h"
#include "RegressionErrorEvaluator.h"
#include "ScoreVectorSequenceErrorEvaluator.h"
using namespace lbcpp;

EvaluatorPtr lbcpp::classificationAccuracyEvaluator(const String& name)
  {return new ClassificationAccuracyEvaluator(name);}

EvaluatorPtr lbcpp::binaryClassificationConfusionEvaluator(const String& name)
  {return new BinaryClassificationConfusionEvaluator(name);}

EvaluatorPtr lbcpp::regressionErrorEvaluator(const String& name)
  {return new RegressionErrorEvaluator(name);}

EvaluatorPtr lbcpp::objectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator)
  {return new ObjectContainerEvaluator(name, objectEvaluator);}

EvaluatorPtr lbcpp::scoreVectorSequenceRegressionErrorEvaluator(const String& name)
  {return new ScoreVectorSequenceRegressionEvaluator(name);}
