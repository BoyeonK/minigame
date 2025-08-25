#include "pch.h"
#include "MatchQueue.h"

void MatchQueue::Push(WatingPlayerData newPlayer){
	lock_guard<mutex> lock(_TQlock);
	_tempQueue.push_back(newPlayer);
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

    {
        lock_guard<mutex> lock(_SQlock);
        _searchQueue.insert(
            _searchQueue.end(),
            make_move_iterator(playersToMove.begin()),
            make_move_iterator(playersToMove.end())
        );

        sort(_searchQueue.begin(), _searchQueue.end(),
            [](WatingPlayerData& a, WatingPlayerData& b) {
                return a._elo < b._elo;
            }
        );
    }
}

void MatchQueue::RemoveInvalidPlayer() {
    auto new_end = remove_if(_searchQueue.begin(), _searchQueue.end(),
        [](WatingPlayerData& player) {
            return !player.IsValidPlayer();
        });

    _searchQueue.erase(new_end, _searchQueue.end());
}

void MatchQueue::SearchMin() {
    //1. 유효하지 않은 그룹 제거
    RemoveInvalidPlayer();
    int32_t mxmidx = _searchQueue.size() - _quota;
    if (mxmidx < 0)
        return;

    //2. 변수 초기화
    long long sum = 0, sqsum = 0, newElo = 0, oldElo = 0;
    double var = 0, devi = 0, mean = 0;
    _selectedChecks = vector<bool>(_searchQueue.size());
    _selectedPlayerIdxs.clear();

    //3. 0번째 index에 대한 분산 계산, 조건에 맞으면 pq에 push
    for (int i = 0; i < _quota; i++) {
        newElo = _searchQueue[i]._elo;
        sum += newElo;
        sqsum += newElo * newElo;
    }
    mean = static_cast<double>(sum) / _quota;
    var = static_cast<double>(sqsum) / _quota - pow(mean, 2);
    devi = sqrt(var);
    if (devi < _allowDevi) {
        _pq.push(Deviset(devi, 0));
    }

    //4. 이후의 분산 계산 및 조건에 맞으면 pq에 push
    for (int i = 1; i <= mxmidx; i++) {
        oldElo = _searchQueue[i - 1]._elo;
        newElo = _searchQueue[i + _quota]._elo;
        sum += (newElo - oldElo);
        sqsum += (newElo * newElo - oldElo * oldElo);
        mean = static_cast<double>(sum) / _quota;
        var = static_cast<double>(sqsum) / _quota - pow(mean, 2);
        devi = sqrt(var);
        if (devi < _allowDevi) {
            _pq.push(Deviset(devi, i));
        }
    }

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

    //_selectedPlayerIdxs 를 바탕으로 매치 진행
}



