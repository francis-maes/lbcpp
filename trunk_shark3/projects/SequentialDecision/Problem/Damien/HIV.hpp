#ifndef HH_HIV_HH
#define HH_HIV_HH

#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <math.h>
#include "optimalControlProblem.hpp"
//#include "objectFunction.hpp"
using namespace std;

//#include <random/random>


namespace damien
{

//
class HIV : public optimalControlProblem{
private:
// Parameters for the system dynamics
  double lambda1;
  double d1;
  //double epsilon1;
  //double epsilon2;
  double k1;
  double lambda2;
  double d2;
  double f;
  double k2;
  double delta;
  double m1;
  double m2;
  double NT;
  double c;
  double rho1;
  double rho2;
  double lambdaE;
  double bE;
  double Kb;
  double dE;
  double Kd;
  double deltaE;
// Parameters for the cost function
  double Q;
  double R1;
  double R2;
  double S;
// Parameters for the cost functional
  vector<double> stateVector;
  vector<double> a;
  double Mcar;
  double g;
  size_t time;
// Old state vector
  vector<double> oldStateVector;
public:  
  HIV() {
// Parameters for the system dynamics
    lambda1=10000;
    d1=0.01;
    k1=0.0000008;
    lambda2=31.98;
    d2=0.01;
    f=0.34;
    k2=0.0001;
    delta=0.7;
    m1=0.00001;
    m2=0.00001;
    NT=100;
    c=13;
    rho1=1;
    rho2=1;
    lambdaE=1;
    bE=0.3;
    Kb=100;
    dE=0.25;
    Kd=500;
    deltaE=0.1;
// Parameters for the cost function
    Q=0.1;
    R1=20000.;
    R2=20000.;
    S=1000.;
// Parameters for the cost functional
  vector<double> stateVector;
  vector<double> a;
  //double Mcar;
  //double g;
  //size_t time;
  //
    stateVector.resize(6);
    a.resize(2);
    oldStateVector.resize(6);
  }
  ~HIV() {;}
  
  bool TerminalStateReached() {
      return false;
  }	
  double GetReward() {
    double V=oldStateVector[4];
    double E=oldStateVector[5];
    double epsilon1=a[0];
    double epsilon2=a[1];
    return -(Q*V+R1*epsilon1*epsilon1+R2*epsilon2*epsilon2-S*E);
  }
  void StateVectorDot(const vector<double>& stateVector_, double answer[]) {
    double T1Dot;
    double T2Dot;
    double T1StarDot;
    double T2StarDot;
    double VDot;
    double EDot;
    double T1=stateVector_[0];
    double T2=stateVector_[1];
    double T1Star=stateVector_[2];
    double T2Star=stateVector_[3];
    double V=stateVector_[4];
    double E=stateVector_[5];
    double epsilon1=a[0];
    double epsilon2=a[1];
    T1Dot=lambda1-d1*T1-(1-epsilon1)*k1*V*T1;
    T2Dot=lambda2-d2*T2-(1-f*epsilon1)*k2*V*T2;
    T1StarDot=(1-epsilon1)*k1*V*T1-delta*T1Star-m1*E*T1Star;
    T2StarDot=(1-f*epsilon1)*k2*V*T2-delta*T2Star-m2*E*T2Star;
    VDot=(1-epsilon2)*NT*delta*(T1Star+T2Star)-c*V-((1-epsilon1)*rho1*k1*T1+(1-f*epsilon1)*rho2*k2*T2)*V;
    EDot=lambdaE+(bE*(T1Star+T2Star)*E)/(T1Star+T2Star+Kb) - (dE*(T1Star+T2Star)*E)/(T1Star+T2Star+Kd)-deltaE*E;
    answer[0]=T1Dot;
    answer[1]=T2Dot;
    answer[2]=T1StarDot;
    answer[3]=T2StarDot;
    answer[4]=VDot;
    answer[5]=EDot;
    //    cout << "answer" << answer << endl;
    //return answer;
  }
  HIV& IncreaseStep(double h=0.001) {
    double tmp[6];
    StateVectorDot(stateVector, tmp);
    for (size_t i = 0; i < 6; ++i)
      stateVector[i] += h * tmp[i];

    //stateVector=stateVector+h*StateVectorDot(stateVector);
    return *this;
  }
  HIV& Transition() {
    for (size_t i=0;i<oldStateVector.size();i++){
      oldStateVector[i]=stateVector[i];
    }
    double h=0.001;
    for (size_t i=0;i<5000;i++) {
      IncreaseStep(h);
    }
    time=time+1;
    return *this;
  }
  HIV& TransitionDay() {
    double h=0.001;
    for (size_t i=0;i<1000;i++) {
      IncreaseStep(h);
    }
    //    cout << "stateVector=" << stateVector << endl;
    return *this;
  }
  HIV& PutAction(const vector<double>& a_) {
    a=a_;
    return *this;
  }
  vector<double> GetAction() {
    return a;
  }
  vector<double> GetState() {
    return stateVector;
  }  
  HIV& PutState(const vector<double>& stateVector_) {
    stateVector=stateVector_;
    return *this;
  }
  size_t GetTime() {
    return time;
  }
  HIV& PutTime(size_t time_) {
    time=time_;
    return *this;
  }
};

/*
vector<vector<double> > P0(){
  vector<vector<double> > answer;
  HIV HIVObject;
  vector<double> iniVector(6);
  iniVector[0]=1000000.;
  iniVector[1]=3198.;
  iniVector[2]=0.0001;
  iniVector[3]=0.0001;
  iniVector[4]=1.;
  iniVector[5]=10.;
  vector<double> action(2);
  action[0]=0.;
  action[1]=0.;
  for (size_t i=0;i<101;i++){
    HIVObject.PutState(iniVector);
    HIVObject.PutAction(action);
    for (size_t j=0;j<i;j++){
      HIVObject.TransitionDay();
    }
    answer.push_back(HIVObject.GetState());
  }
  return answer;
};*/

}; /* namespace damien */

#endif /* HH_HIV_HH */

