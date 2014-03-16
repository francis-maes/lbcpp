//# include <oil/Execution/WorkUnit.h>
//# include <ml/IncrementalLearner.h>
//# include <stdlib.h> // abs
//# define _USE_MATH_DEFINES
//# include <math.h> // sin
//# include <ctime>
//# include "../../lib/ml/IncrementalLearner/HoeffdingTreeIncrementalLearner.h"
//#include <iostream>
//#include <fstream>
//
//
//namespace lbcpp {
//
//extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp
//
//class HoeffdingTreeLearnerTest: public WorkUnit {
//public:
//	int randomSeed;
//	int nbSamples;
//	int nbTestSamples;
//	bool modelNY;
//	bool modelNXY;
//
//	HoeffdingTreeLearnerTest() :
//			randomSeed(0), nbSamples(10000), nbTestSamples(3000), modelNY(false), modelNXY(false) {
//	}
//
//public:
//	virtual ObjectPtr run(ExecutionContext& context) {
//		lbCppMLLibraryCacheTypes(context);
//		//pick a random seed
//		context.getRandomGenerator()->setSeed(randomSeed);
//
//		//ModelType modelType = static_cast<ModelType>(modelTypeInt);
//
//		// Fried dataset
//		//DataDefinition* dataDef = new DataDefinition();
//		//dataDef->addAttribute("numAtt0");
//		/*dataDef->addAttribute("numAtt1");
//		dataDef->addAttribute("numAtt2");
//		dataDef->addAttribute("numAtt3");
//		dataDef->addAttribute("numAtt4");*/
//		//dataDef->addTargetAttribute("targetValue");
//		HoeffdingTreeIncrementalLearnerPtr htlNY = hoeffdingTreeIncrementalLearner(context, randomSeed, NY, Hoeffding, 0.01).staticCast<HoeffdingTreeIncrementalLearner>();
//	    HoeffdingTreeIncrementalLearnerPtr htlNXY = hoeffdingTreeIncrementalLearner(context, randomSeed, NXY, Hoeffding, 0.01).staticCast<HoeffdingTreeIncrementalLearner>();
//		// make root (temp hack)
//		htlNY->createExpression(context, doubleClass);
//		htlNXY->createExpression(context, doubleClass);
//		std::vector<std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr> > testSet = createTestSet(nbTestSamples);
//		if(!modelNY && !modelNXY)
//			new Boolean(false);
//
//		context.enterScope("HoeffdingTreeTest");
//		context.enterScope("testFunctions");
//
//		std::clock_t start;
//		start = std::clock(); 
//
//		// START TRAINING
//		double y;
//		for (int i = 0; i < nbSamples; i++) {
//			std::cout << nbSamples << " - " << i << std::endl;
//			DenseDoubleVectorPtr input = new DenseDoubleVector(0, 0.0);
//			DenseDoubleVectorPtr output = new DenseDoubleVector(0, 0.0);
//			getSample(input, output);
//			y = output->getValue(0);
//
//			context.enterScope(string((double) i));
//			//context.resultCallback("x", (double) sample[0]);
//			context.resultCallback("i", (double) i);
//			if(modelNY){
//				htlNY->addTrainingSample(context, htlNY->root, input, output);
//				context.resultCallback("prediction NY",htlNY->predict(input));
//				//context.resultCallback("predictionError NY", abs(y - htlNY.predict(sample))/y);
//				context.resultCallback("RMSE NY", getRMSE(context, testSet, htlNY));
//				//context.resultCallback("nbOfLeaves NY", (double)htlNY->getNbOfLeaves());
//
//			}
//			if(modelNXY){
//				htlNXY->addTrainingSample(context, htlNXY->root, input, output);
//				context.resultCallback("prediction NXY",htlNXY->predict(input));
//				//context.resultCallback("predictionError NXY", abs(y - htlNXY.predict(sample))/y);
//				context.resultCallback("RMSE NXY", getRMSE(context, testSet, htlNXY));
//				//context.resultCallback("nbOfLeaves NXY", (double)htlNXY.getNbOfLeaves());
//			}
//			context.resultCallback("Training Time: ", (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000));
//
//			context.leaveScope();
//			if(i % 10000 == 0){
//				std::cout << i << std::endl;
//			}
//		}
//		///END TRAINING
//
//		/*std::cout << "results: " << std::endl;
//		if(modelNY)
//			std::cout << "NbOfLeaves NY: " << htlNY.getNbOfLeaves() << std::endl;
//		if(modelNXY)
//			std::cout << "NbOfLeaves NXY: " << htlNXY.getNbOfLeaves() << std::endl;*/
//		std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << std::endl;
//
//		double reNY = 0; // relative error
//		double reNXY = 0;
//
//		// START TESTING
//		start = std::clock();
//
//		for (int i = 0; i < nbTestSamples; i++) {
//			std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr> sample = testSet[i];
//			y = sample.second->getValue(0);
//
//			if(modelNY)
//				reNY+= abs(y-htlNY->predict(sample.first))/y;
//			if(modelNXY)
//				reNXY+= abs(y-htlNXY->predict(sample.first))/y;
//
//			//context.leaveScope();
//			if(i % 10000 == 0){
//				std::cout << i << std::endl;
//			}
//		}
//
//		reNY /= 300000;
//		reNXY /= 300000;
//		if(modelNY)
//			std::cout << "Relative error NY: " << reNY << std::endl;
//		if(modelNXY)
//			std::cout << "Relative error NXY: " << reNXY << std::endl;
//
//		// END TESTING
//
//		// setup htl learner
//		/*DataDefinition* dataDef = new DataDefinition();
//		dataDef->addAttribute("numAtt0");
//		dataDef->addAttribute("numAtt1");
//		dataDef->addTargetAttribute("targetValue");
//		HoeffdingTreeLearner htl = HoeffdingTreeLearner(context, modelType, 0.01, *dataDef);
//
//		context.enterScope("HoeffdingTreeTest");
//		context.enterScope("training");
//
//		std::clock_t start;
//		start = std::clock();
//
//		double re = 0;
//		double x1, x2, y;
//		for (size_t i = 0; i < nbSamples; i++) {
//			std::vector<double> sample;
//			if (i % 4 == 0) {
//				x1 = MathUtils::randDouble() / 2;
//				x2 = MathUtils::randDouble() / 10 * 3;
//				sample.push_back(x1);
//				sample.push_back(x2);
//				y = 0.3 * sample[0] + 0.7 * sample[1] + 2;
//				sample.push_back(y);
//			} else if (i % 4 == 1) {
//				x1 = MathUtils::randDouble() / 2;
//				x2 = MathUtils::randDouble() / 10 * 7 + 3.0 / 10;
//				sample.push_back(x1);
//				sample.push_back(x2);
//				y = -0.5 * sample[0] - 0.2 * sample[1] + 3;
//				sample.push_back(y);
//			} else if (i % 4 == 2) {
//				x1 = MathUtils::randDouble() / 2 + 0.5;
//				x2 = MathUtils::randDouble() / 10 * 6;
//				sample.push_back(x1);
//				sample.push_back(x2);
//				y = -0.2 * sample[0] + 0.3 * sample[1] - 1;
//				sample.push_back(y);
//			} else {
//				x1 = MathUtils::randDouble() / 2 + 0.5;
//				x2 = MathUtils::randDouble() / 10 * 4 + 6.0 / 10;
//				sample.push_back(x1);
//				sample.push_back(x2);
//				y = 0.6 * sample[0] - 0.4 * sample[1] - 3;
//				sample.push_back(y);
//			}
//			//std::cout << "sample" << i << " =" << x1 << ", " << x2 << " , " << y << "\n";
//			context.enterScope(string((double) i));
//			context.resultCallback("i", (double) i);
//
//			std::vector<double> samplex;
//			samplex.push_back(x1);
//			samplex.push_back(x2);
//			samplex.push_back(y);
//
//			LeafNode* leaf = htl.traverseSample(samplex);
//
//			htl.addTrainingSample(sample);
//
//			context.resultCallback("prediction",htl.predict(samplex));
//			if (i % 4 == 0)
//				context.resultCallback("predictionError",
//						abs(y - htl.predict(samplex))/y);
//			else if (i % 4 == 1)
//				context.resultCallback("predictionError",
//						abs(y - htl.predict(samplex))/y);
//			else if (i % 4 == 2)
//				context.resultCallback("predictionError",
//						abs(y - htl.predict(samplex))/y);
//			else
//				context.resultCallback("predictionError",
//						abs(y - htl.predict(samplex))/y);
//
//			context.resultCallback("Time: ", (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000));
//			re+=abs((y - htl.predict(samplex))/y);
//			context.leaveScope();
//		}
//		std::cout << re/nbSamples << std::endl;*/
//
//
//
//		context.leaveScope();
//
//		context.enterScope("Functions");
//		DenseDoubleVectorPtr splitsNY = htlNY->getSplits();
//		DenseDoubleVectorPtr splitsNXY = htlNXY->getSplits();
//		for(double x = 0; x < 1; x+=0.01){
//			context.enterScope(string((double) x));
//			context.resultCallback("x",(double)x);
//			DenseDoubleVectorPtr input = new DenseDoubleVector(0, 0.0);
//			DenseDoubleVectorPtr output = new DenseDoubleVector(0, 0.0);
//			input->appendValue(x);
//			//double noise = MathUtils::randDouble();
//			double noise = 0;
//			if(x > 1)
//				std::cout << x << std::endl;
//			y = getFuncValue(x)+noise; 
//			output->appendValue(y);
//			if(modelNY){
//				context.resultCallback("prediction NY",htlNY->predict(input));
//
//			}
//			if(modelNXY){
//				context.resultCallback("prediction NXY",htlNXY->predict(input));
//			}
//			for(size_t i = 0; i < splitsNY->getNumValues(); i++){
//				if(abs(splitsNY->getValue(i) - x) < 0.01){
//					context.resultCallback("splitsNY",(double)y);
//				}
//			}
//			for(size_t i = 0; i < splitsNXY->getNumValues(); i++){
//				if(abs(splitsNXY->getValue(i) - x) < 0.01){
//					context.resultCallback("splitsNXY",(double)y);
//				}
//			}
//			context.resultCallback("targetFunction",(double)y);
//			context.leaveScope();
//		}
//		htlNY->pprint();
//		std::cout << "****************************" << std::endl;
//		htlNXY->pprint();
//		context.leaveScope();
//
//		/*// test perceptron
//		context.enterScope("PerceptronTest");
//		Perceptron p = Perceptron(*dataDef, 0.75, 0.005);
//		for (int i = 0; i < nbTestSamples; i++) {
//			std::vector<double> sample = getSample();
//			p.train(sample);
//		}
//		for(double x = 0; x < 1; x+=0.01){
//			context.enterScope(string((double) x));
//			context.resultCallback("x",(double)x);
//			std::vector<double> sample;
//			sample.push_back(x);
//			y = getFuncValue(x); 
//			sample.push_back(y);
//			context.resultCallback("targetFunction",(double)y);
//			context.resultCallback("prediction",p.predict(sample));
//			context.leaveScope();
//		}
//		context.leaveScope();*/
//
//		context.leaveScope();
//
//		/*cout << "Result:\n";
//		 std::vector<double> sample;
//		 sample.push_back(MathUtils::randDouble()/2);
//		 sample.push_back(MathUtils::randDouble()/2);
//		 LeafNode* leaf = htl.traverseSample(sample);
//		 cout << "w0: " << leaf->linearModel->numericalWeights[0] << "\n";
//		 cout << "w1: " << leaf->linearModel->numericalWeights[1] << "\n";
//		 cout << "t: " << leaf->linearModel->threshold << "\n";*/
//		std::cout << "succesfull end \n";
//
//		// 2D perceptron parameter space search
//		// average over several functions
//		/*std::vector<std::vector<double> > err;
//		for(float lr = 0; lr < 3; lr+=0.05){
//			for(float d = 0; d < 1; d+=0.05){
//				htlNXY = hoeffdingTreeIncrementalLearner(context, randomSeed, NXY, Hoeffding, 0.01).staticCast<HoeffdingTreeIncrementalLearner>();
//				htlNXY->createExpression(context, doubleClass);
//				for (int i = 0; i < nbSamples; i++) {
//					DenseDoubleVectorPtr input = new DenseDoubleVector(0, 0.0);
//					DenseDoubleVectorPtr output = new DenseDoubleVector(0, 0.0);
//					getSample(input, output);
//					htlNXY->addTrainingSample(context, htlNXY->root, input, output);
//				}
//				err[lr][d] = getRMSE(context, testSet, htlNXY);
//			}
//		}
//		ofstream myfile;
//		myfile.open ("learningRate_Decay.txt");
//	  	for(float lr = 0; lr < 3; lr+=0.05){
//			for(float d = 0; d < 1; d+=0.05){
//				myfile << err[lr][d] << ",";
//			}
//			myfile << "\n";
//		}
//		 myfile.close();*/
//
//
//
//		return new Boolean(true);
//	}
//
//	std::vector<std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr> > createTestSet(int nbTestSamples){
//		std::vector<std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr> > testSamples;
//		for (int i = 0; i < nbTestSamples; i++) {
//			DenseDoubleVectorPtr input = new DenseDoubleVector(0, 0.0);
//			DenseDoubleVectorPtr output = new DenseDoubleVector(0, 0.0);
//			getSample(input, output);
//			testSamples.push_back(std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr>(input, output));
//		}
//		/*std::vector<double> sample;
//		sample.push_back(0.95);
//		sample.push_back(0.95);
//		sample.push_back(0.5);
//		sample.push_back(0.5);
//		sample.push_back(0.5);
//		double y = 10*sin(M_PI*0.95*0.95)+5+2.5-0.5;
//		sample.push_back(y);
//		testSamples.push_back(sample);*/
//		return testSamples;
//	}
//
//	void getSample(DenseDoubleVectorPtr& input, DenseDoubleVectorPtr& output){
//		/*double x1, x2, x3, x4, x5, y, noise;
//		std::vector<double> sample;
//		x1 = MathUtils::randDouble();
//		x2 = MathUtils::randDouble();
//		x3 = MathUtils::randDouble();
//		x4 = MathUtils::randDouble();
//		x5 = MathUtils::randDouble();
//		sample.push_back(x1);
//		sample.push_back(x2);
//		sample.push_back(x3);
//		sample.push_back(x4);
//		sample.push_back(x5);
//		noise = MathUtils::randDouble();
//		//noise = 0;
//		y = 10*sin(M_PI*x1*x2)+20*(x3-0.5)*(x3-0.5)+10*x4+5*x5+noise;
//		sample.push_back(y);
//		return sample;*/
//		double x1, y, noise;
//		input->clear();
//		output->clear();
//		x1 = MathUtils::randDouble();
//		input->appendValue(x1);
//		//noise = MathUtils::randDouble();
//		noise = 0;
//		y = getFuncValue(x1)+noise;
//		output->appendValue(y);
//	}
//
//	double getFuncValue(double x){
//		//return sin(M_PI*x);
//
//		//return sin(x*M_PI)+1/30*exp(x*M_PI)+0.5*sin(3*x*M_PI);
//
//		/*if(x < 0.25)
//			return x;
//		else if(x < 0.5)
//			return -x+0.5;
//		else if(x < 0.75)
//			return x-0.5;
//		else
//			return -x+1;*/
//
//		//return 3*x+1;
//
//		//return -3*x+1;
//
//		if(x < 0.25)
//			return x;
//		else
//			return -x+0.5;
//	}
//
//	/*std::vector<double> getSample1D(){
//		double x, y, noise;
//		std::vector<double> sample;
//		x = MathUtils::randDouble();
//		sample.push_back(x);
//		//noise = MathUtils::randDouble();
//		y = sin(M_PI*x);
//		sample.push_back(y);
//		return sample;
//	}*/
//
//	// normalization?
//	double getRMSE(ExecutionContext& context, std::vector<std::pair<DenseDoubleVectorPtr,DenseDoubleVectorPtr> > testSet, HoeffdingTreeIncrementalLearnerPtr htl){
//		double se = 0;
//		double diff;
//		for (int i = 0; i < nbTestSamples; i++) {
//			std::pair<DenseDoubleVectorPtr,DenseDoubleVectorPtr> sample = testSet[i];
//			/*if(i == nbTestSamples){
//				context.resultCallback("Ttargetvalue",sample.back());
//				context.resultCallback("TpredictedValue",htl.predict(sample));
//			}*/
//			diff = sample.second->getValue(0) - htl->predict(sample.first);
//			se+= diff*diff;
//		}
//		return sqrt(se/nbTestSamples);
//	}
//
//	double getRRSE(ExecutionContext& context, std::vector<std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr> > testSet, HoeffdingTreeIncrementalLearnerPtr htl){
//		double avg = 0;
//		double se1 = 0;
//		double diff;
//		for (int i = 0; i < nbTestSamples; i++) {
//			std::pair<DenseDoubleVectorPtr,DenseDoubleVectorPtr> sample = testSet[i];
//			double diff = sample.second->getValue(0) - htl->predict(sample.first);
//			avg+=sample.second->getValue(0);
//			se1+= diff*diff;
//		}
//		avg /= nbTestSamples;
//		double se2 = 0;
//		for (int i = 0; i < nbTestSamples; i++) {
//			std::pair<DenseDoubleVectorPtr,DenseDoubleVectorPtr> sample = testSet[i];
//			diff = avg - htl->predict(sample.first);
//			se2+= diff*diff;
//		}
//		return se1/se2;
//	}
//
//};
//
//}
//;
///* namespace lbcpp */
