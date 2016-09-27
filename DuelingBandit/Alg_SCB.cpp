#include "stdafx.h"
#include "Alg_SCB.h"


CAlg_SCB::CAlg_SCB()
{
}


CAlg_SCB::~CAlg_SCB()
{
}

// initialization
void CAlg_SCB::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	eps_KLRCB = config.klucb_eps;
	scale_factor_KLRCB = config.ucb_scale_factor;

	ind_best = -1;

	duringPairExplore = false;
	ind_curFirst = -1;
	ind_curSecond = -1;

	roundInd = 1;
	roundLength = 1000;
	curStep = 1;
	virtual_running_index = 0;
	failProb = log(roundLength) / roundLength;

	remainArms.clear();

	wins.clear();
	counts.clear();
	valid_arms.clear();
	virtual_plays.clear();
	virtual_rewards.clear();
	ucb.clear();
	lcb.clear();

	for (int i = 0; i < num_arms; i++) {
		remainArms.push_back(i);

		wins.push_back(vector<double>(num_arms, 0.000001));
		counts.push_back(vector<double>(num_arms, 0.000002));

		vector<int> cur_valid_arms;
		for (int j = 0; j < num_arms; j++) {
			if (j != i) {
				cur_valid_arms.push_back(j);
			}
		}
		valid_arms.push_back(cur_valid_arms);

		virtual_plays.push_back(0.0);
		virtual_rewards.push_back(0.0);
		ucb.push_back(1.0);
		lcb.push_back(0.0);
	}
}

// select the pair for comparison
int CAlg_SCB::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	cmpPair.clear();
	int ind_first, ind_second;
	// return the best arm if the exploration process is done
	if (ind_best >= 0) {
		ind_first = ind_best;
		ind_second = ind_best;
	}
	// if is during the pair explore 
	else if (duringPairExplore) {
		ind_first = ind_curFirst;
		ind_second = ind_curSecond;
	}
	// find a new pair to explore
	else {
		while (true) {
			ind_first = remainArms[virtual_running_index];
			ind_second = rand() % num_arms;
			while (ind_second == ind_first) {
				ind_second = rand() % num_arms;
			}

			// if PrefMat have been learned w.h.p., we can find the Copeland winner
			bool isLearned = true;
			for (int i = 0; i < (int)remainArms.size(); i++) {
				int ind_arm = remainArms[i];
				if (!valid_arms[ind_arm].empty()) {
					isLearned = false;
					break;
				}
			}
			if (isLearned) {
				int max_cpld = -1;
				for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
					int cur_cpld = 0;
					for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
						if (ind_arm2 != ind_arm1 && wins[ind_arm1][ind_arm2] / counts[ind_arm1][ind_arm2] > 0.5) {
							cur_cpld++;
						}
					}
					if (cur_cpld > max_cpld) {
						ind_best = ind_arm1;
						max_cpld = cur_cpld;
					}
				}
				ind_first = ind_best;
				ind_second = ind_best;
				break;
			}
			else {
				// if we already know the answer of (ind_first, ind_second), don't query them
				if (!find(valid_arms[ind_first], ind_second)) {
					if (wins[ind_first][ind_second] / counts[ind_first][ind_second] > 0.5) {
						updateVirtualMAB(1);
					}
					else {
						updateVirtualMAB(0);
					}
					if (ind_best >= 0) {
						ind_first = ind_best;
						ind_second = ind_best;
						break;
					}
				}
				else {
					break;
					// start to explore this pair if we don't know the answer
				}
			}
		}
		// start to explore
		if (ind_best < 0) {
			duringPairExplore = true;
			ind_curFirst = ind_first;
			ind_curSecond = ind_second;
		}
	}

	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	// compare and update
	int ind_show = -1;
	int ind_winner = -1;
	int ind_loser = -1;
	if (beat[ind_first][ind_second] == 1) {
		ind_show = ind_first;
		ind_winner = ind_first;
		ind_loser = ind_second;
	}
	else {
		ind_show = ind_second;
		ind_winner = ind_second;
		ind_loser = ind_first;
	}

	// check the confidence
	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;
	wins[ind_first][ind_second] += beat[ind_first][ind_second];
	wins[ind_second][ind_first] += (1 - beat[ind_first][ind_second]);
	if (checkConfidence(wins[ind_winner][ind_loser], counts[ind_winner][ind_loser])) {
		for (int i = 0; i < (int)valid_arms[ind_winner].size(); i++) {
			if (valid_arms[ind_winner][i] == ind_loser) {
				valid_arms[ind_winner].erase(valid_arms[ind_winner].begin() + i);
				break;
			}
		}
		for (int i = 0; i < (int)valid_arms[ind_loser].size(); i++) {
			if (valid_arms[ind_loser][i] == ind_winner) {
				valid_arms[ind_loser].erase(valid_arms[ind_loser].begin() + i);
				break;
			}
		}
		if (ind_winner == ind_first) {
			updateVirtualMAB(1);
		}
		else {
			updateVirtualMAB(0);
		}
		duringPairExplore = false;
	}

	// update current index 
	curStep++;
	if (curStep > roundLength) {
		reset();
	}
	return ind_show;
}

// update the virtual MAB
void CAlg_SCB::updateVirtualMAB(int reward) {
	int ind_arm = remainArms[virtual_running_index];
	virtual_plays[ind_arm] += 1.0;
	virtual_rewards[ind_arm] += reward;
	virtual_running_index++;
	if (virtual_running_index >= (int)remainArms.size()) {
		virtual_running_index = 0;

		calKLCB();
		double max_lcb = -1.0;


		for (int i = 0; i < (int)remainArms.size(); i++) {
			int ind_arm = remainArms[i];
			if (lcb[ind_arm] > max_lcb) {
				max_lcb = lcb[ind_arm];
			}
		}

		for (int i = 0; i < (int)remainArms.size();) {
			int ind_arm = remainArms[i];
			if (ucb[ind_arm] < max_lcb) {
				remainArms.erase(remainArms.begin() + i);
			}
			else {
				i++;
			}
		}

		if (remainArms.size() == 1) {
			ind_best = remainArms[0];
		}
		else if (remainArms.size() < 1) {
			ind_best = 0;
		}
	}
}

// reset for each round
void CAlg_SCB::reset() {
	ind_best = -1;

	duringPairExplore = false;
	ind_curFirst = -1;
	ind_curSecond = -1;

	roundInd++;
	roundLength *= roundLength;
	curStep = 1;
	virtual_running_index = 0;
	failProb = log(roundLength) / roundLength;
	remainArms.clear();
	for (int i = 0; i < num_arms; i++) {
		remainArms.push_back(i);
		virtual_plays[i] = 0.0;
		virtual_rewards[i] = 0.0;
		for (int j = 0; j < num_arms; j++) {
			counts[i][j] = 0.000002;
			wins[i][j] = 0.000001;
		}
		valid_arms[i].clear();
		for (int j = 0; j < num_arms; j++) {
			if (j != i) {
				valid_arms[i].push_back(j);
			}
		}
	}
}

bool CAlg_SCB::checkConfidence(double num_wins, double num_plays) {
	//if (curStep % 10000 == 0) {
	//	cout << "T = " << roundLength << "  t = " << curStep << " " << ind_best << endl;
	//	for (int i = 0; i < num_arms; i++) {
	//		cout << valid_arms[i].size() << endl;
	//	}
	//	cout << "Est_PMat" << endl;
	//	for (int i = 0; i < num_arms; i++) {
	//		for (int j = 0; j < num_arms; j++) {
	//			cout << wins[i][j] / counts[i][j] << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << " cur index " << virtual_running_index << endl;
	//	cout << "Virtual Wins" << endl;
	//	for (int i = 0; i < num_arms; i++) {
	//		cout << virtual_rewards[i] << " " << virtual_plays[i] << endl;
	//	}
	//}

	if (num_plays < 1 || num_wins / num_plays < 0.5) {
		return false;
	}
	if (log(4.0 * curStep * num_arms) < 0)
	{
		cout << log(4.0 * curStep * num_arms) << endl;
	}

	double interv_width = scale_factor_KLRCB * (log(4.0 * curStep * num_arms) +
		2.0 * max(1.0, log(max(1.0, log(curStep))))) / num_plays;
	double prob = num_wins / num_plays;
	return prob * log(prob / 0.5) + (1 - prob) * log((1.0 - prob) / 0.5) > interv_width;
}

void CAlg_SCB::calKLCB() {
	for (int ind = 0; ind < (int)remainArms.size(); ind++) {
		int ind_arm = remainArms[ind];

		// empirical reward
		double prob = -1.0;
		if (virtual_plays[ind_arm] > 0) {
			prob = virtual_rewards[ind_arm] / virtual_plays[ind_arm];
		}
		if (prob < 0.0) {
			ucb[ind_arm] = 1.0;
			lcb[ind_arm] = 0.0;
			continue;
		}

		// right hand side of the KL-RCB definition
		//double rhs = log(4.0 * curStep * num_arms / failProb) + 2 * max(1.0, log(max(1.0, log(curStep)))); 
		double rhs = log(4.0 * roundLength * num_arms) + 2 * max(1.0, log(max(1.0, log(curStep))));
		rhs = scale_factor_KLRCB * rhs / virtual_plays[ind_arm];

		// UCB
		double cur_ucb = 0.0;
		if (prob >= 1.0) {
			cur_ucb = 1.0;
		}
		else if (prob <= 0.0) {
			cur_ucb = 1.0 - exp(-1.0 * rhs);
		}
		else {
			double start = prob;
			double end = 1 - eps_KLRCB;
			if (klDiv(prob, end) <= rhs) {
				cur_ucb = end;
			}
			else {
				while ((end - start) > eps_KLRCB) {
					double mid = (start + end) / 2.0;
					if (klDiv(prob, mid) < rhs) {
						start = mid;
					}
					else {
						end = mid;
					}
				}
				cur_ucb = (start + end) / 2.0;
			}
		}
		ucb[ind_arm] = cur_ucb;

		// LCB
		double cur_lcb = 0.0;
		if (prob >= 1.0) {
			cur_lcb = exp(-1.0 * rhs);
		}
		else if (prob <= 0.0) {
			cur_lcb = 0.0;
		}
		else {
			double start = eps_KLRCB;
			double end = prob;
			if (klDiv(prob, start) <= rhs) {
				cur_lcb = start;
			}
			else {
				while ((end - start) > eps_KLRCB) {
					double mid = (start + end) / 2.0;
					if (klDiv(prob, mid) < rhs) {
						end = mid;
					}
					else {
						start = mid;
					}
				}
				cur_lcb = (start + end) / 2.0;
			}
		}
		lcb[ind_arm] = cur_lcb;
	}
}
