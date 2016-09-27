#pragma once
#include "Alg.h"
class CAlg_UCB :
	public CAlg
{
public:
	CAlg_UCB();
	~CAlg_UCB();
private:
	double scale_factor;
	vector<double> counts;
	vector<double> cum_reward; 
public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward); // select the estimated best arm
private:
	void calUCB(int ind_slot, vector<double>& ucb);
};

