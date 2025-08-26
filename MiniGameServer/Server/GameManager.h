#pragma once
#include <shared_mutex>
#include "MatchQueue.h"
#include "GameRoom.h"
#include "PlayerSession.h"

class GameManager {
protected:
	//TODO : psv안에 모든 친구들이 유효한 친구들인지 확인.
	//유효하면 해당 vector로서 MakeRoom을 실행.
	virtual void SearchMatchGroup() = 0;
	virtual void MatchMake(vector<WatingPlayerData> pdv) = 0;
	virtual void MakeRoom(vector<WatingPlayerData> pdv) = 0;
	virtual void Update() = 0;
	void AddRoom(shared_ptr<GameRoom> room) {
		unique_lock<shared_mutex> lock(_roomsLock);
		_rooms.push_back(room);
	}

	vector<shared_ptr<GameRoom>> _rooms;
	shared_mutex _roomsLock;
};

class PingPongManager : public GameManager {
public:
	void SearchMatchGroup() override {
        //1. 유효하지 않은 그룹 제거
        _matchQueue.RemoveInvalidPlayer();
        int32_t mxmidx = _matchQueue.searchQueue.size() - _quota;
        if (mxmidx < 0)
            return;

        //2. 변수 초기화
        long long sum = 0, sqsum = 0, newElo = 0, oldElo = 0;
        double var = 0, devi = 0, mean = 0;
        _matchQueue.selectedChecks = vector<bool>(_matchQueue.searchQueue.size());
        _matchQueue.selectedPlayerIdxs.clear();

        //3. 0번째 index에 대한 분산 계산, 조건에 맞으면 pq에 push
        for (int i = 0; i < _quota; i++) {
            newElo = _matchQueue.searchQueue[i]._elo;
            sum += newElo;
            sqsum += newElo * newElo;
        }
        mean = static_cast<double>(sum) / _quota;
        var = static_cast<double>(sqsum) / _quota - pow(mean, 2);
        devi = sqrt(var);
        if (devi < _matchQueue.allowDevi) {
            _matchQueue.pq.push(Deviset(devi, 0));
        }

        //4. 이후의 분산 계산 및 조건에 맞으면 pq에 push
        for (int i = 1; i <= mxmidx; i++) {
            oldElo = _matchQueue.searchQueue[i - 1]._elo;
            newElo = _matchQueue.searchQueue[i + _quota]._elo;
            sum += (newElo - oldElo);
            sqsum += (newElo * newElo - oldElo * oldElo);
            mean = static_cast<double>(sum) / _quota;
            var = static_cast<double>(sqsum) / _quota - pow(mean, 2);
            devi = sqrt(var);
            if (devi < _matchQueue.allowDevi) {
                _matchQueue.pq.push(Deviset(devi, i));
            }
        }

        while (!_matchQueue.pq.empty()) {
            Deviset Ds = _matchQueue.pq.top();
            _matchQueue.pq.pop();
            int idx = Ds._idx;
            bool isOverlapped = false;
            for (int i = idx; i < idx + _quota; i++) {
                if (_matchQueue.selectedChecks[i]) {
                    isOverlapped = true;
                    break;
                }
            }

            if (isOverlapped)
                continue;

            _matchQueue.selectedPlayerIdxs.push_back(Ds._idx);
            for (int i = idx; i < idx + _quota; i++) {
                _matchQueue.selectedChecks[i] = true;
            }
        }

        //5. _selectedPlayerIdxs 를 바탕으로 매치 진행
        for (auto& idx : _matchQueue.selectedPlayerIdxs) {
            vector<WatingPlayerData> selectedSet;
            for (int i = 0; i < _quota; i++)
                selectedSet.push_back(_matchQueue.searchQueue[idx + i]);

        }
	}

	void MatchMake(vector<WatingPlayerData> pdv) override {
		bool isReady = true;
		for (auto& pd : pdv) {
			shared_ptr<PlayerSession> playerSessionRef = pd._playerSessionRef.lock();
			if (playerSessionRef == nullptr || playerSessionRef->GetMatchingState() != GameType::PingPong) {
				isReady = false;
				break;
			}
		}

		if (isReady) {

		}
			//MakeRoom(psv);
	}

	void MakeRoom(vector<WatingPlayerData> pdv) override {
		shared_ptr<PingPongGameRoom> newRoom = { objectPool<PingPongGameRoom>::alloc(), objectPool<PingPongGameRoom>::dealloc };
		newRoom->DoAsync(true, &PingPongGameRoom::Start, pdv);
	}

	void Update() override {
		
	}
	
	void StartGame() { }
	
private:
	GameType _ty = GameType::PingPong;
	MatchQueue _matchQueue;
    int32_t _quota = 4;
};
