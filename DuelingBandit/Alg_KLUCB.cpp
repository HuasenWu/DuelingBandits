#include "stdafx.h"
#include "Alg_KLUCB.h"


CAlg_KLUCB::CAlg_KLUCB()
{
}


CAlg_KLUCB::~CAlg_KLUCB()
{
}

void CAlg_KLUCB::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	ctrl_factor = config.ctrl_factor_klucb;
	eps = config.eps_klucb;
	for (int i = 0; i < num_arms; i++) {
		counts.push_back(0.000001);
		cum_reward.push_back(0.00001);
	}
}

int CAlg_KLUCB::pull(int ind_slot, vector<double> reward) {
	vector<double> cur_ucb;
	calKLUCB(ind_slot, cur_ucb);
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
void CAlg_KLUCB::calKLUCB(int ind_slot, vector<double>& ucb) {
	ucb.clear();
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		ucb.push_back(0.0);
	}
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		double rhs = log((ind_slot + 1.0) / counts[ind_arm]) + ctrl_factor * log(log(ind_slot + 1.0 + eps));
		rhs /= counts[ind_arm];
		double prob = cum_reward[ind_arm] / counts[ind_arm];
		if (prob >= 1.0) {
			ucb[ind_arm] = 1.0;
		}
		else if (prob <= 0.0) {
			ucb[ind_arm] = 1.0 - exp(-1.0 * rhs);
		}
		else {
			rhs -= (prob * log(prob) + (1 - prob) * log(1 - prob));
			double start = prob;
			double end = 1 - eps;
			if (-1.0 *  prob * log(end) - (1.0 - prob) * log(1 - end) < rhs) {
				ucb[ind_arm] = end;
			}
			else {
				while ((end - start) > eps) {
					double mid = (start + end) / 2.0;
					if (-1.0 * prob * log(mid) - (1.0 - prob) * log(1 - mid) < rhs) {
						start = mid;
					}
					else {
						end = mid;
					}
				}
				ucb[ind_arm] = (start + end) / 2.0;
			}
		}
	}
}

