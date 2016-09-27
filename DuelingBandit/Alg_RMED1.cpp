#include "stdafx.h"
#include "Alg_RMED1.h"


CAlg_RMED1::CAlg_RMED1()
{
}


CAlg_RMED1::~CAlg_RMED1()
{
}

void CAlg_RMED1::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	num_pairs = num_arms * (num_arms - 1) / 2;
	for (int i = 0; i < num_arms; i++) {
		vector<double> cur_count(num_arms, 0.0);
		vector<double> cur_beat(num_arms, 0.0);
		vector<double> cur_empi_beat_prob(num_arms, 0.0);
		counts.push_back(cur_count);
		wins.push_back(cur_beat);
		empi_beat_prob.push_back(cur_empi_beat_prob);
		empi_div.push_back(0.0);
		current_set.push_back(i);
		remain_set.push_back(i);
	}
	current_ind = 0;
	next_set.clear();
}

int CAlg_RMED1::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	int ind_first, ind_second;
	// Initial stage
	if (ind_slot < num_pairs) {
		for (ind_first = 0; ind_first <= num_arms; ind_first++) {
			if (ind_first * (ind_first - 1) / 2 > ind_slot) {
				ind_first--;
				break;
			}
		}
		ind_second = ind_slot - ind_first * (ind_first - 1) / 2;
	}
	// After 
	else {
		// calculate the empirical preference
		calEmpiBeatProb();

		// First candidate: picked in an arbitrarily fixed order from L_C
		ind_first = current_set[current_ind];
		// Second candidate
		// Find the min-empirical-divengence arm
		double min_div_val = DBL_MAX;
		int min_div_ind = -1;

		for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
			double cur_empi_div = 0.0;
			for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {				
				if (ind_arm2 != ind_arm1 && counts[ind_arm1][ind_arm2] > 0 
					    && wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2] <= 0.5) {
					double empi_beat = empi_beat_prob[ind_arm1][ind_arm2];
					if (empi_beat <= 0.0) {
						cur_empi_div += counts[ind_arm1][ind_arm2] * log(2.0);
					}
					else {
						cur_empi_div += counts[ind_arm1][ind_arm2] * (empi_beat * log(empi_beat / 0.5) + (1 - empi_beat) * log((1 - empi_beat) / 0.5));
					}					
				}
			}
			empi_div[ind_arm1] = cur_empi_div;
			if (cur_empi_div < min_div_val) {
				min_div_ind = ind_arm1;
				min_div_val = cur_empi_div;
			}
		}
		vector<int> cand_set_second;
		for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
			if (ind_arm == ind_first) {
				continue;
			}
			if (empi_beat_prob[ind_first][ind_arm] <= 0.5) {
				cand_set_second.push_back(ind_arm);
			}
		}
		if (cand_set_second.empty() || find(cand_set_second, min_div_ind)) {
			ind_second = min_div_ind;
			//cout << "First " << cand_set_second.size() << " " << ind_first << " " << ind_second << endl;
		}
		else {
			double min_empi_beat = DBL_MAX;
			for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
				double empi_beat = empi_beat_prob[ind_first][ind_arm];
				if (ind_arm != ind_first && empi_beat < min_empi_beat) {
					min_empi_beat = empi_beat;
					ind_second = ind_arm;
				}
			}
			//cout << "Second " << ind_first << " " << ind_second << endl;
		}

		// Update the remaining set
		for (int ind = 0; ind < (int)remain_set.size(); ind++) {
			if (remain_set[ind] == ind_first) {
				remain_set.erase(remain_set.begin() + ind);
				break;
			}
		}
		// Update the next set
		// We use f(K) as: f(K) = 0.3 * K^1.01, as suggested in the paper
		for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
			if (!find(remain_set, ind_arm) && !find(next_set, ind_arm)
				&& empi_div[ind_arm] - min_div_val <= log(ind_slot + 1.0) + 0.3 * pow(num_arms, 1.01)) {
				next_set.push_back(ind_arm);
			}
		}



		// Update current_ind, and renew a round if necessary
		current_ind++;
		if (current_ind >= (int)current_set.size()) {
			current_set.clear();
			remain_set.clear();
			for (int ind = 0; ind < (int)next_set.size(); ind++) {
				current_set.push_back(next_set[ind]);
				remain_set.push_back(next_set[ind]);
			}
			next_set.clear();
			current_ind = 0;
		}
	}

	// decide the pulling arm
	int ind_show = -1;
	if (beat[ind_first][ind_second] == 1) {
		ind_show = ind_first;
	}
	else {
		ind_show = ind_second;
	}
	//cout << ind_first << " " << ind_second << endl;
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;
	wins[ind_first][ind_second] += beat[ind_first][ind_second];
	wins[ind_second][ind_first] += (1 - beat[ind_first][ind_second]);
	return ind_show;
}

void CAlg_RMED1::calEmpiBeatProb() {
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 <= ind_arm1; ind_arm2++) {
			if (counts[ind_arm1][ind_arm2] > 0.0) {
				empi_beat_prob[ind_arm1][ind_arm2] = wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2];				
			}
			else {
				empi_beat_prob[ind_arm1][ind_arm2] = 0.5;
			}
			empi_beat_prob[ind_arm2][ind_arm1] = 1.0 - empi_beat_prob[ind_arm1][ind_arm2];
		}
	}
}


