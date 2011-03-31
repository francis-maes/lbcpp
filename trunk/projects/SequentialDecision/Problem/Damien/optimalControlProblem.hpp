#ifndef HH_OPTIMALCONTROLPROBLEM_HH
#define HH_OPTIMALCONTROLPROBLEM_HH

#include <vector>
#include <utility>
#include <stdexcept>
#include <algorithm>
//#include <random/random>
//#include "objectFunction.hpp"
//#include "supervisedLearning.hpp"
//#include "sequence.hpp"

namespace damien
{

using std::vector;

class optimalControlProblem : public lbcpp::Object {
public:
  virtual ~optimalControlProblem() {}
  virtual bool TerminalStateReached() =0;
  virtual double GetReward() =0;
  virtual optimalControlProblem& Transition() =0;
  virtual optimalControlProblem& PutAction(const vector<double>& )  =0;
  virtual vector<double> GetAction() = 0;
  virtual optimalControlProblem& PutState(const vector<double>& ) =0;
  virtual vector<double> GetState() = 0;
  virtual optimalControlProblem& PutTime(size_t) =0;
  virtual size_t GetTime() = 0;
};

typedef ReferenceCountedObjectPtr<optimalControlProblem> optimalControlProblemPtr;

template<class V> inline vector<V> operator*(V c,const std::vector<V>& a) {
  vector<V> answer(a.size());
  for (size_t i=0;i<a.size();i++){
    answer[i]=c*a[i];
  }
  return answer;
}
template<class V> inline vector<V> operator + (const std::vector<V>& a, const std::vector<V>& b) {
  if (a.size() != b.size())
    throw std::runtime_error("Unable to compute the sum of two vectors of different size");
  vector<V> sum(a.size());
  for (size_t i=0; i < a.size(); i++) {
    sum[i]=a[i]+b[i];
  };
  return sum;
};

}; /* namespace damien */

#if 0
class mapping{
public:
  virtual vector<double> operator() (const vector<double>& state) = 0;
};

// *** Computation of VN and of QN when dealing with an optimal control problem whose 
// 1] system is deterministic
// 2] reward function can only be equal to -R, R or 0. Moreover when the reward is not equal to 0 a terminal state has to be reached.
//
double computeVNIterativelyParticularized(vector<double> initialState,optimalControlProblem& optimalControlProblemObject,vector<vector<double> > actions,double gamma,double R,size_t N) {
  if (N==0)
    return 0.;
  vector<vector<double> > x(append(initialState));
  size_t t=0;
  while (true) {
    vector<vector<double> > newX;
    for (size_t i=0;i<x.size();i++){
      for (size_t j=0;j<actions.size();j++){
	optimalControlProblemObject.PutState(x[i]);
	optimalControlProblemObject.PutAction(actions[j]);
	optimalControlProblemObject.Transition();
	newX.push_back(optimalControlProblemObject.GetState());
        if (optimalControlProblemObject.GetReward() == R){
	  return pow(gamma,(double)t)*R;
	}
      }
    }
    x.resize(0);
    for (size_t i=0;i<newX.size();i++){
      optimalControlProblemObject.PutState(newX[i]);
      if (!optimalControlProblemObject.TerminalStateReached())
	x.push_back(newX[i]);
    }
    if (x.size() ==0) {
      return -pow(gamma,(double) t)*R;
    }
    if (t+1==N) {
      return 0.;
    }
    t=t+1;
    cout << "t=" << t << endl;
    cout << "x.size()=" << x.size() << endl;
  }
}

vector<double>  computeQNIterativelyParticularized(vector<double> initialState,optimalControlProblem& optimalControlProblemObject,vector<vector<double> > actions,double gamma,double R,size_t N) {
  vector<double> Q(actions.size(),0.);
  if (N==0){
    return Q;
  }
  for (size_t i=0;i<actions.size();i++){
    optimalControlProblemObject.PutState(initialState);
    optimalControlProblemObject.PutAction(actions[i]);
    optimalControlProblemObject.Transition();
    if (optimalControlProblemObject.GetReward()==0.){
      double temp(computeVNIterativelyParticularized(optimalControlProblemObject.GetState(),optimalControlProblemObject,actions,gamma,R,N-1));
      Q[i]=gamma*temp;
    }
    else {
      Q[i]=optimalControlProblemObject.GetReward();
    }
  }
  return Q;
}



double ComputeScoreOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,model<double>* modelObject,vector<vector<double> > actions,double gamma,size_t N,vector<vector<double> > initialStates) {
  ofstream fileScoreSimulation("scoreSimulation");
  ofstream fileScore("score");
  ofstream fileTerminal("terminal");
  fileScoreSimulation << "curves;" << endl;
  fileScoreSimulation << "\"localEstimator\"" << endl;
  fileScore << "curves;" << endl;
  fileScore << "\"estimator\"" << endl;
  double score=0.;
  for (size_t i=0;i<initialStates.size();i++) {
      double scoreSimulation=0.;
      double decay=1.;
      bool stop=false;
      double reward=0;
      optimalControlProblemObject.PutState(initialStates[i]);
      optimalControlProblemObject.PutTime(0);
      optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
      while (!stop) {
	  optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	  if (optimalControlProblemObject.TerminalStateReached()) {
	      scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	      fileTerminal << initialStates[i] << " " << optimalControlProblemObject.GetTime() << endl;
	      stop=true;
	  }
	  else {
	      optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
	      scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	  }
//  Decrease factor
	  decay=decay*gamma;
	  if (optimalControlProblemObject.GetTime() > N)
	      stop=true;
      }
      score=score+(scoreSimulation-score)/((double)i+1);
      fileScoreSimulation << i << " " << scoreSimulation << endl;
      fileScore << i << " " << score << endl;
  }
  cout << "End score computation" << endl;
  return score;
};

double ComputeReturnFromStateOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,model<double>* modelObject,vector<vector<double> > actions,double gamma,size_t N,vector<double> state) {
    double scoreSimulation=0.;
    double decay=1.;
    bool stop=false;
    double reward=0;
    optimalControlProblemObject.PutState(state);
    optimalControlProblemObject.PutTime(0);
    optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
    while (!stop) {
	optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	if (optimalControlProblemObject.TerminalStateReached()) {
	    scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	    stop=true;
	}
	else {
	    optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
	    scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	}
//  Decrease factor
	decay=decay*gamma;
	if (optimalControlProblemObject.GetTime() > N)
	    stop=true;
    }
  return scoreSimulation;
};


double ComputeBellmanError(optimalControlProblem& optimalControlProblemObject,double gamma,vector<model<double>*> modelObject,vector<vector<double> > actions,vector<vector<double> > initialStates,size_t noiseKiller = 1) { 
    double bellmanError=0.;
    for (size_t i=0;i<initialStates.size();i++) {
 	for (size_t j=0;j<actions.size();j++){
 	    double leftSize=modelObject[j]->ModelPrediction(initialStates[i]);
 	    double rightSize=0.;
 	    for (size_t k=0;k<noiseKiller;k++){
 		optimalControlProblemObject.PutTime(0);
 		optimalControlProblemObject.PutState(initialStates[i]);
 		optimalControlProblemObject.PutAction(actions[j]);
 		optimalControlProblemObject.Transition();
 		if (!optimalControlProblemObject.TerminalStateReached()) {
 		    rightSize=rightSize+optimalControlProblemObject.GetReward()+gamma*ComputeV(optimalControlProblemObject.GetState(),modelObject,actions);
 		} else {
 		    rightSize=rightSize+optimalControlProblemObject.GetReward();
 		}
 	    }
 	    bellmanError=bellmanError+abs((leftSize-rightSize)/((double) noiseKiller));
 	}
    }
    return (bellmanError/((double) initialStates.size()*actions.size()));
};


void TerminalStateReached(string outputFile,optimalControlProblem& optimalControlProblemObject,model<double>* modelObject,vector<vector<double> > actions,size_t N,vector<vector<double> > initialStates) {
  ofstream fileTerminal(outputFile.c_str());
  for (size_t i=0;i<initialStates.size();i++) {
      bool stop=false;
      optimalControlProblemObject.PutState(initialStates[i]);
      optimalControlProblemObject.PutTime(0);
      optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
      while (!stop) {
	  optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	  if (optimalControlProblemObject.TerminalStateReached()) {
	      fileTerminal << initialStates[i] << " " << optimalControlProblemObject.GetTime() << endl;
	      stop=true;
	  }
	  if (optimalControlProblemObject.GetTime() >= N)
	      stop=true;
      }
  }
  return;
};

void TerminalStateReached(string outputFile,optimalControlProblem& optimalControlProblemObject,vector<double>  action,size_t N,vector<vector<double> > initialStates) {
  ofstream fileTerminal(outputFile.c_str());
  for (size_t i=0;i<initialStates.size();i++) {
      bool stop=false;
      optimalControlProblemObject.PutState(initialStates[i]);
      optimalControlProblemObject.PutTime(0);
      optimalControlProblemObject.PutAction(action);
      while (!stop) {
	  optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	  if (optimalControlProblemObject.TerminalStateReached()) {
	      fileTerminal << initialStates[i] << " " << optimalControlProblemObject.GetTime() << endl;
	      stop=true;
	  }
	  if (optimalControlProblemObject.GetTime() >= N)
	      stop=true;
      }
  }
  return;
};

double ComputeScoreOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,vector<model<double>*> modelObject,vector<vector<double> > actions,double gamma,size_t N,vector<vector<double> > initialStates) {
  ofstream fileScoreSimulation("scoreSimulation");
  ofstream fileScore("score");
  ofstream fileTerminal("terminal");
  fileScoreSimulation << "curves;" << endl;
  fileScoreSimulation << "\"localEstimator\"" << endl;
  fileScore << "curves;" << endl;
  fileScore << "\"estimator\"" << endl;
  double score=0.;
  for (size_t i=0;i<initialStates.size();i++) {
      double scoreSimulation=0.;
      double decay=1.;
      bool stop=false;
      double reward=0;
      optimalControlProblemObject.PutState(initialStates[i]);
      optimalControlProblemObject.PutTime(0);
      optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
      while (!stop) {
	  optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	  if (optimalControlProblemObject.TerminalStateReached()) {
	      scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	      fileTerminal << initialStates[i] << " " << optimalControlProblemObject.GetTime() << endl;
	      stop=true;
	  }
	  else {
	      optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
	      scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	  }
//  Decrease factor
	  decay=decay*gamma;
	  if (optimalControlProblemObject.GetTime() > N)
	      stop=true;
      }
      score=score+(scoreSimulation-score)/((double)i+1);
      fileScoreSimulation << i << " " << scoreSimulation << endl;
      fileScore << i << " " << score << endl;
  }
  cout << "End score computation" << endl;
  return score;
};

double ComputeScoreOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,vector<model<double>*> modelObject,vector<vector<double> > actions,double gamma,size_t N,vector<vector<double> > initialStates,mapping& mappingObject) {
  ofstream fileScoreSimulation("scoreSimulation");
  ofstream fileScore("score");
  ofstream fileTerminal("terminal");
  fileScoreSimulation << "curves;" << endl;
  fileScoreSimulation << "\"localEstimator\"" << endl;
  fileScore << "curves;" << endl;
  fileScore << "\"estimator\"" << endl;
  double score=0.;
  for (size_t i=0;i<initialStates.size();i++) {
      double scoreSimulation=0.;
      double decay=1.;
      bool stop=false;
      double reward=0;
      optimalControlProblemObject.PutState(initialStates[i]);
      optimalControlProblemObject.PutTime(0);
      optimalControlProblemObject.PutAction(GreedyAction(mappingObject(optimalControlProblemObject.GetState()),modelObject,actions));
      while (!stop) {
	  optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	  if (optimalControlProblemObject.TerminalStateReached()) {
	      scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	      fileTerminal << initialStates[i] << " " << optimalControlProblemObject.GetTime() << endl;
	      stop=true;
	  }
	  else {
	      optimalControlProblemObject.PutAction(GreedyAction(mappingObject(optimalControlProblemObject.GetState()),modelObject,actions));
	      scoreSimulation=scoreSimulation+decay*(optimalControlProblemObject.GetReward());
	  }
//  Decrease factor
	  decay=decay*gamma;
	  if (optimalControlProblemObject.GetTime() > N)
	      stop=true;
      }
      score=score+(scoreSimulation-score)/((double)i+1);
      fileScoreSimulation << i << " " << scoreSimulation << endl;
      fileScore << i << " " << score << endl;
  }
  cout << "End score computation" << endl;
  return score;
};

double ComputeScoreFiniteTimeOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,vector<model<double>*> modelObject,vector<vector<double> > actions,size_t optimizationHorizon,vector<vector<double> > initialStates) {
  ofstream fileScoreSimulation("scoreSimulation");
  fileScoreSimulation << "curves;" << endl;
  fileScoreSimulation << "\"localEstimator\"" << endl;
  double score=0.;
  for (size_t i=0;i<initialStates.size();i++) {
    cout << "i=" << i << endl;
      double scoreSimulation=0.;
      bool stop=false;
      double reward=0.;
      optimalControlProblemObject.PutState(initialStates[i]);
      optimalControlProblemObject.PutTime(0);
      optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject[optimizationHorizon-1-optimalControlProblemObject.GetTime()],actions));
      while (!stop) {
	  optimalControlProblemObject.Transition();
//  Reinforcement algorithm part
	  if (optimalControlProblemObject.TerminalStateReached()) {
	      scoreSimulation=scoreSimulation+optimalControlProblemObject.GetReward();
	      stop=true;
	  }
	  else {
	      scoreSimulation=scoreSimulation+optimalControlProblemObject.GetReward();
	  }
//  Decrease factor
	  if (optimalControlProblemObject.GetTime() == optimizationHorizon)
	    stop=true;
	  if (!stop){
	    optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject[optimizationHorizon-1-optimalControlProblemObject.GetTime()],actions));
	  }
      }
      score=score+scoreSimulation;
      fileScoreSimulation << i << " " << scoreSimulation << endl;
  }
  cout << "End score computation" << endl;
  return score/((double) optimizationHorizon);
};

// Return de trajectory (state and action as a function of time)
vector<pair<vector<double>,vector<double> > > TrajectoryFiniteTimeOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,vector<model<double>*> modelObject,vector<vector<double> > actions,size_t optimizationHorizon,vector<double> initialState) {
  vector<pair<vector<double>,vector<double> > > trajectory;
  bool stop=false;
  optimalControlProblemObject.PutState(initialState);
  optimalControlProblemObject.PutTime(0);
  optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject[optimizationHorizon-1-optimalControlProblemObject.GetTime()],actions));
  trajectory.push_back(make_pair(optimalControlProblemObject.GetState(),optimalControlProblemObject.GetAction()));
  while (!stop) {
    optimalControlProblemObject.Transition();
    optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject[optimizationHorizon-1-optimalControlProblemObject.GetTime()],actions));
    trajectory.push_back(make_pair(optimalControlProblemObject.GetState(),optimalControlProblemObject.GetAction()));
    if (optimalControlProblemObject.GetTime() == (optimizationHorizon-1)) {
      stop=true;
    }
  }
  return trajectory;
};

vector<pair<vector<double>,vector<double> > > TrajectoryFiniteTimeOptimalControlProblem(optimalControlProblem& optimalControlProblemObject,model<double>* modelObject,vector<vector<double> > actions,size_t optimizationHorizon,vector<double> initialState) {
  vector<pair<vector<double>,vector<double> > > trajectory;
  bool stop=false;
  optimalControlProblemObject.PutState(initialState);
  optimalControlProblemObject.PutTime(0);
  optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
  trajectory.push_back(make_pair(optimalControlProblemObject.GetState(),optimalControlProblemObject.GetAction()));
  while (!stop) {
    optimalControlProblemObject.Transition();
    optimalControlProblemObject.PutAction(GreedyAction(optimalControlProblemObject.GetState(),modelObject,actions));
    trajectory.push_back(make_pair(optimalControlProblemObject.GetState(),optimalControlProblemObject.GetAction()));
    if (optimalControlProblemObject.GetTime() == (optimizationHorizon-1)) {
      stop=true;
    }
  }
  return trajectory;
};
//
// Gener four-tuples with Epsilon Greedy policy.
//
vector<FourTuple*> GenerFourTuplesEpsilonGreedy(optimalControlProblem& optimalControlProblemObject,model<double>* modelObject,vector<vector<double> > actions,size_t nbFourTuple,vector<double> initialState,double epsilon) {
  vector<FourTuple*> experience;
  bool stop=false;
  optimalControlProblemObject.PutState(initialState);
  optimalControlProblemObject.PutTime(0);
  optimalControlProblemObject.PutAction(EpsilonGreedy(optimalControlProblemObject.GetState(),modelObject,actions,epsilon));  
  while (!stop) {
    vector<double> state(optimalControlProblemObject.GetState());
    vector<double> action(optimalControlProblemObject.GetAction());
    optimalControlProblemObject.Transition();
    double reward=optimalControlProblemObject.GetReward();
    if (optimalControlProblemObject.TerminalStateReached()){
      vector<double> nextState;
      experience.push_back(new FourTuple(state,action,reward,nextState));
      stop=true;
    } else {
      vector<double> nextState(optimalControlProblemObject.GetState());
      experience.push_back(new FourTuple(state,action,reward,nextState));
    }
    optimalControlProblemObject.PutAction(EpsilonGreedy(optimalControlProblemObject.GetState(),modelObject,actions,epsilon));
    if (optimalControlProblemObject.GetTime() == (nbFourTuple)) {
      stop=true;
    }
  }
  return experience;
};
#endif // 0

#endif /* HH_OPTIMALCONTROLPROBLEM_HH */
