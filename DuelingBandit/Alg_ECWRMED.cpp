#include "stdafx.h"
#include "Alg_ECWRMED.h"
#include<map>


CAlg_ECWRMED::CAlg_ECWRMED()
{
}


CAlg_ECWRMED::~CAlg_ECWRMED()
{
}

void CAlg_ECWRMED::init(int num_slots, int num_arms, struConfig config){
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	num_pairs = num_arms * (num_arms - 1) / 2;
	alpha = config.ecwrmed_alpha;
	beta = config.ecwrmed_beta;

	current_ind = 0;
	counts.clear();
	wins.clear();
	empi_beat_prob.clear();
	current_set.clear();
	remain_set.clear();
	next_set.clear();
	min_num_superiors = vector<int>(2, 0);

	for (int i = 0; i < num_arms; i++) {
		counts.push_back(vector<double>(num_arms, 0.0));
		wins.push_back(vector<double>(num_arms, 0.0));
		empi_beat_prob.push_back(vector<double>(num_arms, 0.5));
		for (int j = 0; j < i; j++) {
			current_set.push_back(i * num_arms + j);
			remain_set.push_back(i * num_arms + j);
		}
	}
}


int CAlg_ECWRMED::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	int ind_first, ind_second;
	cmpPair.clear();

	// initial exploration
	ind_first = -1;
	ind_second = -1;
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j < i; j++) {
			if (counts[i][j] < alpha * sqrt(log(ind_slot + 1))
				|| abs(empi_beat_prob[i][j] - 0.5) < beta / max(1.0, log(max(1.0, log(ind_slot + 1))))) {
				ind_first = i;
				ind_second = j;
				break;
			}
		}
		// draw and update
		if (ind_first >= 0 && ind_second >= 0) {
			update(ind_first, ind_second, beat[ind_first][ind_second]);
			break;
		}
	}

	// Explore each pair in L_C
	if (ind_first < 0 || ind_second < 0) {
		// select the candidate pairs
		int ind_pair = current_set[current_ind];
		ind_first = ind_pair / num_arms;
		ind_second = ind_pair % num_arms;
		vector<int> next_cand_set;

		// update information
		update(ind_first, ind_second, beat[ind_first][ind_second]);
		vector<int> cpldWinnersCand = findCpldWinnersAndUpdateL(empi_beat_prob);

		// confidence check
		// ind_winner = -1 means no one is confident enough
		int ind_winner = checkConfidence(ind_slot, cpldWinnersCand);

		// some arm is confident enough to be the winner
		if (ind_winner >= 0) {
			next_cand_set.push_back(ind_winner * num_arms + ind_winner);
		}
		// no one is confident enough
		else {
			vector<vector<double>> argmin_q;
			ind_winner = findMinRegret(empi_beat_prob, argmin_q);
			for (int i = 0; i < num_arms; i++) {
				for (int j = 0; j < i; j++) {
					if (argmin_q[i][j] > counts[i][j] / max(1.0, log(ind_slot + 1))) {
						next_cand_set.push_back(i * num_arms + j);
					}
				}
			}
			next_cand_set.push_back(ind_winner * num_arms + ind_winner);
		}

		// Update arm set
		for (int i = 0; i < (int)remain_set.size(); i++) {
			if (remain_set[i] == ind_pair) {
				remain_set.erase(remain_set.begin() + i);
				break;
			}
		}
		for (int i = 0; i < (int)next_cand_set.size(); i++) {
			if (!find(next_set, next_cand_set[i]) && !find(remain_set, next_cand_set[i])) {
				next_set.push_back(next_cand_set[i]);
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
				//cout << next_set[ind] << " ";
			}
			//cout << endl;
			next_set.clear();
			current_ind = 0;
		}
	}

	// return the results
	//cout << "ECW " << ind_first << " " << ind_second << endl;
	////cout << "True " << endl;
	////for (int i = 0; i < num_arms; i++) {
	////	for (int j = 0; j < num_arms; j++) {
	////		cout << pref_mat[i][j] << " ";
	////	}
	////	cout << endl;
	////}
	//cout << "EmpiProb " << endl;
	//for (int i = 0; i < num_arms; i++) {
	//	for (int j = 0; j < num_arms; j++) {
	//		cout << empi_beat_prob[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << "Count " << endl;
	//for (int i = 0; i < num_arms; i++) {
	//	for (int j = 0; j < num_arms; j++) {
	//		cout << counts[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	if (beat[ind_first][ind_second] == 1) {
		return ind_first;
	}
	else {
		return ind_second;
	}
}

void CAlg_ECWRMED::update(int ind_first, int ind_second, int firstBeatsSec) {
	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;
	if (firstBeatsSec == 1) {
		wins[ind_first][ind_second] += 1.0;
	}
	else {
		wins[ind_second][ind_first] += 1.0;
	}
	empi_beat_prob[ind_first][ind_second] = wins[ind_first][ind_second] / counts[ind_first][ind_second];
	empi_beat_prob[ind_second][ind_first] = wins[ind_second][ind_first] / counts[ind_second][ind_first];
}

vector<int> CAlg_ECWRMED::findCpldWinnersAndUpdateL(vector<vector<double>> pref_mat) {
	vector<int> cpldScore(num_arms, 0);
	vector<int> cpldWinners;
	int max_score = 0;
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j < num_arms; j++) {
			if (i != j && pref_mat[i][j] > 0.5) {
				cpldScore[i]++;
			}
		}
		if (cpldScore[i] > max_score) {
			max_score = cpldScore[i];
		}
	}
	for (int i = 0; i < num_arms; i++) {
		if (cpldScore[i] == max_score) {
			cpldWinners.push_back(i);
		}
	}

	// update the number of superiors
	sort(cpldScore.begin(), cpldScore.end());
	min_num_superiors[0] = num_arms - 1 - cpldScore[num_arms - 1];
	min_num_superiors[1] = num_arms - 1 - cpldScore[num_arms - 2];

	return cpldWinners;
}

int CAlg_ECWRMED::checkConfidence(int ind_slot, vector<int> candidates) {
	int ind_winner = -1;
	for (int i1 : candidates) {
		bool isSatisfied = true;
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j < i; j++) {
				if (counts[i][j] / max(1.0, log(ind_slot + 1.0))
					* klDiv(empi_beat_prob[i][j], 0.5) > 1.0) {
					isSatisfied = false;
					break;
				}
			}
			if (!isSatisfied) {
				break;
			}
		}
		if (!isSatisfied) {
			continue;
		}
		vector<int> test_set_i1;
		vector<double> empi_div_i1;
		// inferiors
		vector<int> inferiors_i1;
		for (int j = 0; j < num_arms; j++) {
			if (j != i1 && empi_beat_prob[i1][j] > 0.5) {
				if (i1 > j) {
					test_set_i1.push_back(i1 * num_arms + j);
				}
				else {
					test_set_i1.push_back(j * num_arms + i1);
				}
				double cur_div = counts[i1][j] / max(1.0, log(ind_slot + 1.0))
					* klDiv(empi_beat_prob[i1][j], 0.5);
				empi_div_i1.push_back(cur_div);
			}
		}

		// check for all i2 != i1
		for (int i2 = 0; i2 < num_arms; i2++) {
			if (i2 == i1) {
				continue;
			}

			int L_i2 = 0;
			vector<int> test_set(test_set_i1);
			vector<double> empi_div(empi_div_i1);
			for (int j = 0; j < num_arms; j++) {
				if (j != i2 && empi_beat_prob[j][i2] > 0.5) {
					L_i2++;
					if (j != i1) {
						if (i2 > j) {
							test_set.push_back(i2 * num_arms + j);
						}
						else {
							test_set.push_back(j * num_arms + i2);
						}
						empi_div.push_back(counts[i2][j] / max(1.0, log(ind_slot + 1.0))
							* klDiv(empi_beat_prob[i2][j], 0.5));
					}					
				}
			}

			
			//sort(test_set.begin(), test_set.end(), 
			//	[&empi_div](size_t ii, size_t jj) { return empi_div[ii] < empi_div[jj]; });

			vector<int> idx = sort_indices(empi_div);
			for (int l = max(0, min_num_superiors[0]); l <= min_num_superiors[1]; l++) {
				double sum = 0.0;
				int size_H = l + 1 - min_num_superiors[0];
				int size_O = max(0, L_i2 - l);
				int iH = 0;
				int iO = 0;
				int iAll = 0;

				while ((iH < size_H || iO < size_O) && iAll < (int)test_set.size()) {
					if (iH < size_H && find(test_set_i1, test_set[idx[iAll]])) {
						sum += empi_div[idx[iAll]];						
						if ((test_set[idx[iAll]] / num_arms == i1 && test_set[idx[iAll]] % num_arms == i2) ||
							(test_set[idx[iAll]] / num_arms == i2 && test_set[idx[iAll]] % num_arms == i1)) {
							iO++;    // if i2 \in H_i1, we need to subtract the number of elements by 1.
						}
						iH++;
						iAll++;
					}
					else if (iO < size_O && !find(test_set_i1, test_set[idx[iAll]])) {
						sum += empi_div[idx[iAll]];
						iO++;
						iAll++;
					}
					else {
						iAll++;
					}
				}

				if (sum < 1.0) {
					isSatisfied = false;
					break;
				}
			}
		}
		if (isSatisfied) {
			ind_winner = i1;
			return ind_winner;
		}		
	}
	return ind_winner;
}

vector<vector<int>> CAlg_ECWRMED::getSubset(vector<int> set, int size) {
	vector<vector<int>> subsets;
	int n = (int)set.size();
	vector<bool> v(n, false);
	fill(v.begin() + n - size, v.end(), true);
	do {
		vector<int> cur_subset;
		for (int i = 0; i < n; i++) {
			if (v[i]) {
				cur_subset.push_back(i);
			}
		}
		subsets.push_back(cur_subset);
	} while (next_permutation(v.begin(), v.end()));
	return subsets;
}

int CAlg_ECWRMED::findMinRegret(vector<vector<double>> pref_mat, vector<vector<double>>& argmin_q) {
	vector<int> num_superiors(num_arms, 0);
	vector<int> candidates;
	int min_loss = INT_MAX;
	vector<vector<int>> inferiors_set;
	vector<vector<double>> regret(num_arms, vector<double>(num_arms, 0.0));
	int ind_winner = -1; 
	for (int i = 0; i < num_arms; i++) {
		vector<int> cur_inferiors;
		for (int j = 0; j < num_arms; j++) {
			if (j != i && pref_mat[j][i] > 0.5) {
				num_superiors[i]++;
			}
			if (j != i && pref_mat[j][i] < 0.5) {
				cur_inferiors.push_back(j);
			}
		}
		inferiors_set.push_back(cur_inferiors);
		if (num_superiors[i] < min_loss) {
			min_loss = num_superiors[i];
		}
	}
	for (int i = 0; i < num_arms; i++) {
		if (num_superiors[i] == min_loss) {
			candidates.push_back(i);
		}
	}

	// regret
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j < num_arms; j++) {
			regret[i][j] = (num_superiors[i] + num_superiors[j] - 2.0 * min_loss) / 2.0 / (num_arms - 1.0);
		}
	}
	double min_regret = DBL_MAX;
	argmin_q.clear();
	for (int i = 0; i < num_arms; i++) {
		vector<double> q_vec;
		for (int j = 0; j < i; j++) {
			q_vec.push_back(0.0);
		}
		argmin_q.push_back(q_vec);
	}
	for (int i1 : candidates) {
		vector<vector<double>> cur_q;
		for (int ii = 0; ii < num_arms; ii++) {
			vector<double> q_vec;
			for (int jj = 0; jj < ii; jj++) {
				q_vec.push_back(0.0);
			}
			cur_q.push_back(q_vec);
		}
		for (int j : inferiors_set[i1]) {
			if (i1 > j) {
				cur_q[i1][j] = 1.0 / klDiv(pref_mat[i1][j], 0.5);
			}
			else {
				cur_q[j][i1] = 1.0 / klDiv(pref_mat[i1][j], 0.5);
			}
		}
		for (int i2 = 0; i2 < num_arms; i2++) {
			if (i2 == i1) {
				continue;
			}
			vector<int> superiors;
			vector<double> costs;
			for (int j = 0; j < num_arms; j++) {
				if (j != i1 && j != i2 && pref_mat[j][i2] > 0.5) {
					double cost = regret[j][i2] / klDiv(pref_mat[j][i2], 0.5);
					superiors.push_back(j);
					costs.push_back(cost);
				}
			}
			int k = (int)superiors.size() - (num_superiors[i2] - num_superiors[i1] + 1);
			if (k >= 0 && (int)superiors.size() > k) {
				//sort(superiors.begin(), superiors.end(),
				//	[&costs](size_t ii, size_t jj) {return costs[ii] < costs[jj]; });

				vector<int> sup_idx = sort_indices(costs);

				double min_obj = DBL_MAX;
				int min_h = -1;
				for (int h = k + 1; h <= (int)superiors.size(); h++) {
					double cur_obj = 0.0;
					for (int jj = 0; jj < h; jj++) {
						cur_obj += regret[superiors[sup_idx[jj]]][i2]
							/ klDiv(pref_mat[superiors[sup_idx[jj]]][i2], 0.5) / (h - k);
					}
					
					if (cur_obj < min_obj) {
						min_obj = cur_obj;
						min_h = h;						
					}					
				}
				for (int jj = 0; jj < min_h; jj++) {
					if (superiors[sup_idx[jj]] > i2) {
						cur_q[superiors[sup_idx[jj]]][i2] = 1.0 / (min_h - k) / klDiv(pref_mat[superiors[sup_idx[jj]]][i2], 0.5);
					}
					else {
						cur_q[i2][superiors[sup_idx[jj]]] = 1.0 / (min_h - k) / klDiv(pref_mat[superiors[sup_idx[jj]]][i2], 0.5);
					}					
				}
			}
			//cout << "here2" << endl;
			
		}
		double cur_regret = 0.0;
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j < i; j++) {
				cur_regret += regret[i][j] * cur_q[i][j];
			}
		}
		if (cur_regret < min_regret) {
			min_regret = cur_regret;
			ind_winner = i1;
			for (int i = 0; i < num_arms; i++) {
				for (int j = 0; j < i; j++) {
					argmin_q[i][j] = cur_q[i][j];
				}
			}
		}	

	}
	return ind_winner;
}

