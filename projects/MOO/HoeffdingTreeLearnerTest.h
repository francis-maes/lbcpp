# include <oil/Execution/WorkUnit.h>
# include "HoeffdingTreeLearner.h"
# include <stdlib.h> // abs
# define _USE_MATH_DEFINES
# include <math.h> // sin

namespace lbcpp {

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class HoeffdingTreeLearnerTest: public WorkUnit {
public:
	int randomSeed;
	int nbSamples;

	HoeffdingTreeLearnerTest() :
			randomSeed(0), nbSamples(80) {
	}

public:
	virtual ObjectPtr run(ExecutionContext& context) {
		lbCppMLLibraryCacheTypes(context);
		//pick a random seed
		context.getRandomGenerator()->setSeed(randomSeed);

		// Fried dataset
		/*DataDefinition* dataDef = new DataDefinition();
		dataDef->addAttribute("numAtt0");
		dataDef->addAttribute("numAtt1");
		dataDef->addAttribute("numAtt2");
		dataDef->addAttribute("numAtt3");
		dataDef->addAttribute("numAtt4");
		dataDef->addTargetAttribute("targetValue");
		HoeffdingTreeLearner htl = HoeffdingTreeLearner(context, 0.01, *dataDef);

		context.enterScope("HoeffdingTreeTest");
		context.enterScope("testFunctions");

		double x1, x2, x3, x4, x5, y, noise;
		for (size_t i = 0; i < nbSamples; i++) {
			std::vector<float> sample;
			x1 = MathUtils::randDouble();
			x2 = MathUtils::randDouble();
			x3 = MathUtils::randDouble();
			x4 = MathUtils::randDouble();
			x5 = MathUtils::randDouble();
			sample.push_back(x1);
			sample.push_back(x2);
			sample.push_back(x3);
			sample.push_back(x4);
			sample.push_back(x5);
			noise = MathUtils::randDouble();
			y = 10*sin(M_PI*x1*x2)+20*(x3-0.5)*(x3-0.5)+10*x4+5*x5+noise;
			sample.push_back(y);

			context.enterScope(string((double) i));
			context.resultCallback("i", (double) i);
			htl.addTrainingSample(sample);

			context.resultCallback("prediction",htl.predict(sample));
			context.resultCallback("predictionError", abs(y - htl.predict(sample))/y);

			context.leaveScope();
		}*/

		// setup htl learner
		DataDefinition* dataDef = new DataDefinition();
		dataDef->addAttribute("numAtt0");
		dataDef->addAttribute("numAtt1");
		dataDef->addTargetAttribute("targetValue");
		HoeffdingTreeLearner htl = HoeffdingTreeLearner(context, 0.01, *dataDef);

		context.enterScope("HoeffdingTreeTest");
		context.enterScope("testFunctions");

		double x1, x2, y;
		for (size_t i = 0; i < nbSamples; i++) {
			std::vector<float> sample;
			if (i % 4 == 0) {
				x1 = MathUtils::randDouble() / 2;
				x2 = MathUtils::randDouble() / 10 * 3;
				sample.push_back(x1);
				sample.push_back(x2);
				y = 0.3 * sample[0] + 0.7 * sample[1] + 2;
				sample.push_back(y);
			} else if (i % 4 == 1) {
				x1 = MathUtils::randDouble() / 2;
				x2 = MathUtils::randDouble() / 10 * 7 + 3.0 / 10;
				sample.push_back(x1);
				sample.push_back(x2);
				y = -0.5 * sample[0] - 0.2 * sample[1] + 3;
				sample.push_back(y);
			} else if (i % 4 == 2) {
				x1 = MathUtils::randDouble() / 2 + 0.5;
				x2 = MathUtils::randDouble() / 10 * 6;
				sample.push_back(x1);
				sample.push_back(x2);
				y = -0.2 * sample[0] + 0.3 * sample[1] - 1;
				sample.push_back(y);
			} else {
				x1 = MathUtils::randDouble() / 2 + 0.5;
				x2 = MathUtils::randDouble() / 10 * 4 + 6.0 / 10;
				sample.push_back(x1);
				sample.push_back(x2);
				y = 0.6 * sample[0] - 0.4 * sample[1] - 3;
				sample.push_back(y);
			}
			std::cout << "sample" << i << " =" << x1 << ", " << x2 << " , " << y << "\n";
			context.enterScope(string((double) i));
			context.resultCallback("i", (double) i);

			std::vector<float> samplex;
			samplex.push_back(x1);
			samplex.push_back(x2);
			samplex.push_back(y);

			LeafNode* leaf = htl.traverseSample(samplex);

			htl.addTrainingSample(sample);

			context.resultCallback("prediction",htl.predict(samplex));
			if (i % 4 == 0)
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);
			else if (i % 4 == 1)
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);
			else if (i % 4 == 2)
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);
			else
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);

			context.leaveScope();
		}

		context.leaveScope();
		context.leaveScope();

		/*cout << "Result:\n";
		 std::vector<float> sample;
		 sample.push_back(MathUtils::randDouble()/2);
		 sample.push_back(MathUtils::randDouble()/2);
		 LeafNode* leaf = htl.traverseSample(sample);
		 cout << "w0: " << leaf->linearModel->numericalWeights[0] << "\n";
		 cout << "w1: " << leaf->linearModel->numericalWeights[1] << "\n";
		 cout << "t: " << leaf->linearModel->threshold << "\n";*/
		std::cout << "succesfull end \n";

		return new Boolean(true);
	}

};

}
;
/* namespace lbcpp */
