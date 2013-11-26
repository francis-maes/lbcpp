# include <oil/Execution/WorkUnit.h>
# include "HoeffdingTreeLearner.h"
# include <stdlib.h> // abs
# define _USE_MATH_DEFINES
# include <math.h> // sin
# include <ctime>

namespace lbcpp {

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class HoeffdingTreeLearnerTest: public WorkUnit {
public:
	int randomSeed;
	int nbSamples;
	int nbTestSamples;
	bool modelNY;
	bool modelNXY;

	HoeffdingTreeLearnerTest() :
			randomSeed(0), nbSamples(10000), nbTestSamples(3000), modelNY(false), modelNXY(false) {
	}

public:
	virtual ObjectPtr run(ExecutionContext& context) {
		lbCppMLLibraryCacheTypes(context);
		//pick a random seed
		context.getRandomGenerator()->setSeed(randomSeed);

		//ModelType modelType = static_cast<ModelType>(modelTypeInt);

		// Fried dataset
		DataDefinition* dataDef = new DataDefinition();
		dataDef->addAttribute("numAtt0");
		dataDef->addAttribute("numAtt1");
		dataDef->addAttribute("numAtt2");
		dataDef->addAttribute("numAtt3");
		dataDef->addAttribute("numAtt4");
		dataDef->addTargetAttribute("targetValue");
		HoeffdingTreeLearner htlNY = HoeffdingTreeLearner(context, NY, 0.01, *dataDef);
		HoeffdingTreeLearner htlNXY = HoeffdingTreeLearner(context, NXY, 0.01, *dataDef);
		if(!modelNY && !modelNXY)
			new Boolean(false);

		context.enterScope("HoeffdingTreeTest");
		context.enterScope("testFunctions");

		std::clock_t start;
		start = std::clock();

		// START TRAINING
		float x1, x2, x3, x4, x5, y, noise;
		for (int i = 0; i < nbSamples; i++) {
			std::vector<float> sample;
			x1 = (float)MathUtils::randDouble();
			x2 = (float)MathUtils::randDouble();
			x3 = (float)MathUtils::randDouble();
			x4 = (float)MathUtils::randDouble();
			x5 = (float)MathUtils::randDouble();
			sample.push_back(x1);
			sample.push_back(x2);
			sample.push_back(x3);
			sample.push_back(x4);
			sample.push_back(x5);
			noise = (float)MathUtils::randDouble();
			y = (float)(10*sin(M_PI*x1*x2)+20*(x3-0.5)*(x3-0.5)+10*x4+5*x5+noise);
			sample.push_back(y);

			context.enterScope(string((double) i));
			context.resultCallback("i", (double) i);
			if(modelNY){
				htlNY.addTrainingSample(sample);
				context.resultCallback("prediction NY",htlNY.predict(sample));
				context.resultCallback("predictionError NY", abs(y - htlNY.predict(sample))/y);
			}
			if(modelNXY){
				htlNXY.addTrainingSample(sample);
				context.resultCallback("prediction NXY",htlNXY.predict(sample));
				context.resultCallback("predictionError NXY", abs(y - htlNXY.predict(sample))/y);
			}
			context.resultCallback("Time: ", (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000));

			context.leaveScope();
			if(i % 10000 == 0){
				std::cout << i << std::endl;
			}
		}
		///END TRAINING

		std::cout << "results: " << std::endl;
		if(modelNY)
			std::cout << "NbOfLeaves NY: " << htlNY.getNbOfLeaves() << std::endl;
		if(modelNXY)
			std::cout << "NbOfLeaves NXY: " << htlNXY.getNbOfLeaves() << std::endl;
		std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << std::endl;

		double reNY = 0; // relative error
		double reNXY = 0;

		// START TESTING
		start = std::clock();

		for (int i = 0; i < nbTestSamples; i++) {
			std::vector<float> sample;
			x1 = (float)MathUtils::randDouble();
			x2 = (float)MathUtils::randDouble();
			x3 = (float)MathUtils::randDouble();
			x4 = (float)MathUtils::randDouble();
			x5 = (float)MathUtils::randDouble();
			sample.push_back(x1);
			sample.push_back(x2);
			sample.push_back(x3);
			sample.push_back(x4);
			sample.push_back(x5);
			noise = (float)MathUtils::randDouble();
			y = (float)(10*sin(M_PI*x1*x2)+20*(x3-0.5)*(x3-0.5)+10*x4+5*x5+noise);
			sample.push_back(y);

			//context.enterScope(string((double) i));
			//context.resultCallback("i", (double) i);
			//context.resultCallback("prediction",htl.predict(sample));
			//context.resultCallback("predictionError", abs(y - htl.predict(sample))/y);
			//context.resultCallback("Time: ", (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000));

			if(modelNY)
				reNY+= abs(y-htlNY.predict(sample))/y;
			if(modelNXY)
				reNXY+= abs(y-htlNXY.predict(sample))/y;

			//context.leaveScope();
			if(i % 10000 == 0){
				std::cout << i << std::endl;
			}
		}

		reNY /= 300000;
		reNXY /= 300000;
		if(modelNY)
			std::cout << "Relative error NY: " << reNY << std::endl;
		if(modelNXY)
			std::cout << "Relative error NXY: " << reNXY << std::endl;

		// END TESTING

		// setup htl learner
		/*DataDefinition* dataDef = new DataDefinition();
		dataDef->addAttribute("numAtt0");
		dataDef->addAttribute("numAtt1");
		dataDef->addTargetAttribute("targetValue");
		HoeffdingTreeLearner htl = HoeffdingTreeLearner(context, modelType, 0.01, *dataDef);

		context.enterScope("HoeffdingTreeTest");
		context.enterScope("training");

		std::clock_t start;
		start = std::clock();

		double re = 0;
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
			//std::cout << "sample" << i << " =" << x1 << ", " << x2 << " , " << y << "\n";
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

			context.resultCallback("Time: ", (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000));
			re+=abs((y - htl.predict(samplex))/y);
			context.leaveScope();
		}
		std::cout << re/nbSamples << std::endl;*/



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
