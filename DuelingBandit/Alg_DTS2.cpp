#include "stdafx.h"
#include "Alg_DTS2.h"


#include "stdafx.h"
#include "Alg_DTS.h"


CAlg_DTS2::CAlg_DTS2()
{
}


CAlg_DTS2::~CAlg_DTS2()
{
}

void CAlg_DTS2::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	num_pairs = num_arms * (num_arms - 1) / 2;

	scale_factor_RCB = config.ucb_scale_factor;
	ctrl_factor_KLRCB = config.klucb_ctrl_factor;
	eps_KLRCB = config.klucb_eps;
	counts.clear();
	wins.clear();
	rucb.clear();
	rlcb.clear();
	upper_Cpld.clear();
	lower_Cpld.clear();
	samples.clear();
	for (int i = 0; i < num_arms; i++) {
		counts.push_back(vector<double>(num_arms, 0.000001));
		wins.push_back(vector<double>(num_arms, 0.00001));
		rucb.push_back(vector<double>(num_arms, 1.0));
		rlcb.push_back(vector<double>(num_arms, 0.0));
		upper_Cpld.push_back(num_arms);
		lower_Cpld.push_back(0);
		samples.push_back(vector<double>(num_arms, 0.5));
	}

	// random generator
	random_device rd;
	rndEngine = mt19937(rd());
}
int CAlg_DTS2::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	cmpPair.clear();

	// First round TS, find the best candidate
	int ind_first;

	// Calculate RUCB/RLCB
	calRCB(ind_slot);
	//calKLRCB(ind_slot);

	// Calculate the Upper/Lower bounds for the Copeland score
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		upper_Cpld[ind_arm1] = 0;
		lower_Cpld[ind_arm1] = 0;
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			//cout << rucb[ind_arm1][ind_arm2] << " ";
			if (ind_arm2 != ind_arm1 && rucb[ind_arm1][ind_arm2] >= 0.5) {
				upper_Cpld[ind_arm1]++;
			}
		}
		//cout << endl;
	}

	// Find the set with largest score
	vector<int> candidates = argmaxAll(upper_Cpld);

	// Select the best arm within the candidate set
	if (candidates.empty()) {
		ind_first = rand() % num_arms;
	}
	else {
		// sample and count the comparison results
		vector<int> counts1(num_arms, 0);
		genSamples();
		for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
			for (int ind_arm2 = 0; ind_arm2 < ind_arm1; ind_arm2++) {
				if (ind_arm2 == ind_arm1) {
					continue;
				}
				double sample = samples[ind_arm1][ind_arm2];
				if (sample > 0.5) {
					counts1[ind_arm1]++;
				}
				else {
					counts1[ind_arm2]++;
				}
			}
		}
		ind_first = selectFirstCandidate(counts1, candidates); //ind_first = argmaxGivenIndSet(counts1, candidates);
	}


	// Second round TS, find the comparison arm
	int ind_second;
	/*
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
	if (ind_arm == ind_first) {
	beat_prob[ind_arm] = 0.5;
	}
	else {
	// sample the distribution
	std::mt19937 gen(rd());
	sftrabbit::beta_distribution<> beta(wins[ind_arm][ind_first] + 1.0,
	wins[ind_first][ind_arm] + 1.0);
	beat_prob[ind_arm] = beta(gen);
	}
	}
	*/
	genSamples(ind_first);
	ind_second = -1;
	double max_beat_prob = -1.0;
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		if (rlcb[ind_arm][ind_first] <= 0.5 &&  samples[ind_arm][ind_first] > max_beat_prob) {
			max_beat_prob = samples[ind_arm][ind_first];
			ind_second = ind_arm;
		}
	}
	if (ind_second < 0) {
		ind_second = rand() % num_arms;
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

int CAlg_DTS2::selectFirstCandidate(vector<int> cpldScores, vector<int> candidates) {
	if (candidates.empty()) {
		return -1;
	}
	// find the maximum value
	int len = (int)candidates.size();
	int max_score = INT_MIN;
	for (int ind : candidates) {
		if (cpldScores[ind] > max_score) {
			max_score = cpldScores[ind];
		}
	}

	// find the argMax indices
	vector<int> max_ind;
	for (int ind : candidates) {
		if (cpldScores[ind] == max_score) {
			max_ind.push_back(ind);
		}
	}
	if ((int)max_ind.size() == 1) {
		return max_ind[0];
	}

	// break the ties
	double min_regret = DBL_MAX;
	int ind_winner = -1;
	for (int ind : max_ind) {
		double cur_regret = 0.0; 
		for (int j = 0; j < num_arms; j++) {
			if (j != ind && samples[ind][j] != 0.5) {
				cur_regret += (2.0 * max_score - cpldScores[ind] - cpldScores[j]) / klDiv(samples[ind][j], 0.5);
			}
		}
		if (cur_regret < min_regret) {
			min_regret = cur_regret;
			ind_winner = ind;
		}
	}
	return ind_winner;
}

void CAlg_DTS2::calRCB(int ind_slot) {
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 <= ind_arm1; ind_arm2++) {
			if (ind_arm2 != ind_arm1) {
				double empi_prob = wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2];
				double interv_width = sqrt(scale_factor_RCB * log(ind_slot + 1.0) / counts[ind_arm1][ind_arm2]);
				rucb[ind_arm1][ind_arm2] = empi_prob + interv_width;    // Relative Upper Confidence Bound (RUCB)			
				rlcb[ind_arm1][ind_arm2] = empi_prob - interv_width;   // Relative Lower Confidence Bound (RLCB)
				rucb[ind_arm2][ind_arm1] = 1.0 - rlcb[ind_arm1][ind_arm2];
				rlcb[ind_arm2][ind_arm1] = 1.0 - rucb[ind_arm1][ind_arm2];
			}
			else {
				rucb[ind_arm1][ind_arm2] = 0.5;
				rlcb[ind_arm1][ind_arm2] = 0.5;
			}
		}
	}
}

void CAlg_DTS2::calKLRCB(int ind_slot) {
	// for KL-RLCB: we use an approximation, rather than the defintion - the RLCB of p_{ij} is given by l_{ij} = 1 - u_{ji}
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm2 == ind_arm1) {
				rucb[ind_arm1][ind_arm2] = 0.5;
				rlcb[ind_arm1][ind_arm2] = 0.5;
				continue;
			}
			double cur_ucb = 0.0;
			double rhs = log((ind_slot + 1.0) / counts[ind_arm1][ind_arm2]) + ctrl_factor_KLRCB * log(log(ind_slot + 1.0 + eps_KLRCB));
			rhs /= counts[ind_arm1][ind_arm2];
			//double rhs = scale_factor * log(ind_slot + 1.0) / counts[ind_arm][ind_arm2];
			double prob = wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2];
			if (prob >= 1.0) {
				cur_ucb = 1.0;
			}
			else if (prob <= 0.0) {
				cur_ucb = 1.0 - exp(-1.0 * rhs);
			}
			else {
				rhs -= (prob * log(prob) + (1 - prob) * log(1 - prob));
				double start = prob;
				double end = 1 - eps_KLRCB;
				if (-1.0 *  prob * log(end) - (1.0 - prob) * log(1 - end) < rhs) {
					cur_ucb = end;
				}
				else {
					while ((end - start) > eps_KLRCB) {
						double mid = (start + end) / 2.0;
						if (-1.0 * prob * log(mid) - (1.0 - prob) * log(1 - mid) < rhs) {
							start = mid;
						}
						else {
							end = mid;
						}
					}
					cur_ucb = (start + end) / 2.0;
				}
			}
			rucb[ind_arm1][ind_arm2] = cur_ucb;
			rlcb[ind_arm2][ind_arm1] = 1 - rucb[ind_arm1][ind_arm2];
		}
	}
}

void CAlg_DTS2::genSamples() {
	double gamma_scale = 2.0;
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 <= ind_arm1; ind_arm2++) {
			if (ind_arm2 == ind_arm1) {
				samples[ind_arm1][ind_arm2] = 0.5;
			}
			else {
				gamma_distribution<> gamma1(wins[ind_arm1][ind_arm2] + 1.0, gamma_scale);
				gamma_distribution<> gamma2(wins[ind_arm2][ind_arm1] + 1.0, gamma_scale);
				double x = gamma1(rndEngine);
				double y = gamma2(rndEngine);
				if (x + y == 0) {
					samples[ind_arm1][ind_arm2] = 0.5;
				}
				else{
					samples[ind_arm1][ind_arm2] = x / (x + y);
				}
				samples[ind_arm2][ind_arm1] = 1 - samples[ind_arm1][ind_arm2];
			}
		}
	}
}

void CAlg_DTS2::genSamples(int ind_col) {
	double gamma_scale = 2.0;
	for (int ind_row = 0; ind_row < num_arms; ind_row++) {
		if (ind_row == ind_col) {
			samples[ind_row][ind_col] = 0.5;
		}
		else {
			gamma_distribution<> gamma1(wins[ind_row][ind_col] + 1.0, gamma_scale);
			gamma_distribution<> gamma2(wins[ind_col][ind_row] + 1.0, gamma_scale);
			double x = gamma1(rndEngine);
			double y = gamma2(rndEngine);
			if (x + y == 0) {
				samples[ind_row][ind_col] = 0.5;
			}
			else{
				samples[ind_row][ind_col] = x / (x + y);
			}
		}
	}
}

