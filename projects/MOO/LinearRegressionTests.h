/*-------------------------------------------.-----------------------------------------.
 | Filename: LinearRegressionTests.h         | Testing Linear Regression Code          |
 | Author  : Denny Verbeeck                  |                                         |
 | Started : 11/06/2014 16:00                |                                         |
 `-------------------------------------------/                                         |
                                  |                                                    |
                                  `---------------------------------------------------*/

#ifndef MOO_LINEAR_REGRESSION_TESTS_H_
# define MOO_LINEAR_REGRESSION_TESTS_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class LinearRegressionTests : public WorkUnit
{
public:
  LinearRegressionTests() : randomSeed(0), datasetPath("") {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);

    std::vector<std::pair<string, IncrementalLearnerPtr> > learners;
    learners.push_back(std::make_pair("Incremental Linear Least Squares", linearLeastSquaresRegressionIncrementalLearner()));
    learners.push_back(std::make_pair("Perceptron", perceptronIncrementalLearner(50, 0.1, 0.005)));
    
    std::vector<std::pair<string, TablePtr> > data = loadData(context, datasetPath);
    std::map<string, std::vector<double> > trueWeights = getWeights();
    for(size_t i = 0; i < data.size(); ++i)
    {
      context.enterScope(data[i].first);
      TablePtr dataset = data[i].second;
      size_t numRows = dataset->getNumRows();
      size_t interval = numRows / 200;
      std::vector<ExpressionPtr> models(learners.size());
      for (size_t j = 0; j < learners.size(); ++j)
        models[j] = learners[j].second->createExpression(context, doubleClass);
      context.progressCallback(new ProgressionState(0, numRows, "Instances"));
      for (size_t k = 0; k < numRows; ++k)
      {
        if ((k+1) % interval == 0)
        {
          context.enterScope("Iteration " + string((int)k));
          context.resultCallback("Iteration", k);
        }
        for (size_t l = 0; l < learners.size(); ++l)
        {
          learners[l].second->addTrainingSample(context, dataset->getRow(k), models[l]);
          if ((k+1) % interval == 0)
            context.resultCallback(learners[l].first + "weight RMSE", getWeightRMSE(models[l], trueWeights[data[i].first]));
        }
        if ((k+1) % interval == 0)
          context.leaveScope();
        context.progressCallback(new ProgressionState(k+1, dataset->getNumRows(), "Instances"));
      }
      for (size_t l = 0; l < learners.size(); ++l)
        context.resultCallback(learners[l].first + " weight RMSE", getWeightRMSE(models[l], trueWeights[data[i].first]));
      context.leaveScope();
    }
    

    return new Boolean(true);
  }
  
protected:
  friend class LinearRegressionTestsClass;

  size_t randomSeed;
  string datasetPath;

  double getWeightRMSE(ExpressionPtr model, const std::vector<double>& weights)
  {
    if (!model.isInstanceOf<LinearModelExpression>())
      return -1.0;
    DenseDoubleVectorPtr modelWeights = model.staticCast<LinearModelExpression>()->getWeights();
    if (modelWeights->getNumValues() != weights.size())
      return -1.0;
    double error = 0.0;
    for (size_t i = 0; i < weights.size(); ++i)
      error += (modelWeights->getValue(i) - weights[i]) * (modelWeights->getValue(i) - weights[i]);
    error /= weights.size();
    return sqrt(error);
  }

  std::vector<std::pair<string, TablePtr> > loadData(ExecutionContext& context, string arffPath)
  {
    std::vector<std::pair<string, TablePtr> > data;
    std::vector<string> datasets;
    datasets.push_back("fried_delve.arff");
    datasets.push_back("cart_delve.arff");
    datasets.push_back("pol.arff");
    datasets.push_back("winequality-white.arff");
    datasets.push_back("cal_housing.arff");
    datasets.push_back("cal_housing_norm.arff");
    //datasets.push_back("CASP.arff");
    
    std::vector<string> problemnames;
    problemnames.push_back("Friedmann");
    problemnames.push_back("CART");
    problemnames.push_back("PoleTelecom");
    problemnames.push_back("Wine quality");
    problemnames.push_back("California housing");
    problemnames.push_back("California housing normalized");
    //problemnames.push_back("Physicochemical Properties of Protein Tertiary Structure");
    
    ArffLoader loader;
    for (size_t i = 0; i < datasets.size(); ++i)
    {
      TablePtr table = loader.loadFromFile(context, juce::File(arffPath + "/" + datasets[i])).staticCast<Table>();
      context.informationCallback("Finished loading " + datasets[i]);
      data.push_back(std::make_pair(problemnames[i], table));
    }
    return data;
  }

  std::map<string, std::vector<double> > getWeights()
  {
    std::map<string, std::vector<double> > result;
    double friedmannWeights[] = {4.34529070e-01, 6.59610949e+00, 6.64149911e+00, -1.82994107e-01,
                                 9.99457152e+00, 4.92421269e+00, 1.86019599e-02,  2.70657042e-03,
                                -2.70262186e-02, 2.84026606e-02, -6.74268369e-02};
    result["Friedmann"] = std::vector<double>(friedmannWeights, friedmannWeights + sizeof(friedmannWeights) / sizeof(double));
    double cartWeights[] = {-1.47634349e-02, 2.97304161e+00,  1.53435925e+00, 1.03568980e+00,
                             5.28100431e-01, 1.47663839e+00,  1.00437093e+00, 5.23752575e-01,
                            -1.08379791e-03, 2.38448618e-03, -2.78099690e-02};
    result["CART"] = std::vector<double>(cartWeights, cartWeights + sizeof(cartWeights) / sizeof(double));
    double poleWeights[] = {4.77509829e+02, -7.20579804e-01, -1.90619311e+00,  7.60499795e-02,
                           -1.00308650e+00, -1.65062985e+00,  8.66322219e-03,  2.11241922e-02,
                           6.56345410e-02,  7.10758104e-02,  1.45726936e-01,  3.57042916e-01,
                           2.90675758e-01,  5.49934706e-01,  8.20498496e-01,  4.31436359e-01,
                           -1.63354011e-01, -8.96004616e-01, -1.20491776e+00, -6.79210898e-01,
                           -2.99223014e-01,  3.03594905e-02,  3.54571377e-01,  3.77047200e-01,
                           5.99523278e-01,  1.12213130e+00,  1.42786044e+00};
    result["PoleTelecom"] = std::vector<double>(poleWeights, poleWeights + sizeof(poleWeights) / sizeof(double));
    double wineWeights[] = {1.50192842e+02, 6.55199612e-02,  -1.86317709e+00, 2.20902010e-02,
                            8.14828026e-02, -2.47276539e-01, 3.73276519e-03,  -2.85747419e-04,
                            -1.50284180e+02, 6.86343741e-01, 6.31476472e-01, 1.93475697e-01};
    result["Wine quality"] = std::vector<double>(wineWeights, wineWeights + sizeof(wineWeights) / sizeof(double));
    double calHousingWeights[] = {-3.59402294e+06, -4.28237438e+04, -4.25767219e+04,  1.15630387e+03, 
                                  -8.18164928e+00,  1.13410689e+02, -3.85350953e+01,  4.83082868e+01,
                                  4.02485142e+04};
    result["California housing"] = std::vector<double>(calHousingWeights, calHousingWeights + sizeof(calHousingWeights) / sizeof(double));
    double calHousingNormalizedWeights[] = {0.72570952, -0.88649242, -0.82607312, 0.12159015, -0.66326763, 
                                            1.50683763, -2.83481955, 0.6056915, 1.2033175};
    result["California housing normalized"] = std::vector<double>(calHousingNormalizedWeights, calHousingNormalizedWeights + sizeof(calHousingNormalizedWeights) / sizeof(double));
    return result;
  }
};

} /* namespace lbcpp */

#endif // MOO_LINEAR_REGRESSION_TESTS_H_
