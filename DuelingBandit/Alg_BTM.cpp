#include "stdafx.h"
#include "Alg_BTM.h"


CAlg_BTM::CAlg_BTM()
{
}


CAlg_BTM::~CAlg_BTM()
{
}

void CAlg_BTM::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	gamma = config.btm_gamma;
	active_set.clear();
	counts.clear();
	wins.clear();
	counts_active.clear();
	wins_active.clear();
	empi_beat_prob.clear();
	for (int i = 0; i < num_arms; i++) {
		active_set.push_back(i);
		counts.push_back(vector<int>(num_arms, 0));
		wins.push_back(vector<int>(num_arms, 0));
		counts_active.push_back(0);
		wins_active.push_back(0);
		empi_beat_prob.push_back(0.0);
	}
}


int CAlg_BTM::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	// return the one in the active set if there is only one arm
	if ((int)active_set.size() == 1) {
		cmpPair.push_back(active_set[0]);
		cmpPair.push_back(active_set[0]);
		return active_set[0];
	} 

	// find the arm with minimum comparisons in the active set
	for (int ind1 = 0; ind1 < (int)active_set.size(); ind1++) {
		int ind_arm1 = active_set[ind1];
		counts_active[ind_arm1] = 0;
		wins_active[ind_arm1] = 0;
		for (int ind2 = 0; ind2 < (int)active_set.size(); ind2++) {
			int ind_arm2 = active_set[ind2];
			counts_active[ind_arm1] += counts[ind_arm1][ind_arm2];
			wins_active[ind_arm1] += wins[ind_arm1][ind_arm2];
		}
		if (counts_active[ind_arm1] == 0) {
			empi_beat_prob[ind_arm1] = 0.5;
		}
		else {
			empi_beat_prob[ind_arm1] = (double)wins_active[ind_arm1] / (double)counts_active[ind_arm1];
		}
	}

	int ind_first = argminGivenIndSet(counts_active, active_set);
	
	// Find the second candidate
	int ind_second = active_set[rand() % (int)active_set.size()];
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	counts[ind_first][ind_second]++;   // different from other algorithms, 
	                                   // BTM only counts the comparisons that one arm is choosen as the first candidate

	if (beat[ind_first][ind_second] == 1) {
		wins[ind_first][ind_second] += beat[ind_first][ind_second];
	}

	// Check the confidence level and update active set
	int min_cmps = counts_active[ind_first];
	double confidence_r;      //
	if (min_cmps == 0) {
		confidence_r = 1.0;
	}
	else {
		confidence_r = 3.0 * gamma * gamma * sqrt(1.0 / min_cmps * log(1.0 * 2 * num_arms * num_slots));
	}
	int min_prob_ind;
	double min_empi_prob = 1.0;
	double max_empi_prob = 0.0;
	for (int ind = 0; ind < (int)active_set.size(); ind++) {
		int ind_arm = active_set[ind];
		if (empi_beat_prob[ind_arm] < min_empi_prob) {
			min_empi_prob = empi_beat_prob[ind_arm];
			min_prob_ind = ind_arm;
		}
		if (empi_beat_prob[ind_arm] > max_empi_prob) {
			max_empi_prob = empi_beat_prob[ind];
		}
	}
	if (min_empi_prob + confidence_r <= max_empi_prob - confidence_r) {
		for (int i = 0; i < (int)active_set.size(); i++) {
			if (active_set[i] == min_prob_ind) {
				active_set.erase(active_set.begin() + i);
			}
		}		
	}

	if (beat[ind_first][ind_second] == 1) {
		return ind_first;
	} else {
		return ind_second;
	}
}
