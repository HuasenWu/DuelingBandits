#pragma once
#include "stdafx.h"

#include "DuelingBandit.h"

class CResults
{
public:
	vector<double> oracle;
	vector<double> btm;
	vector<double> savage;
	vector<double> rucb;
	vector<double> rcs;
	vector<double> multisbm;
	vector<double> sparring;
	vector<double> ccb;
	vector<double> scb;
	vector<double> rmed1;
	vector<double> rmed2;
	vector<double> ecw_rmed;
	vector<double> d_ts;
	vector<double> d_ts2;


public:
	CResults();
	~CResults();
public:
	void init(int num_slots) {
		for (int i = 0; i < num_slots; i++) {
			oracle.push_back(0.0);
			btm.push_back(0.0);
			savage.push_back(0.0);
			rucb.push_back(0.0);
			rcs.push_back(0.0);
			multisbm.push_back(0.0);
			sparring.push_back(0.0);
			ccb.push_back(0.0);
			scb.push_back(0.0);
			rmed1.push_back(0.0);
			rmed2.push_back(0.0);
			ecw_rmed.push_back(0.0);
			d_ts.push_back(0.0);
			d_ts2.push_back(0.0);
		}
	};
	void add(int num_slots, CResults temp) {
		for (int i = 0; i < num_slots; i++) {
			oracle[i] += temp.oracle[i];
			btm[i] += temp.btm[i];
			savage[i] += temp.savage[i];
			rucb[i] += temp.rucb[i];
			rcs[i] += temp.rcs[i];
			multisbm[i] += temp.multisbm[i];
			sparring[i] += temp.sparring[i];
			ccb[i] += temp.ccb[i];
			scb[i] += temp.scb[i];
			rmed1[i] += temp.rmed1[i];
			rmed2[i] += temp.rmed2[i];
			ecw_rmed[i] += temp.ecw_rmed[i];
			d_ts[i] += temp.d_ts[i];
			d_ts2[i] += temp.d_ts2[i];
		}
	};

	void average(int num_slots, int num_sims) {
		for (int i = 0; i < num_slots; i++) {
			oracle[i] /= num_sims;
			btm[i] /= num_sims;
			savage[i] /= num_sims;
			rucb[i] /= num_sims;
			multisbm[i] /= num_sims;
			sparring[i] /= num_sims;
			rcs[i] /= num_sims;
			ccb[i] /= num_sims;
			scb[i] /= num_sims;
			rmed1[i] /= num_sims;
			rmed2[i] /= num_sims;
			ecw_rmed[i] /= num_sims;
			d_ts[i] /= num_sims;
			d_ts2[i] /= num_sims;
		}
	}
};

