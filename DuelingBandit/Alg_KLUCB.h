#pragma once
#include "Alg.h"
class CAlg_KLUCB :
	public CAlg
{
public:
	CAlg_KLUCB();
	~CAlg_KLUCB();
private:
	double ctrl_factor;
	double eps;
	vector<double> counts;
	vector<double> cum_reward;
public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward); // select the estimated best arm
private:
	void calKLUCB(int ind_slot, vector<double>& ucb);
};

