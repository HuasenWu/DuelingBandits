#include "stdafx.h"
#include "Alg_TS.h"


CAlg_TS::CAlg_TS()
{
}


CAlg_TS::~CAlg_TS()
{
}

void CAlg_TS::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	for (int i = 0; i < num_arms; i++) {
		alpha.push_back(1.0);
		beta.push_back(1.0);
	}

	
}
int CAlg_TS::pull(int ind_slot, vector<double> reward) {
	// beta distribution generator
	std::random_device rd;
	double max_sample = -1.0;
	int ind_best;
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		std::mt19937 gen(rd());
		sftrabbit::beta_distribution<> beta(alpha[ind_arm], beta[ind_arm]);
		double sample = beta(gen);
		if (sample > max_sample) {
			max_sample = sample;
			ind_best = ind_arm;
		}
	}
	alpha[ind_best] += reward[ind_best];
	beta[ind_best] += (1 - reward[ind_best]);
	return ind_best;
}
