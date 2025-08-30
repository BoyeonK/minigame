#pragma once

struct Deviset {
	Deviset(double devi, int32_t idx) : _devi(devi), _idx(idx) {}

	double _devi;
	int32_t _idx;

	bool operator < (const Deviset& other) const {
		return this->_devi > other._devi;
	}
};

