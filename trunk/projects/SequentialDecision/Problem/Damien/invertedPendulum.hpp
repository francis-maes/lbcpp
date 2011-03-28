#ifndef HH_INVERTEDPEDULUM_HH
#define HH_INVERTEDPEDULUM_HH

#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <math.h>
#include "optimalControlProblem.hpp"
//#include "objectFunction.hpp"
//#include "draw.hpp"
using namespace std;

//#include <random/random>

namespace damien
{

//
class invertedPendulum : public optimalControlProblem{
private:
  vector<double> stateVector;
  vector<double> a;
  double M1;
  double L1;
  double mu1;
  double g;
  size_t time;
public:  
  invertedPendulum() {
    stateVector.resize(2);
    stateVector[0]=0.;
    stateVector[1]=0.;
    M1=1.0;
    L1=1.0;
    mu1=0.01;
    g=9.81;
    a.resize(1);
    a[0]=0.;
    time=0;
  }
  ~invertedPendulum() {;}
  
  bool TerminalStateReached() {
    return false;
  }	
  double GetReward() {
    double pi=acos(-1.);
    double theta1=stateVector[0];
    bool stop=false;
    while (!stop){
      if (theta1-pi > pi)
	theta1=theta1-2*pi;
      else if (theta1-pi < -pi)
	theta1=theta1+2*pi;
      else
	stop=true;
    }
    //    double a=(pow(pow(theta1-pi,2.)+pow(stateVector[1],2.)+pow(theta2,2.)+pow(stateVector[3],2.),0.5));
    double a=(pow(pow(stateVector[0]-pi,2.)+pow(stateVector[1],2.),0.5));
    if (a < 1.)
      return (1.-a);
    else
      return 0.;
  }
  vector<double> f(vector<double> x) {
    vector<double> fx(2);
    double u=a[0];
    fx[0]=x[1];
    fx[1]=(u-mu1*x[1]-M1*g*L1*sin(x[0]))/(M1*pow(L1,2));
    return fx;
  }
  invertedPendulum& IncreaseStep(double h=0.001) {
    stateVector=stateVector+h*f(stateVector);
    return *this;
  }
  invertedPendulum& Transition() {
    double h=0.001;
    for (size_t i=0;i<100;i++) {
      IncreaseStep(h);
    }
    time=time+1;
    return *this;
  }
  invertedPendulum& PutAction(vector<double> a_) {
    a=a_;
    return *this;
  }
  vector<double> GetAction() {
    return a;
  }
  vector<double> GetState() {
    return stateVector;
  }  
  invertedPendulum& PutState(vector<double> stateVector_) {
    stateVector=stateVector_;
    return *this;
  }
  size_t GetTime() {
    return time;
  }
  invertedPendulum& PutTime(size_t time_) {
    time=time_;
    return *this;
  }
  
};

/*
void DrawFigureInvertedPendulum(string nameFigure,vector<vector<double> > stateVectors){
  double L1=1.;
  vector<vector<pair<double,double> > > polylines(stateVectors.size());
  for (size_t i=0;i<polylines.size();i++){
    vector<pair<double,double> > temp;
    double x;
    double y;
    x=(L1*(double) i);
    y=0;
    temp.push_back(make_pair(x,y));
    x=x+ L1*sin(stateVectors[i][0]);
    y=y-L1*cos(stateVectors[i][0]);
    temp.push_back(make_pair(x,y));
    polylines[i]=temp;
  }
  DrawPolylineInFig(nameFigure,polylines);
  return;
}
*/

}; /* namespace damien */

#endif /* HH_INVERTEDPEDULUM_HH */

