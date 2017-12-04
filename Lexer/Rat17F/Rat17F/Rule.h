
#ifndef Rule_h
#define Rule_h
#include "Symbol.h"

class Rule {
public:
	int ruleID;
	int size;
	Symbol lhs;
	Symbol rhs[6];


	Rule() {

	}
	Rule(int ruleID_, Symbol lhs_, Symbol rhs_[], int size_) {
		ruleID = ruleID_;
		lhs = lhs_;
		size = size_;
		for (int i = 0; i < size; i++) {
			rhs[i] = rhs_[i];
		}
	}
	~Rule() {

	}
	void print() {

		cout << lhs << "=> ";

		for (int i = 0; i < size; i++) {
			cout << rhs[i];
		}
		cout << "\n";
	}
};

#endif /* Rule_h */