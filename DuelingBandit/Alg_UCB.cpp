#include "stdafx.h"
#include "Alg_UCB.h"


CAlg_UCB::CAlg_UCB()
{
}


CAlg_UCB::~CAlg_UCB()
{
}

void CAlg_UCB::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	scale_factor = config.ucb_scale_factor;
	for (int i = 0; i < num_arms; i++) {
		counts.push_back(0.000001);
		cum_reward.push_back(0.00001);
	}
}

int CAlg_UCB::pull(int ind_slot, vector<double> reward) {
	vector<double> cur_ucb;
	calUCB(ind_slot, cur_ucb);
	int ind_best = 0;
	for (int ind_arm = 1; ind_arm < num_arms; ind_arm++) {
		if (cur_ucb[ind_arm] > cur_ucb[ind_best]) {
			ind_best = ind_arm;
		}
	}
	counts[ind_best]++;
	cum_reward[ind_best] += reward[ind_best];
	return ind_best;
}
void CAlg_UCB::calUCB(int ind_slot, vector<double>& ucb) {
	ucb.clear();
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		double cur_ucb = cum_reward[ind_arm] / counts[ind_arm] + sqrt(scale_factor * log(ind_slot + 1.0) / counts[ind_arm]);
		ucb.push_back(cur_ucb);
	}
}
