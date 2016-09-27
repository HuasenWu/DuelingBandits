#include "stdafx.h"
#include "Alg_RMED2.h"


CAlg_RMED2::CAlg_RMED2()
{
}


CAlg_RMED2::~CAlg_RMED2()
{
}

void CAlg_RMED2::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	scale_factor = config.rmed2_alpha;
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

int CAlg_RMED2::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
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
	// Second stage
	else if (ind_slot < num_pairs * scale_factor * log(log(ind_slot + 1))) {
		int ind_pair = ind_slot % num_pairs;
		for (ind_first = 0; ind_first <= num_arms; ind_first++) {
			if (ind_first * (ind_first - 1) / 2 > ind_pair) {
				ind_first--;
				break;
			}
		}
		ind_second = ind_pair - ind_first * (ind_first - 1) / 2;
	}
	// Third stage
	else {
		// First candidate: picked in an arbitrarily fixed order from L_C
		ind_first = current_set[current_ind];
		// Second candidate
		// Find the min-empirical-divengence arm
		// calculate the empirical preference
		calEmpiBeatProb();
		double val_min_div = DBL_MAX;
		int ind_min_div = -1;

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
			if (cur_empi_div < val_min_div) {
				ind_min_div = ind_arm1;
				val_min_div = cur_empi_div;
			}
		}
		// Find \hat{b}^*
		double val_min_ratio = DBL_MAX;
		int ind_min_ratio = -1;
		for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
			if (ind_arm == ind_first) {
				continue;
			}
			
			double cur_ratio = 0;
			if (empi_beat_prob[ind_first][ind_arm] >= 0.5) {
				cur_ratio = DBL_MAX;
			}
			else {
				double div_plus;
				if (empi_beat_prob[ind_first][ind_arm] <= 0.0) {
					div_plus = log(2.0);
				}
				else {
					div_plus = empi_beat_prob[ind_first][ind_arm] * log(empi_beat_prob[ind_first][ind_arm] / 0.5)
						+ (1.0 - empi_beat_prob[ind_first][ind_arm]) * log((1.0 - empi_beat_prob[ind_first][ind_arm]) / 0.5);
				}					
				cur_ratio = 1.0 - empi_beat_prob[ind_min_div][ind_first] - empi_beat_prob[ind_min_div][ind_arm];
				cur_ratio /= div_plus;
			}
			if (cur_ratio < val_min_ratio) {
				val_min_ratio = cur_ratio;
				ind_min_ratio = ind_arm;
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
		if (find(cand_set_second, ind_min_ratio) && counts[ind_first][ind_min_div] >= counts[ind_first][ind_min_ratio] / log(log(ind_slot + 1))) {
			ind_second = ind_min_ratio;
		}
		else {
			if (cand_set_second.empty() || find(cand_set_second, ind_min_div)) {
				ind_second = ind_min_div;
			}
			else {
				double min_empi_beat = DBL_MAX;
				for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
					if (ind_arm != ind_first && empi_beat_prob[ind_first][ind_arm] < min_empi_beat) {
						min_empi_beat = empi_beat_prob[ind_first][ind_arm];
						ind_second = ind_arm;
					}
				}
			}
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
				&& empi_div[ind_arm] - val_min_div <= log(ind_slot + 1) + 0.3 * pow(num_arms, 1.01)) {
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
	
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;
	wins[ind_first][ind_second] += beat[ind_first][ind_second];
	wins[ind_second][ind_first] += (1 - beat[ind_first][ind_second]);
	return ind_show;
}

void CAlg_RMED2::calEmpiBeatProb() {
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


