#include "stdafx.h"
#include "MAB.h"
#include "Alg_BTM.h"
#include "Alg_SAVAGE.h"
#include "Alg_RUCB.h"
#include "Alg_RCS.h"
#include "Alg_MultiSBM.h"
#include "Alg_Sparring.h"
#include "Alg_CCB.h"
#include "Alg_SCB.h"
#include "Alg_RMED.h"
#include "Alg_ECWRMED.h"
#include "Alg_DTS.h"
#include "Alg_DTS2.h"


CMAB::CMAB()
{
	isEteOnly = false;
	isLargeScale = false;
}

CMAB::CMAB(bool flagEteOnly) {
	isEteOnly = flagEteOnly;
}

CMAB::~CMAB()
{
}
CResults CMAB::singleRun(int num_slots, vector<vector<double>> pref_mat, struConfig config) {
	CResults total_reward = CResults();
	total_reward.init(num_slots);
	int num_arms = (int)pref_mat.size();

	// check the preference matrix
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j <= i; j++) {
			if (pref_mat[i][j] + pref_mat[j][i] != 1.0) {
				double sum = pref_mat[i][j] + pref_mat[j][i];
				if (abs(sum - 1.0) < 0.001) { // certain errors are tolerated 
					pref_mat[i][j] /= sum;
					pref_mat[j][i] /= sum;
				}
				else {
					cout << "MAB.cpp: Invalid preference matrix! " << pref_mat[i][j] << " " << pref_mat[j][i] << endl;
					exit(ERROR_INVALID_PREF_MAT);
				}				
			}
		}
	}

	vector<double> copeland_score;
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		double num_wins = 0;
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm1 != ind_arm2 && pref_mat[ind_arm1][ind_arm2] > 0.5) {
				num_wins += 1.0;
			}
		}
		copeland_score.push_back(num_wins / (num_arms - 1));
	}
	int ind_best = 0;
	for (int ind_arm = 1; ind_arm < num_arms; ind_arm++) {
		if (copeland_score[ind_arm] > copeland_score[ind_best]) {
			ind_best = ind_arm;
		}
	}

	// set the winner type
	if (copeland_score[ind_best] < 1.0) {
		config.winner_type = WINNER_TYPE_NON_CONDORCET;
	}
	else {
		config.winner_type = WINNER_TYPE_CONDORCET;
	}

	// print the winner index and maximum score
	cout << "ind_winners ";
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		if (copeland_score[ind_arm] == copeland_score[ind_best]) {
			cout << ind_arm << " ";
		}
	}
	cout << endl;
	cout << "Max_score " << copeland_score[ind_best] << endl;

	// MAB algorithms
	CAlg_BTM algBTM;
	algBTM.init(num_slots, num_arms, config);
	CAlg_SAVAGE algSAVAGE;
	algSAVAGE.init(num_slots, num_arms, config);
	CAlg_RUCB algRUCB;
	algRUCB.init(num_slots, num_arms, config);
	CAlg_RCS algRCS;
	algRCS.init(num_slots, num_arms, config);
	CAlg_MultiSBM algMultiSBM;
	algMultiSBM.init(num_slots, num_arms, config);
	CAlg_Sparring algSparring;
	algSparring.init(num_slots, num_arms, config);
	CAlg_CCB algCCB;
	algCCB.init(num_slots, num_arms, config);
	CAlg_SCB algSCB;
	algSCB.init(num_slots, num_arms, config);
	CAlg_RMED algRMED1 = CAlg_RMED(ALG_RMED1);
	algRMED1.init(num_slots, num_arms, config);
	CAlg_RMED algRMED2 = CAlg_RMED(ALG_RMED2);
	algRMED2.init(num_slots, num_arms, config);
	CAlg_ECWRMED algECWRMED;
	algECWRMED.init(num_slots, num_arms, config);
	CAlg_DTS algDTS;
	algDTS.init(num_slots, num_arms, config);
	CAlg_DTS2 algDTS2;
	algDTS2.init(num_slots, num_arms, config);

	// beating matrix
	vector<vector<int>> beat;
	vector<double> reward(num_arms, 0);
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		vector<int> cur_beat(num_arms, 0);
		beat.push_back(cur_beat);
	}

	bool regretType_isCpldScore; // flag for the regret definition: true: based on normalized Copeland score, false: Yue's definition
	if (config.regret_type == REGRET_TYPE_ALL_CPLD_SCORE || config.winner_type == WINNER_TYPE_NON_CONDORCET) {
		regretType_isCpldScore = true;
	}
	else {
		regretType_isCpldScore = false;
	}

	// 
	for (int ind_slot = 0; ind_slot < num_slots; ind_slot++) {
		if (ind_slot % 10000 == 0) {
			cout << "t = " << ind_slot << endl;
		}
		//cout << "t = " << ind_slot << endl;
		//for (int i = 0; i < num_arms; i++) {
		//	if (copeland_score[i] == copeland_score[ind_best]) {
		//		cout << i << " ";
		//	}
		//}
		//cout << endl;
		// generate reward 
		for (int ind1 = 0; ind1 < num_arms; ind1++) {
			reward[ind1] = copeland_score[ind1];
			for (int ind2 = 0; ind2 < ind1; ind2++) {
				double beat_prob = pref_mat[ind1][ind2];
				beat[ind1][ind2] = int(double(rand()) / RAND_MAX < beat_prob);
				beat[ind2][ind1] = 1 - beat[ind1][ind2];
			}
			beat[ind1][ind1] = int(double(rand()) / RAND_MAX < 0.5);
		}

		// Oracle
		if (ind_slot > 0) {
			if (regretType_isCpldScore) {
				total_reward.oracle[ind_slot] = total_reward.oracle[ind_slot - 1] + reward[ind_best];
			}
		}
		else {
			if (regretType_isCpldScore) {
				total_reward.oracle[ind_slot] = reward[ind_best];
			}
		}


		// Compare and update


		// BTM
		vector<int> cmp_pair_btm;
		algBTM.pull(ind_slot, reward, beat, cmp_pair_btm);
		if (ind_slot > 0) {
			if (regretType_isCpldScore) {
				total_reward.btm[ind_slot] = total_reward.btm[ind_slot - 1] + (reward[cmp_pair_btm[0]] + reward[cmp_pair_btm[1]]) / 2.0;
			}
			else {
				total_reward.btm[ind_slot] = total_reward.btm[ind_slot - 1] + pref_mat[cmp_pair_btm[0]][ind_best] + pref_mat[cmp_pair_btm[1]][ind_best] - 1.0;
			}

		}
		else {
			if (regretType_isCpldScore) {
				total_reward.btm[ind_slot] = (reward[cmp_pair_btm[0]] + reward[cmp_pair_btm[1]]) / 2.0;
			}
			else {
				total_reward.btm[ind_slot] = pref_mat[cmp_pair_btm[0]][ind_best] + pref_mat[cmp_pair_btm[1]][ind_best] - 1.0;
			}
		}

		// SAVAGE
		vector<int> cmp_pair_savage;
		algSAVAGE.pull(ind_slot, reward, beat, cmp_pair_savage);
		if (ind_slot > 0) {
			if (regretType_isCpldScore) {
				total_reward.savage[ind_slot] = total_reward.savage[ind_slot - 1] + (reward[cmp_pair_savage[0]] + reward[cmp_pair_savage[1]]) / 2.0;
			}
			else {
				total_reward.savage[ind_slot] = total_reward.savage[ind_slot - 1] + pref_mat[cmp_pair_savage[0]][ind_best] + pref_mat[cmp_pair_savage[1]][ind_best] - 1.0;
			}
		}
		else {
			if (regretType_isCpldScore) {
				total_reward.savage[ind_slot] = (reward[cmp_pair_savage[0]] + reward[cmp_pair_savage[1]]) / 2.0;
			}
			else {
				total_reward.savage[ind_slot] = pref_mat[cmp_pair_savage[0]][ind_best] + pref_mat[cmp_pair_savage[1]][ind_best] - 1.0;
			}
		}

		if (!isEteOnly) {
			// RUCB
			vector<int> cmp_pair_rucb;
			algRUCB.pull(ind_slot, reward, beat, cmp_pair_rucb);

			// RCS
			vector<int> cmp_pair_rcs;
			algRCS.pull(ind_slot, reward, beat, cmp_pair_rcs);

			// MultiSBM
			vector<int> cmp_pair_multisbm;
			algMultiSBM.pull(ind_slot, reward, beat, cmp_pair_multisbm);

			// Sparring
			vector<int> cmp_pair_sparring;
			algSparring.pull(ind_slot, reward, beat, cmp_pair_sparring);

			// CCB
			vector<int> cmp_pair_ccb;
			algCCB.pull(ind_slot, reward, beat, cmp_pair_ccb);

			// SCB
			vector<int> cmp_pair_scb;
			algSCB.pull(ind_slot, reward, beat, cmp_pair_scb);

			// RMED1
			vector<int> cmp_pair_rmed1;
			algRMED1.pull(ind_slot, reward, beat, cmp_pair_rmed1);

			// RMED2
			vector<int> cmp_pair_rmed2;
			algRMED2.pull(ind_slot, reward, beat, cmp_pair_rmed2);

			// ECW-RMED
			vector<int> cmp_pair_ecwrmed;
			algECWRMED.pull(ind_slot, reward, beat, cmp_pair_ecwrmed);

			// DTS
			vector<int> cmp_pair_dts;
			algDTS.pull(ind_slot, reward, beat, cmp_pair_dts);

			// DTS2
			vector<int> cmp_pair_dts2;
			algDTS2.pull(ind_slot, reward, beat, cmp_pair_dts2);


			if (ind_slot > 0) {
				if (regretType_isCpldScore) {
					total_reward.rucb[ind_slot] = total_reward.rucb[ind_slot - 1] + (reward[cmp_pair_rucb[0]] + reward[cmp_pair_rucb[1]]) / 2.0;
					total_reward.rcs[ind_slot] = total_reward.rcs[ind_slot - 1] + (reward[cmp_pair_rcs[0]] + reward[cmp_pair_rcs[1]]) / 2.0;
					total_reward.multisbm[ind_slot] = total_reward.multisbm[ind_slot - 1] + (reward[cmp_pair_multisbm[0]] + reward[cmp_pair_multisbm[1]]) / 2.0;
					total_reward.sparring[ind_slot] = total_reward.sparring[ind_slot - 1] + (reward[cmp_pair_sparring[0]] + reward[cmp_pair_sparring[1]]) / 2.0;
					total_reward.ccb[ind_slot] = total_reward.ccb[ind_slot - 1] + (reward[cmp_pair_ccb[0]] + reward[cmp_pair_ccb[1]]) / 2.0;
					total_reward.scb[ind_slot] = total_reward.scb[ind_slot - 1] + (reward[cmp_pair_scb[0]] + reward[cmp_pair_scb[1]]) / 2.0;
					total_reward.rmed1[ind_slot] = total_reward.rmed1[ind_slot - 1] + (reward[cmp_pair_rmed1[0]] + reward[cmp_pair_rmed1[1]]) / 2.0;
					total_reward.rmed2[ind_slot] = total_reward.rmed2[ind_slot - 1] + (reward[cmp_pair_rmed2[0]] + reward[cmp_pair_rmed2[1]]) / 2.0;
					total_reward.ecw_rmed[ind_slot] = total_reward.ecw_rmed[ind_slot - 1] + (reward[cmp_pair_ecwrmed[0]] + reward[cmp_pair_ecwrmed[1]]) / 2.0;
					total_reward.d_ts[ind_slot] = total_reward.d_ts[ind_slot - 1] + (reward[cmp_pair_dts[0]] + reward[cmp_pair_dts[1]]) / 2.0;
					total_reward.d_ts2[ind_slot] = total_reward.d_ts2[ind_slot - 1] + (reward[cmp_pair_dts2[0]] + reward[cmp_pair_dts2[1]]) / 2.0;
				}
				else {
					total_reward.rucb[ind_slot] = total_reward.rucb[ind_slot - 1] + pref_mat[cmp_pair_rucb[0]][ind_best] + pref_mat[cmp_pair_rucb[1]][ind_best] - 1.0;
					total_reward.rcs[ind_slot] = total_reward.rcs[ind_slot - 1] + pref_mat[cmp_pair_rcs[0]][ind_best] + pref_mat[cmp_pair_rcs[1]][ind_best] - 1.0;
					total_reward.multisbm[ind_slot] = total_reward.multisbm[ind_slot - 1] + pref_mat[cmp_pair_multisbm[0]][ind_best] + pref_mat[cmp_pair_multisbm[1]][ind_best] - 1.0;
					total_reward.sparring[ind_slot] = total_reward.sparring[ind_slot - 1] + pref_mat[cmp_pair_sparring[0]][ind_best] + pref_mat[cmp_pair_sparring[1]][ind_best] - 1.0;
					total_reward.ccb[ind_slot] = total_reward.ccb[ind_slot - 1] + pref_mat[cmp_pair_ccb[0]][ind_best] + pref_mat[cmp_pair_ccb[1]][ind_best] - 1.0;
					total_reward.scb[ind_slot] = total_reward.scb[ind_slot - 1] + pref_mat[cmp_pair_scb[0]][ind_best] + pref_mat[cmp_pair_scb[1]][ind_best] - 1.0;
					total_reward.rmed1[ind_slot] = total_reward.rmed1[ind_slot - 1] + pref_mat[cmp_pair_rmed1[0]][ind_best] + pref_mat[cmp_pair_rmed1[1]][ind_best] - 1.0;
					total_reward.rmed2[ind_slot] = total_reward.rmed2[ind_slot - 1] + pref_mat[cmp_pair_rmed2[0]][ind_best] + pref_mat[cmp_pair_rmed2[1]][ind_best] - 1.0;
					total_reward.ecw_rmed[ind_slot] = total_reward.ecw_rmed[ind_slot - 1] + pref_mat[cmp_pair_ecwrmed[0]][ind_best] + pref_mat[cmp_pair_ecwrmed[1]][ind_best] - 1.0;
					total_reward.d_ts[ind_slot] = total_reward.d_ts[ind_slot - 1] + pref_mat[cmp_pair_dts[0]][ind_best] + pref_mat[cmp_pair_dts[1]][ind_best] - 1.0;
					total_reward.d_ts2[ind_slot] = total_reward.d_ts2[ind_slot - 1] + pref_mat[cmp_pair_dts2[0]][ind_best] + pref_mat[cmp_pair_dts2[1]][ind_best] - 1.0;
				}
			}
			else {
				if (regretType_isCpldScore) {
					total_reward.rucb[ind_slot] = (reward[cmp_pair_rucb[0]] + reward[cmp_pair_rucb[1]]) / 2.0;
					total_reward.rcs[ind_slot] = (reward[cmp_pair_rcs[0]] + reward[cmp_pair_rcs[1]]) / 2.0;
					total_reward.multisbm[ind_slot] = (reward[cmp_pair_multisbm[0]] + reward[cmp_pair_multisbm[1]]) / 2.0;
					total_reward.sparring[ind_slot] = (reward[cmp_pair_sparring[0]] + reward[cmp_pair_sparring[1]]) / 2.0;
					total_reward.ccb[ind_slot] = (reward[cmp_pair_ccb[0]] + reward[cmp_pair_ccb[1]]) / 2.0;
					total_reward.scb[ind_slot] = (reward[cmp_pair_scb[0]] + reward[cmp_pair_scb[1]]) / 2.0;
					total_reward.rmed1[ind_slot] = (reward[cmp_pair_rmed1[0]] + reward[cmp_pair_rmed1[1]]) / 2.0;
					total_reward.rmed2[ind_slot] = (reward[cmp_pair_rmed2[0]] + reward[cmp_pair_rmed2[1]]) / 2.0;
					total_reward.ecw_rmed[ind_slot] = (reward[cmp_pair_ecwrmed[0]] + reward[cmp_pair_ecwrmed[1]]) / 2.0;
					total_reward.d_ts[ind_slot] = (reward[cmp_pair_dts[0]] + reward[cmp_pair_dts[1]]) / 2.0;
					total_reward.d_ts2[ind_slot] = (reward[cmp_pair_dts2[0]] + reward[cmp_pair_dts2[1]]) / 2.0;
				}
				else {
					total_reward.rucb[ind_slot] = pref_mat[cmp_pair_rucb[0]][ind_best] + pref_mat[cmp_pair_rucb[1]][ind_best] - 1.0;
					total_reward.rcs[ind_slot] = pref_mat[cmp_pair_rcs[0]][ind_best] + pref_mat[cmp_pair_rcs[1]][ind_best] - 1.0;
					total_reward.multisbm[ind_slot] = pref_mat[cmp_pair_multisbm[0]][ind_best] + pref_mat[cmp_pair_multisbm[1]][ind_best] - 1.0;
					total_reward.sparring[ind_slot] = pref_mat[cmp_pair_sparring[0]][ind_best] + pref_mat[cmp_pair_sparring[1]][ind_best] - 1.0;
					total_reward.ccb[ind_slot] = pref_mat[cmp_pair_ccb[0]][ind_best] + pref_mat[cmp_pair_ccb[1]][ind_best] - 1.0;
					total_reward.scb[ind_slot] = pref_mat[cmp_pair_scb[0]][ind_best] + pref_mat[cmp_pair_scb[1]][ind_best] - 1.0;
					total_reward.rmed1[ind_slot] = pref_mat[cmp_pair_rmed1[0]][ind_best] + pref_mat[cmp_pair_rmed1[1]][ind_best] - 1.0;
					total_reward.rmed2[ind_slot] = pref_mat[cmp_pair_rmed2[0]][ind_best] + pref_mat[cmp_pair_rmed2[1]][ind_best] - 1.0;
					total_reward.ecw_rmed[ind_slot] = pref_mat[cmp_pair_ecwrmed[0]][ind_best] + pref_mat[cmp_pair_ecwrmed[1]][ind_best] - 1.0;
					total_reward.d_ts[ind_slot] = pref_mat[cmp_pair_dts[0]][ind_best] + pref_mat[cmp_pair_dts[1]][ind_best] - 1.0;
					total_reward.d_ts2[ind_slot] = pref_mat[cmp_pair_dts2[0]][ind_best] + pref_mat[cmp_pair_dts2[1]][ind_best] - 1.0;
				}
			}
		}
	}
	return total_reward;
}