#include "pch.h"
#include "MatchQueue.h"

void MatchQueue::Push(WatingPlayerData newPlayer){
	lock_guard<mutex> lock(_TQlock);
	_tempQueue.push_back(move(newPlayer));
}

void MatchQueue::Push(const vector<WatingPlayerData>& pdv) {
    lock_guard<mutex> lock(_TQlock);
    for (auto& pd : pdv) {
        _tempQueue.push_back(pd);
    }
}

void MatchQueue::FlushTempQueueAndSort() {
    vector<WatingPlayerData> playersToMove;
    {
        lock_guard<mutex> lock(_TQlock);
        _tempQueue.swap(playersToMove);
    }

    if (playersToMove.empty()) {
        return;
    }

#ifdef _DEBUG
    cout << "찐 대기열에 무언가 진입" << endl;
#endif

    {
        lock_guard<mutex> lock(_SQlock);
        _searchQueue.insert(
            _searchQueue.end(),
            make_move_iterator(playersToMove.begin()),
            make_move_iterator(playersToMove.end())
        );

        sort(_searchQueue.begin(), _searchQueue.end(),
            [](WatingPlayerData& a, WatingPlayerData& b) {
                return a.elo < b.elo;
            }
        );
    }
}

void MatchQueue::RemoveInvalidPlayer() {
    GameType validGameType = _gameType;
    auto new_end = remove_if(_searchQueue.begin(), _searchQueue.end(),
        [validGameType](WatingPlayerData& player) {
            return !player.IsValidPlayer(validGameType);
        });

    _searchQueue.erase(new_end, _searchQueue.end());
}

vector<vector<WatingPlayerData>> MatchQueue::SearchMatchGroups() {
    vector<vector<WatingPlayerData>> matchGruops;

    //1. 유효하지 않은 그룹 제거
    RemoveInvalidPlayer();
    int32_t mxmidx = _searchQueue.size() - _quota;
    if (mxmidx < 0)
        return matchGruops;

    //2. 변수 초기화
    long long sum = 0, sqsum = 0, newElo = 0, oldElo = 0;
    double var = 0, devi = 0, mean = 0;
    _selectedChecks = vector<bool>(_searchQueue.size());
    _selectedPlayerIdxs.clear();

	//3. 0번째 index부터 _quota번째 플레이어까지의 분산 계산.
    for (int i = 0; i < _quota; i++) {
        newElo = _searchQueue[i].elo;
        sum += newElo;
        sqsum += newElo * newElo;
    }
    mean = static_cast<double>(sum) / _quota;
    var = static_cast<double>(sqsum) / _quota - pow(mean, 2);
    devi = sqrt(var);
    if (devi < _allowDevi) {
        _pq.push(Deviset(devi, 0));
    }

    //4. 이후의 분산 계산 및 조건에 맞으면 pq에 push.
    for (int i = 1; i <= mxmidx; i++) {
        oldElo = _searchQueue[i - 1].elo;
        newElo = _searchQueue[i + _quota].elo;
        sum += (newElo - oldElo);
        sqsum += (newElo * newElo - oldElo * oldElo);
        mean = static_cast<double>(sum) / _quota;
        var = static_cast<double>(sqsum) / _quota - pow(mean, 2);
        devi = sqrt(var);
        if (devi < _allowDevi) {
            _pq.push(Deviset(devi, i));
        }
    }

    //5. 분산이 _allowDevi 이하인 조합 중에, 겹치는 인원이 없도록 idx를 선택.
    while (!_pq.empty()) {
        Deviset Ds = _pq.top();
        _pq.pop();
        int idx = Ds._idx;
        bool isOverlapped = false;
        for (int i = idx; i < idx + _quota; i++) {
            if (_selectedChecks[i]) {
                isOverlapped = true;
                break;
            }
        }

        if (isOverlapped)
            continue;

        _selectedPlayerIdxs.push_back(Ds._idx);
        for (int i = idx; i < idx + _quota; i++) {
            _selectedChecks[i] = true;
        }
    }

	//6. _selectedPlayerIdxs 를 바탕으로 매칭을 진행할 플레이어 그룹의 조합(matchGruops)을 채움.
    for (auto& idx : _selectedPlayerIdxs) {
        vector<WatingPlayerData> matchGroup;
        matchGroup.reserve(_quota);
        for (int i = 0; i < _quota; i++)
            matchGroup.push_back(_searchQueue[idx + i]);

        matchGruops.push_back(move(matchGroup));
    }

	//7. matchGruops에 포함된 플레이어들을 _searchQueue에서 제거하고 matchGroups 리턴.
    int it_idx = 0;
    auto new_end = std::remove_if(_searchQueue.begin(), _searchQueue.end(),
        [this, &it_idx](const WatingPlayerData& player) mutable {
            return _selectedChecks[it_idx++];
        });
    _searchQueue.erase(new_end, _searchQueue.end());

    return matchGruops;
}

