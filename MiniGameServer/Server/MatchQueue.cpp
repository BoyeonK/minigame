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
    cout << "�� ��⿭�� ���� ����" << endl;
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

    //1. ��ȿ���� ���� �׷� ����
    RemoveInvalidPlayer();
    int32_t mxmidx = _searchQueue.size() - _quota;
    if (mxmidx < 0)
        return matchGruops;

    //2. ���� �ʱ�ȭ
    long long sum = 0, sqsum = 0, newElo = 0, oldElo = 0;
    double var = 0, devi = 0, mean = 0;
    _selectedChecks = vector<bool>(_searchQueue.size());
    _selectedPlayerIdxs.clear();

    //3. 0��° index�� ���� �л� ���, ���ǿ� ������ pq�� push
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

    //4. ������ �л� ��� �� ���ǿ� ������ pq�� push
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

    //5. �л��� _allowDevi ������ ���� �߿�, ��ġ�� �ο��� ������ idx�� ����.
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

    //6. _selectedPlayerIdxs �� �������� ��ġ �׷���� ����.
    for (auto& idx : _selectedPlayerIdxs) {
        vector<WatingPlayerData> matchGroup;
        matchGroup.reserve(_quota);
        for (int i = 0; i < _quota; i++)
            matchGroup.push_back(_searchQueue[idx + i]);

        matchGruops.push_back(move(matchGroup));
    }

    //7. group���� ������ ��Ī�� ���۵� �÷��̾���� Queue���� ����
    int it_idx = 0;
    auto new_end = std::remove_if(_searchQueue.begin(), _searchQueue.end(),
        [this, &it_idx](const WatingPlayerData& player) mutable {
            return _selectedChecks[it_idx++];
        });
    _searchQueue.erase(new_end, _searchQueue.end());

    return matchGruops;
}

