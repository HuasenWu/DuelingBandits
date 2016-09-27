#pragma once
#include "Alg.h"
#include "BetaDist.h"
class CAlg_TS :
	public CAlg
{
public:
	CAlg_TS();
	~CAlg_TS();
private:
	vector<double> alpha;    // alpha-parameter for each arm
	vector<double> beta;     // beta-parameter for each arm
public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward); // select the estimated best arm
};

