// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// DuelingBandit.cpp : Main file for the simulations.
//

#include "stdafx.h"
#include <random>

#include "DuelingBandit.h"
#include "Results.h"
#include "MAB.h"

void getPrefMat(int input_type, string inputFilePath, int& num_arms, vector<vector<double>>& pref_mat);

int main()
{
	srand((unsigned int)time(NULL));

	// Simulation setting

	// directory and file names
	// data set
	string strSimConfig = "MSLR_Informational_5_Condorcet";
	// input directory
	string strInputDir = "E:\\DuelingBanditSim_GitHub\\DuelingBandit\\data\\";
	// ouput directory
	string strOutputDir = "E:\\DuelingBanditSim_GitHub\\DuelingBandit\\results\\";
	//// input directory
	//string strInputDir = "data\\";
	//// ouput directory
	//string strOutputDir = "results\\";

	// simulation config
	int num_sims = 10;
	int num_slots = 50000;

	bool seperateForETE = true; // seperate simulations for explore-then-exploit algorithms (BTM & SAVAGE)
	int num_slot_sacle_factor = 10;
	vector<int> num_slots_ETE;
	int num_ETE_Algs = 2;
	vector<vector<double>> total_reward_ETE;
	if (seperateForETE) {
		int cur_num = num_slot_sacle_factor;
		while (cur_num <= num_slots) {
			num_slots_ETE.push_back(cur_num);
			total_reward_ETE.push_back(vector<double>(num_ETE_Algs + 1, 0.0));
			num_slots_ETE.push_back(cur_num * 3);
			total_reward_ETE.push_back(vector<double>(num_ETE_Algs + 1, 0.0));
			cur_num *= num_slot_sacle_factor;
		}
	}

	// preference matrix
	int num_arms = 32;     // default number of arms
	int input_type = INPUT_TYPE_FILE;
	//int input_type = INPUT_TYPE_FILE;

	// Policy parameters
	struConfig config;
	config.regret_type = REGRET_TYPE_ALL_CPLD_SCORE;
	config.btm_gamma = 1.3;
	config.ucb_scale_factor = 0.51;
	config.klucb_ctrl_factor = 3.0;
	config.klucb_eps = 0.000001;
	config.sbm_scale_factor = 3.0;
	config.rmed2_alpha = 3.0;
	config.ecwrmed_alpha = 3.0;
	config.ecwrmed_beta = 0.01;

	// input file	
	string inputFileName = strSimConfig + ".txt";
	string inputFilePath = strInputDir + inputFileName;

	// output files
	ofstream fout_data, fout_results, fout_results_EteOnly;
	string settingFile = strOutputDir + strSimConfig + "_results" + ".txt";
	string names = strOutputDir + strSimConfig + "_results" + ".mat";
	fout_data.open(settingFile.c_str());
	fout_results.open(names.c_str());
	if (!fout_data.is_open() || !fout_results.is_open())
	{
		cout << "DuelingBandit.cpp: Failed to create the output file!" << endl;
		exit(ERROR_OPEN_OUPUT_FILE);
	}

	if (seperateForETE) {
		string path = strOutputDir + strSimConfig + "_results_EteOnly" + ".mat";
		fout_results_EteOnly.open(path);
		if (!fout_results_EteOnly.is_open())
		{
			cout << "DuelingBandit.cpp: Failed to create the output file!" << endl;
			exit(ERROR_OPEN_OUPUT_FILE);
		}
	}

	vector<vector<double>> pref_mat;
	getPrefMat(input_type, inputFilePath, num_arms, pref_mat);
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j < num_arms; j++) {
			cout << pref_mat[i][j] << " ";
		}
		cout << endl;
	}


	// MAB model
	CMAB objMAB = CMAB(false);
	CResults total_reward;
	total_reward.init(num_slots);

	// Runsim
	for (int ind_sim = 0; ind_sim < num_sims; ind_sim++) {
		cout << "Sim # " << ind_sim << endl;
		//shuffle the pref_mat
		vector<int> ind_arms;
		for (int i = 0; i < num_arms; i++) {
			ind_arms.push_back(i);
		}
		random_shuffle(ind_arms.begin(), ind_arms.end());
		vector<vector<double>> cur_pref_mat;
		for (int i = 0; i < num_arms; i++) {
			vector<double> pref_vec;
			for (int j = 0; j < num_arms; j++) {
				pref_vec.push_back(pref_mat[ind_arms[i]][ind_arms[j]]);
			}
			cur_pref_mat.push_back(pref_vec);
		}
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j < num_arms; j++) {
				cout << cur_pref_mat[i][j] << " ";
			}
			cout << endl;
		}
		CResults total_reward_temp = objMAB.singleRun(num_slots, cur_pref_mat, config);
		total_reward.add(num_slots, total_reward_temp);

		// Seperate for the explore-then-exploit policies: BTM & SAVAGE
		if (seperateForETE) {
			for (int i = 0; i < (int)num_slots_ETE.size(); i++) {
				int cur_num_slots = num_slots_ETE[i];
				CMAB objMAB_ETE = CMAB(seperateForETE);
				CResults total_reward_temp = objMAB_ETE.singleRun(cur_num_slots, cur_pref_mat, config);
				total_reward_ETE[i][0] += total_reward_temp.oracle[cur_num_slots - 1];
				total_reward_ETE[i][1] += total_reward_temp.btm[cur_num_slots - 1];
				total_reward_ETE[i][2] += total_reward_temp.savage[cur_num_slots - 1];
			}
		}
	}
	total_reward.average(num_slots, num_sims);

	//total_reward.record(fout, num_slots);
	for (int i = 0; i < num_slots; i++) {
		fout_results << i << " " << total_reward.oracle[i] << " "
			<< total_reward.btm[i] << " " << total_reward.savage[i] << " "
			<< total_reward.rucb[i] << " " << total_reward.rcs[i] << " "
			<< total_reward.multisbm[i] << " " << total_reward.sparring[i] << " "
			<< total_reward.ccb[i] << " " << total_reward.scb[i] << " "
			<< total_reward.rmed1[i] << " " << total_reward.rmed2[i] << " "
			<< total_reward.ecw_rmed[i] << " " 
			<< total_reward.d_ts[i] << " " << total_reward.d_ts2[i] << endl;
	}


	fout_data.close();
	fout_results.close();

	if (seperateForETE) {
		// average the results and write to the output file
		for (int i = 0; i < (int)num_slots_ETE.size(); i++) {
			fout_results_EteOnly << num_slots_ETE[i] << " "
				<< total_reward_ETE[i][0] / num_sims << " "
				<< total_reward_ETE[i][1] / num_sims << " "
				<< total_reward_ETE[i][2] / num_sims << " " << endl;
		}
		fout_results_EteOnly.close();
	}

	system("PAUSE");
	return 0;
}

// generate or load the preference matrix
void getPrefMat(int input_type, string inputFilePath, int& num_arms, vector<vector<double>>& pref_mat) {
	pref_mat.clear();
	if (input_type == INPUT_TYPE_RANDOM) {
		ofstream fin_data;                      // save the pref_mat for repeated experiments
		fin_data.open(inputFilePath);
		if (!fin_data.is_open()) {
			cout << "DuelingBandit.cpp: Failed to create the input file!" << endl;
			exit(ERROR_OPEN_INPUT_FILE);
		}
		for (int i = 0; i < num_arms; i++) {
			pref_mat.push_back(vector<double>(num_arms, 0.0));
		}
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j <= i; j++) {
				if (j == i) {
					pref_mat[i][j] = 0.5;
				}
				else {
					pref_mat[i][j] = (double)rand() / RAND_MAX;
					pref_mat[j][i] = 1 - pref_mat[i][j];
				}
			}
		}
		fin_data << num_arms << endl;
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j < num_arms; j++) {
				fin_data << pref_mat[i][j] << " ";
			}
			fin_data << endl;
		}
	}
	else if (input_type == INPUT_TYPE_FILE) {
		ifstream fin_data;
		fin_data.open(inputFilePath);
		if (!fin_data.is_open()) {
			cout << "DuelingBandit.cpp: Failed to create the input file!" << endl;
			exit(ERROR_OPEN_INPUT_FILE);
		}
		fin_data >> num_arms;
		for (int i = 0; i < num_arms; i++) {
			vector<double> pref_vec;
			for (int j = 0; j < num_arms; j++) {
				double pref_prob;
				fin_data >> pref_prob;
				pref_vec.push_back(pref_prob);
			}
			pref_mat.push_back(pref_vec);
		}
	}
	// For select the arms from the MSLR dataset
	else if (input_type == INPUT_TYPE_CUSTOMIZED) {
		ifstream fin_data;
		fin_data.open(inputFilePath);
		if (!fin_data.is_open()) {
			cout << "DuelingBandit.cpp: Failed to create the input file!" << endl;
			exit(ERROR_OPEN_INPUT_FILE);
		}
		int num_arms_orig;
		fin_data >> num_arms_orig;
		vector<vector<double>> pref_mat_original;
		for (int i = 0; i < num_arms_orig; i++) {
			vector<double> pref_vec;
			for (int j = 0; j < num_arms_orig; j++) {
				double pref_prob;
				fin_data >> pref_prob;
				pref_vec.push_back(pref_prob);
			}
			pref_mat_original.push_back(pref_vec);
		}
		for (int i = 3; i < num_arms_orig - 8; i += 4) {
			vector<double> pref_vec;
			for (int j = 3; j < num_arms_orig - 8; j += 4) {
				pref_vec.push_back(pref_mat_original[i][j]);
			}
			pref_mat.push_back(pref_vec);
		}
		num_arms = (int)pref_mat.size();
		cout << "num_arms " << num_arms << endl;
		vector<double> copeland_score;
		double max_score = -1.0;
		for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
			double num_wins = 0;
			for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
				if (ind_arm1 != ind_arm2 && pref_mat[ind_arm1][ind_arm2] > 0.5) {
					num_wins += 1.0;
				}
			}
			cout << num_wins / (num_arms - 1) << " ";
			if (num_wins / (num_arms - 1) > max_score) {
				max_score = num_wins / (num_arms - 1);
			}
		}
		cout << endl;
		cout << "Max Score " << max_score << endl;
	}
	else {
		cout << "DuelingBandit.cpp: Invalid input type!" << endl;
		exit(ERROR_INPUT_TYPE);
	}
}

