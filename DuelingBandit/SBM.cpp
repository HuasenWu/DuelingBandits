#include "stdafx.h"
#include "SBM.h"



CSBM::CSBM(int num_arms, double alpha)
{
	this->num_arms = num_arms;
	this->alpha = alpha;
	counts = vector<double>(num_arms, 0.000001);
	cum_rewards = vector<double>(num_arms, 0.000005);
}


CSBM::~CSBM()
{
}

void CSBM::update(int ind_arm, double reward) {
	counts[ind_arm]++;
	cum_rewards[ind_arm] += reward;
}

int CSBM::pull(int ind_slot) {
	double max_ucb = -1.0 * DBL_MAX;
	int ind_best = -1;
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		double ucb;
		if (counts[ind_arm] < 1) {
			ucb = 1.0;
		}
		else {
			ucb = cum_rewards[ind_arm] / counts[ind_arm] + sqrt((alpha + 2.0) * log(ind_slot + 1) / 2.0 / counts[ind_arm]);
		}
		if (ucb > max_ucb) {
			max_ucb = ucb;
			ind_best = ind_arm;
		}
	}
	return ind_best;
}

