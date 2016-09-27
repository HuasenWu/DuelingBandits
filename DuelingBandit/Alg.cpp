#include "stdafx.h"
#include "Alg.h"


CAlg::CAlg()
{
}


CAlg::~CAlg()
{
}

void CAlg::init(int num_slots, int num_arms, struConfig config) {

}
int CAlg::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	return 0;
}

double CAlg::klDiv(double p, double q) {
	p = max(p, 0.0000001);
	q = max(q, 0.0000001);
	p = min(p, 0.9999999);
	q = min(q, 0.9999999);
	return p * log(p / q) + (1.0 - p) * log((1.0 - p) / (1.0 - q));
}

void CAlg::normalize(vector<double>& prob) {
	double sum = 0.0;
	for (int i = 0; i < (int)prob.size(); i++) {
		sum += prob[i];
	}
	for (int i = 0; i < (int)prob.size(); i++) {
		prob[i] /= sum;
	}
}


