#include "stdafx.h"
#include "Alg_SAVAGE.h"


CAlg_SAVAGE::CAlg_SAVAGE()
{
}


CAlg_SAVAGE::~CAlg_SAVAGE()
{
}

void CAlg_SAVAGE::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	winner_type = config.winner_type;
	ind_best = -1;

	counts.clear();
	wins.clear();
	rucb.clear();
	rlcb.clear();
	upper_cpld.clear();
	lower_cpld.clear();
	explore_set.clear();

	for (int i = 0; i < num_arms; i++) {
		wins.push_back(vector<double>(num_arms, 0.0000005));
		counts.push_back(vector<double>(num_arms, 0.000001));		

		rucb.push_back(vector<double>(num_arms, 1.0));
		if (winner_type == WINNER_TYPE_NON_CONDORCET) {
			rlcb.push_back(vector<double>(num_arms, 0.0));
		}

		upper_cpld.push_back(num_arms - 1);
		lower_cpld.push_back(0);

		for (int j = 0; j < i; j++) {
			explore_set.push_back(i * num_arms + j);
		}
	}
}

int CAlg_SAVAGE::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	cmpPair.clear();

	int ind_first;
	int ind_second;

	if (explore_set.empty()) {
		if (ind_best < 0) {
			cout << "Alg_SAVAGE.cpp: fail to identify ind_best!" << endl;
			exit(ERROR_INVALID_INDEX);
		}
		ind_first = ind_best;
		ind_second = ind_best;
	}
	else {
		// choose the one pair that is least compared
		int ind_pair = argminGivenIndSet(counts, explore_set);
		ind_second = ind_pair % num_arms;
		ind_first = ind_pair / num_arms;
	}

	// update 
	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;	
	wins[ind_first][ind_second] += beat[ind_first][ind_second];
	wins[ind_second][ind_first] += (1 - beat[ind_first][ind_second]);	

	// update RCB
	updateRCB(ind_first, ind_second);

	// Independent test
	independentTest();

	// output
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);

	if (beat[ind_first][ind_second] == 1) {
		return ind_first;
	}
	else {
		return ind_second;
	}
}

int CAlg_SAVAGE::argminGivenIndSet(vector<vector<double>> val, vector<int> ind_set) {
	if (ind_set.empty() || val.empty()) {
		return -1;
	}
	//for (int i = 0; i < num_arms; i++) {
	//	for (int j = 0; j < num_arms; j++) {
	//		cout << counts[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	int min_ind = ind_set[0];
	double min_val = val[ind_set[0] / num_arms][ind_set[0] % num_arms];
	for (int ind = 1; ind < (int)ind_set.size(); ind++) {
		double cur_val = val[ind_set[ind] / num_arms][ind_set[ind] % num_arms];
		if (cur_val < min_val) {
			min_val = cur_val;
			min_ind = ind_set[ind];
		}
	}
	return min_ind;
}

// do not need to calculate the RUCB/RLCB for all pairs, useless here
void CAlg_SAVAGE::calRCB() {
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 <= ind_arm1; ind_arm2++) {
			if (ind_arm2 != ind_arm1) {
				if (counts[ind_arm1][ind_arm2] < 1) {
					rucb[ind_arm1][ind_arm2] = 1.0;
				} 
				else {					
					double interv_width = sqrt((log((double)num_arms * (num_arms - 1)) + 2 * log((double)num_slots)) / (2.0 * counts[ind_arm1][ind_arm2])); // if directly calculated as 2 * num_arms * num_slots * num_slots, it may overflow
					rucb[ind_arm1][ind_arm2] = wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2] + interv_width;
					rucb[ind_arm2][ind_arm1] = wins[ind_arm2][ind_arm1] / counts[ind_arm2][ind_arm1] + interv_width;
				}
			}
			else {
				rucb[ind_arm1][ind_arm2] = 0.5;
			}
		}
	}
	if (winner_type == WINNER_TYPE_NON_CONDORCET) {
		for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
			for (int ind_arm2 = 0; ind_arm2 <= ind_arm1; ind_arm2++) {
				rlcb[ind_arm1][ind_arm2] = 1.0 - rucb[ind_arm2][ind_arm1];
				rlcb[ind_arm2][ind_arm1] = 1.0 - rucb[ind_arm1][ind_arm2];
			}
		}
	}
}

void CAlg_SAVAGE::updateRCB(int ind_first, int ind_second) {
	if (ind_first == ind_second) {
		rucb[ind_first][ind_second] = 0.5;
		if (winner_type == WINNER_TYPE_NON_CONDORCET) {
			rlcb[ind_first][ind_second] = 0.5;
		}		
	}
	else {
		double interv_width = sqrt((log((double)num_arms * (num_arms - 1)) + 2 * log((double)num_slots)) / (2.0 * counts[ind_first][ind_second])); // if directly calculated as 2 * num_arms * num_slots * num_slots, it may overflow
		rucb[ind_first][ind_second] = wins[ind_first][ind_second] / counts[ind_first][ind_second] + interv_width;
		rucb[ind_second][ind_first] = wins[ind_second][ind_first] / counts[ind_second][ind_first] + interv_width;
		if (winner_type == WINNER_TYPE_NON_CONDORCET) {
			rlcb[ind_first][ind_second] = 1.0 - rucb[ind_second][ind_first];
			rlcb[ind_second][ind_first] = 1.0 - rucb[ind_first][ind_second];
		}
	}
}

void CAlg_SAVAGE::independentTest() {
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		upper_cpld[ind_arm1] = 0;
		if (winner_type == WINNER_TYPE_NON_CONDORCET) {
			lower_cpld[ind_arm1] = 0;
		}		
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm2 != ind_arm1) {
				if (rucb[ind_arm1][ind_arm2] > 0.5) {
					upper_cpld[ind_arm1]++;
				}
				if (winner_type == WINNER_TYPE_NON_CONDORCET && rlcb[ind_arm1][ind_arm2] > 0.5) {
					lower_cpld[ind_arm1]++;
				}
			}
		}
	}

	// Condorcet winner
	if (winner_type == WINNER_TYPE_CONDORCET) {		
		// check the independent conditions
		for (int ind = 0; ind < (int)explore_set.size();) {
			int ind_pair = explore_set[ind];
			int ind_second = ind_pair % num_arms;
			int ind_first = ind_pair / num_arms;
			if (upper_cpld[ind_first] < num_arms - 1 || upper_cpld[ind_second] < num_arms - 1) {
				explore_set.erase(explore_set.begin() + ind);
			}
			else {
				ind++;
			}
		}
	}
	// non-Condorcet winners
	else if (winner_type == WINNER_TYPE_NON_CONDORCET) {
		int max_lower_cpld = -1;
		for (int i = 0; i < num_arms; i++) {
			if (lower_cpld[i] > max_lower_cpld) {
				max_lower_cpld = lower_cpld[i];
			}
		}

		// check the independent conditions
		for (int ind = 0; ind < (int)explore_set.size();) {
			int ind_pair = explore_set[ind];
			int ind_second = ind_pair % num_arms;
			int ind_first = ind_pair / num_arms;
			if (rlcb[ind_first][ind_second] > 0.5 || rucb[ind_first][ind_second] < 0.5 ||
				(max_lower_cpld > upper_cpld[ind_first] && max_lower_cpld > upper_cpld[ind_second])) {
				explore_set.erase(explore_set.begin() + ind);
			}
			else {
				ind++;
			}
		}

	}
	else {
		cout << "Alg_SAVAGE.cpp: Invalid winner type!" << endl;
		exit(ERROR_WINNER_TYPE);
	}
	// if all arms have been explored 
	if (explore_set.empty()) {
		ind_best = argmax(upper_cpld);
	}
}

