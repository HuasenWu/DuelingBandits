#include "stdafx.h"
#include "Alg_CCB.h"


CAlg_CCB::CAlg_CCB()
{
}


CAlg_CCB::~CAlg_CCB()
{
}


void CAlg_CCB::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	scale_factor = config.ucb_scale_factor;

	counts.clear();
	wins.clear();
	potential_best.clear();
	potential_challengers.clear();
	upper_Cpld.clear();
	lower_Cpld.clear();

	for (int i = 0; i < num_arms; i++) {
		vector<int> cur_challengers;
		counts.push_back(vector<double>(num_arms, 0.000001));
		wins.push_back(vector<double>(num_arms, 0.00001));
		potential_best.push_back(i);
		potential_challengers.push_back(cur_challengers);
		upper_Cpld.push_back(num_arms);
		lower_Cpld.push_back(0);
	}
	max_loss = num_arms;
}
void CAlg_CCB::resetHypotheses() {
	potential_best.clear();
	potential_challengers.clear();
	for (int i = 0; i < num_arms; i++) {
		vector<int> cur_challengers;
		potential_best.push_back(i);
		potential_challengers.push_back(cur_challengers);
	}
	max_loss = num_arms;
}

bool CAlg_CCB::checkHypotheses(vector<vector<double>> rlcb) {
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind = 0; ind < (int)potential_challengers[ind_arm1].size(); ind++) {
			int ind_arm2 = potential_challengers[ind_arm1][ind];
			if (ind_arm2 != ind_arm1 && rlcb[ind_arm1][ind_arm2] > 0.5) {
				return false;
			}
		}
	}
	return true;
}

int CAlg_CCB::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	auto engine = std::default_random_engine{};  //random engine
	// Calculate RUCB/RLCB
	vector<vector<double>> rucb, rlcb;
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		vector<double> cur_rucb, cur_rlcb;
		calRCB(ind_slot, ind_arm, cur_rucb, cur_rlcb);
		rucb.push_back(cur_rucb);
		rlcb.push_back(cur_rlcb);
	}

	// Calculate the Upper/Lower bounds for the Copeland score
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		upper_Cpld[ind_arm1] = 0;
		lower_Cpld[ind_arm1] = 0;
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm2 != ind_arm1 && rucb[ind_arm1][ind_arm2] >= 0.5) {
				upper_Cpld[ind_arm1]++;
			}
			if (ind_arm2 != ind_arm1 && rlcb[ind_arm1][ind_arm2] >= 0.5) {
				lower_Cpld[ind_arm1]++;
			}
		}
	}

	// Find the set with largest score
	vector<int> candidates = argmaxAll(upper_Cpld);

	// Update the hypotheses set
	// Reset disproven hypotheses set
	if (!checkHypotheses(rlcb)) {
		resetHypotheses();
	}
	// Remove non-Copeland winners
	int max_lower_Cpld = 0;
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		if (lower_Cpld[ind_arm] > max_lower_Cpld) {
			max_lower_Cpld = lower_Cpld[ind_arm];
		}
	}
	for (int ind = 0; ind < (int)potential_best.size(); ind++) {
		int ind_arm = potential_best[ind];
		if (upper_Cpld[ind_arm] < max_lower_Cpld) {
			potential_best.erase(potential_best.begin() + ind);
			if ((int)potential_challengers[ind_arm].size() != max_loss + 1) {
				potential_challengers[ind_arm].clear();
				for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
					if (rucb[ind_arm][ind_arm2] < 0.5) {
						potential_challengers[ind_arm].push_back(ind_arm2);
					}
				}
			}
			ind--;
		}
	}
	if (potential_best.empty()) {
		resetHypotheses();
	}
	// Add Copeland winners
	for (int ind = 0; ind < (int)candidates.size(); ind++) {
		int ind_arm = candidates[ind];
		if (upper_Cpld[ind_arm] == lower_Cpld[ind_arm]) {
			if (!find(potential_best, ind_arm)) {
				potential_best.push_back(ind_arm);
			}
			potential_challengers[ind_arm].clear();
			max_loss = num_arms - 1 - upper_Cpld[ind_arm];
			for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
				if (ind_arm2 == ind_arm) {
					continue;
				}
				if ((int)potential_challengers[ind_arm2].size() < max_loss + 1) {
					potential_challengers[ind_arm2].clear();
				}
				else if ((int)potential_challengers[ind_arm2].size() > max_loss + 1) {
					std::shuffle(std::begin(potential_challengers[ind_arm2]), std::end(potential_challengers[ind_arm2]), engine);
					while ((int)potential_challengers[ind_arm2].size() > max_loss + 1) {
						potential_challengers[ind_arm2].erase(potential_challengers[ind_arm2].begin());
					}
				}
			}
		}
	}

	// Select the arms
	int ind_first, ind_second;
	vector<int> explore_set;
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind2 = 0; ind2 < (int)potential_challengers[ind_arm1].size(); ind2++) {
			int ind_arm2 = potential_challengers[ind_arm1][ind2];
			if (rlcb[ind_arm1][ind_arm2] <= 0.5 && rucb[ind_arm1][ind_arm2] >= 0.5) {
				explore_set.push_back(ind_arm1 * num_arms + ind_arm2);
			}
		}
	}
	if ((double)rand() / RAND_MAX <= 0.25 && !explore_set.empty()) {
		int ind = explore_set[rand() % (int)explore_set.size()];
		ind_second = ind % num_arms;
		ind_first = ind / num_arms;
	}
	else {
		// Choose the best arm
		if (isOverlapped(potential_best, candidates)) {
			if ((double)rand() / RAND_MAX <= 2.0 / 3.0) {
				candidates = getIntersection(potential_best, candidates);
			}
		}
		ind_first = candidates[rand() % (int)candidates.size()];

		// Choose the challenger
		double max_ucb = -1.0;
		for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
			if (rlcb[ind_arm][ind_first] <= 0.5 && rucb[ind_arm][ind_first] > max_ucb) {
				max_ucb = rucb[ind_arm][ind_first];
				ind_second = ind_arm;
			}
		}
		if ((double)rand() / RAND_MAX <= 0.5) {
			max_ucb = -1.0;			
			for (int ind = 0; ind < (int)potential_challengers[ind_first].size(); ind++) {
				int ind_arm = potential_challengers[ind_first][ind];
				if (rlcb[ind_arm][ind_first] <= 0.5 && rucb[ind_arm][ind_first] > max_ucb) {
					max_ucb = rucb[ind_arm][ind_first];
					ind_second = ind_arm;
				}
			}
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
void CAlg_CCB::calRCB(int ind_slot, int ind_arm, vector<double>& ucb, vector<double>& lcb) {
	ucb.clear();
	lcb.clear();
	for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
		double cur_ucb = 0.0;
		double cur_lcb = 0.0;
		if (ind_arm2 != ind_arm) {
			cur_ucb = wins[ind_arm][ind_arm2] / counts[ind_arm][ind_arm2]
				+ sqrt(scale_factor * log(ind_slot + 1.0) / counts[ind_arm][ind_arm2]);  // Relative Upper Confidence Bound (RUCB)
			cur_lcb = wins[ind_arm][ind_arm2] / counts[ind_arm][ind_arm2]
				- sqrt(scale_factor * log(ind_slot + 1.0) / counts[ind_arm][ind_arm2]);  // Relative Lower Confidence Bound (RLCB)
		}
		else {
			cur_ucb = 0.5;
			cur_lcb = 0.5;
		}
		ucb.push_back(cur_ucb);
		lcb.push_back(cur_lcb);
	}
}

bool CAlg_CCB::isOverlapped(vector<int> vec1, vector<int> vec2) {
	if (vec1.empty() || vec2.empty()) {
		return false;
	}
	for (int ind = 0; ind < (int)vec1.size(); ind++) {
		if (find(vec2, vec1[ind])) {
			return true;
		}
	}
	return false;
}

vector<int> CAlg_CCB::getIntersection(vector<int> vec1, vector<int> vec2) {
	vector<int> vec3;
	sort(vec1.begin(), vec1.end());
	sort(vec2.begin(), vec2.end());
	set_intersection(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), back_inserter(vec3));
	return vec3;
}

