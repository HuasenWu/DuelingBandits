#include "stdafx.h"
#include "Alg_RUCB.h"


CAlg_RUCB::CAlg_RUCB()
{
}


CAlg_RUCB::~CAlg_RUCB()
{
}

void CAlg_RUCB::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	scale_factor = config.ucb_scale_factor;

	counts.clear();
	wins.clear();

	hyp_best.clear();
	for (int i = 0; i < num_arms; i++) {
		vector<double> cur_count(num_arms, 0.000001);
		vector<double> cur_beat(num_arms, 0.00001);
		counts.push_back(cur_count);
		wins.push_back(cur_beat);
	}
}

int CAlg_RUCB::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	vector<vector<double>> rucb;
	cmpPair.clear();
	vector<int> cand_set;
	calRUCB(ind_slot, rucb);
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		bool cand_flag = true;
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm2 != ind_arm1 && rucb[ind_arm1][ind_arm2] < 0.5) {
				cand_flag = false;
				break;
			}
		}
		if (cand_flag) {
			cand_set.push_back(ind_arm1);
		}
	}
	int ind_first = -1;
	if ((int)cand_set.size() == 0) {
		ind_first = int(double(rand()) / RAND_MAX * num_arms);
		if (ind_first == num_arms) {
			ind_first = num_arms - 1;
		}
		hyp_best.clear();
	}
	else if ((int)cand_set.size() == 1) {
		ind_first = cand_set[0];
		hyp_best.clear();
		hyp_best.push_back(ind_first);
	}
	else {
		bool flag;
		if ((int)hyp_best.size() == 0 ||
			((int)hyp_best.size() == 1 && !find(cand_set, hyp_best[0]))) {
			flag = true; // select the arms not in hyp_best
		} 
		else {
			flag = (double(rand()) / RAND_MAX) < 0.5;
		}
		if (!flag) {
			ind_first = hyp_best[0];
		}
		else {
			int ind_cand = int(double(rand()) / RAND_MAX * (int)cand_set.size());
			if (ind_cand == (int)cand_set.size()) {
				ind_cand = (int)cand_set.size() - 1;
			}
			if (!find(hyp_best, cand_set[ind_cand])) {
				ind_first = cand_set[ind_cand];
			}
			else {
				if (ind_cand >= 1) {
					ind_first = cand_set[ind_cand - 1];
				}
				else if (ind_cand <= (int)cand_set.size() - 1) {
					ind_first = cand_set[ind_cand + 1];
				}
				else {
					ind_first = cand_set[0];
				}
			}
		}		

	}


	int ind_second = 0;
	for (int ind_arm = 1; ind_arm < num_arms; ind_arm++) {
		if (rucb[ind_arm][ind_first] > rucb[ind_second][ind_first]) {
			ind_second = ind_arm;
		}
	}
	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;
	wins[ind_first][ind_second] += beat[ind_first][ind_second];
	wins[ind_second][ind_first] += (1 - beat[ind_first][ind_second]);
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	if (beat[ind_first][ind_second] == 1) {
		return ind_first;
	}
	else {
		return ind_second;
	}
}
void CAlg_RUCB::calRUCB(int ind_slot, vector<vector<double>>& ucb) {
	ucb.clear();
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		vector<double> cur_ucb(num_arms, 0.0);
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm2 != ind_arm1) {
				cur_ucb[ind_arm2] = wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2]
					+ sqrt(scale_factor * log(ind_slot + 1.0) / counts[ind_arm1][ind_arm2]);
			}
			else {
				cur_ucb[ind_arm1] = 0.5;
			}
		}
		ucb.push_back(cur_ucb);
	}
}

bool CAlg_RUCB::find(vector<int> list, int target) {
	for (int i = 0; i < (int)list.size(); i++) {
		if (list[i] == target) {
			return true;
		}
	}
	return false;
}


