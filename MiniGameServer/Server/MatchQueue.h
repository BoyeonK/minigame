#pragma once
#include "WatingPlayerData.h"
#include "Deviset.h"

class PlayerSession;

class MatchQueue {
public:
	void Push(WatingPlayerData&& newPlayer);
	void FlushTempQueueAndSort();
	void RemoveInvalidPlayer();

	//이 함수가 실행되기까지, 상당히 많은 시간이 걸린다.
	//여러 worker thread에서 같은 MatchQueue의 이 함수를 돌려 실행했을 때, 경쟁상태로 인한 비효율이 발생할 가능성이 농후하다.
	//따라서 매치메이킹의 경우, 별도의 worker thread에서 경쟁상태 없이 독립적으로 실행하도록 한다.
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

