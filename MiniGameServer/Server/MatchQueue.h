#pragma once
#include "WatingPlayerData.h"
#include "Deviset.h"

class PlayerSession;

class MatchQueue {
public:
	MatchQueue(GameType gt, int32_t quota) : _gameType(gt), _quota(quota) { }

	void Push(WatingPlayerData newPlayer);
	void Push(const vector<WatingPlayerData>& pdv);
	void FlushTempQueueAndSort();
	void RemoveInvalidPlayer();

	//�� �Լ��� ����Ǳ����, ����� ���� �ð��� �ɸ���.
	//���� worker thread���� ���� MatchQueue�� �� �Լ��� ���� �������� ��, ������·� ���� ��ȿ���� �߻��� ���ɼ��� �����ϴ�.
	//���� ��ġ����ŷ�� ���, ������ worker thread���� ������� ���� ���������� �����ϵ��� �Ѵ�.
	vector<vector<WatingPlayerData>> SearchMatchGroups();

private:
	mutex _TQlock;
	vector<WatingPlayerData> _tempQueue;

	vector<WatingPlayerData> _searchQueue;
	mutex _SQlock;

	vector<bool> _selectedChecks;
	vector<int32_t> _selectedPlayerIdxs;
	priority_queue<Deviset> _pq;
	int32_t _allowDevi = 50;
	GameType _gameType = GameType::Undefined;
	int32_t _quota;
};

