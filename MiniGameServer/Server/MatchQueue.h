#pragma once
#include "WatingPlayerData.h"
#include "Deviset.h"

class PlayerSession;

class MatchQueue {
public:
	void Push(WatingPlayerData&& newPlayer);
	void FlushTempQueueAndSort();
	void RemoveInvalidPlayer();

	//�� �Լ��� ����Ǳ����, ����� ���� �ð��� �ɸ���.
	//���� worker thread���� ���� MatchQueue�� �� �Լ��� ���� �������� ��, ������·� ���� ��ȿ���� �߻��� ���ɼ��� �����ϴ�.
	//���� ��ġ����ŷ�� ���, ������ worker thread���� ������� ���� ���������� �����ϵ��� �Ѵ�.
	vector<vector<WatingPlayerData>> SearchMatchGroups();

private:
	mutex _TQlock;
	vector<WatingPlayerData> _searchQueue;
	vector<bool> _selectedChecks;
	vector<int32_t> _selectedPlayerIdxs;
	priority_queue<Deviset> _pq;
	int32_t _allowDevi = 50;
	vector<WatingPlayerData> _tempQueue;
	mutex _SQlock;
	int32_t _gameNum = 0;
	int32_t _quota = 4;
};

