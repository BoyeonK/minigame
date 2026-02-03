using Google.Protobuf;
using Google.Protobuf.Protocol;
using NUnit.Framework;
using Org.BouncyCastle.Bcpg;
using Org.BouncyCastle.Utilities;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Net;
using System.Threading;
using TMPro;
using UnityEngine;
using static Define;

public class NetworkManager {
	ServerSession _session = new ServerSession();
	private int _isTryingConnect = 0;
	private bool _isConnected = false;

    public ServerSession GetSession() {
        return _session;
    }

    public bool IsConnected() {
        return _isConnected;
    }

    public bool IsLogined() {
        if (_session.ID != 0)
            return true;
        return false;
    }

    public void Send(IMessage packet) {
        _session.Send(packet);
    }

    public void ConnectCompleted(bool result) {
        _isConnected = result;
        if (_isTryingConnect == 1)
            _isTryingConnect = 0;
        if (result) {
            OnConnectedAct.Invoke();
        }
        else {
            OnConnectedFailedAct.Invoke();
        }
    }

    public void TryConnectToServer() {
        //이미 연결되있지도 않으면서, 현재 연결 함수가 작동중이 아닌 경우.
        if (_isConnected == true || Interlocked.CompareExchange(ref _isTryingConnect, 1, 0) != 0) {
            return;
        }

        // DNS (Domain Name System)
        string host = Dns.GetHostName();
        IPHostEntry ipHost = Dns.GetHostEntry(host);

        //이걸 나중에 UI로 받아야겠음.
        IPAddress ipAddr = IPAddress.Parse(GitIgnores.sAddr);
        //IPAddress ipAddr = IPAddress.Loopback;

        //IPAddress ipAddr = Array.Find(ipHost.AddressList, a => a.AddressFamily == AddressFamily.InterNetwork);
        //IPAddress ipAddr = ipHost.AddressList[0];
        IPEndPoint endPoint = new IPEndPoint(ipAddr, 7777);
        Debug.Log($"{ipAddr}");

        Connector connector = new Connector();

        connector.Connect(endPoint, () => { return _session; }, 1);
    }

    //이 함수는, 일단 내 설계상 메인스레드에서만 호출할 예정.
    //경쟁상태를 고려하지 않고 만듬.
    //TODO : 이 함수가 서버의 Kill을 통해 실행될 수 있음.
    public void TryDisconnect() {
        if (_isConnected == true)  {
            _session.Disconnect();

            //매우 큰 깨달음. 회고할때 반드시 짚고 넘어갈 것
            //
            _session = new ServerSession();
            //

            _isConnected = false;
        }
    }

    public void Init() {
        Lobby.Init(this);
        Match.Init(this);
        Loading.Init(this);
        PingPong.Init(this);
        Mole.Init(this);
        Race.Init(this);
    }

    public NetworkLobbyManager Lobby = new NetworkLobbyManager();
    public class NetworkLobbyManager {
        private NetworkManager _netRef;
        private readonly object _recordLock = new object();
        List<int> _personalScores = new List<int>();
        List<int> _publicScores = new List<int>();
        List<string> _publicIds = new List<string>();

        public void Init(NetworkManager netRef) {
            _netRef = netRef;
        }

        public void TryLogin(string id, string password) {
            if (id == "" || password == "") {
                Managers.UI.ShowErrorUIOnlyConfirm("입력값이 잘못되었습니다.", () => { });
                return;
            }

            if (_netRef.IsConnected() && !(_netRef.IsLogined())) {
                C_Encrypted pkt = PacketMaker.MakeCLogin(_netRef.GetSession(), id, password);
                _netRef.Send(pkt);
            }
        }

        //지금은 직접 0으로 밀고 결과를 통보하지만,
        //일관성을 위해서 서버 주도적으로 바꿀 필요가 있음.
        public void TryLogout() {
            C_Encrypted pkt = PacketMaker.MakeCLogout(_netRef.GetSession());
            Managers.Network.Send(pkt);
            Managers.Network.GetSession().ID = 0;

        }

        public void TryCreateAccount(string id, string pw, string pwc) {
            if (id == "" || pw == "" || pwc == "" || pw != pwc) {
                Managers.UI.ShowErrorUIOnlyConfirm("입력값이 잘못되었습니다.", () => { });
                return;
            }

            if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
                C_Encrypted pkt = PacketMaker.MakeCCreateAccount(_netRef.GetSession(), id, pw);
                Managers.Network.Send(pkt);
            }
        }

        public void LoginCompleted(int result) {
            switch (result)
            {
                case 0:
                    OnLoginAct.Invoke();
                    break;
                case 1:
                    OnWrongIdAct.Invoke();
                    break;
                case 2:
                    OnWrongPasswordAct.Invoke();
                    break;
            }
        }

        public void TryGetPersonalRecords() {
            int dbid = _netRef.GetSession().ID;
            if (dbid == 0)
                return;

            C_RequestMyRecords pkt = new() {
                Dbid = dbid,
            };

            _netRef.Send(pkt);
        }

        public void SetMyRecords(List<int> scores) {
            lock (_recordLock) {
                _personalScores = scores;
            }

            Managers.ExecuteAtMainThread(() => {
                BaseScene scene = Managers.Scene.CurrentScene;
                if (scene == null)
                    return;

                if (scene is LoginScene loginScene)
                    loginScene.SetPersonalRecord();
            });
        }

        public void TryGetPublicRecords() {
            int dbid = _netRef.GetSession().ID;
            if (dbid == 0)
                return;

            C_RequestPublicRecords pkt = new() {
                Dbid = dbid,
            };

            _netRef.Send(pkt);
        }

        public void SetPublicRecords(List<string> Ids, List<int> scores) {
            lock (_recordLock) {
                _publicIds = Ids;
                _publicScores = scores;
            }

            Managers.ExecuteAtMainThread(() => {
                BaseScene scene = Managers.Scene.CurrentScene;
                if (scene == null)
                    return;

                if (scene is LoginScene loginScene)
                    loginScene.SetPublicRecord();
            });
        }

        public List<string> GetPublicIds() {
            List<string> ret;
            lock (_recordLock) {
                ret = new List<string>(_publicIds);
            }
            return ret;
        }

        public List<int> GetPublicRecord() {
            List<int> ret;
            lock (_recordLock) {
                ret = new List<int>(_publicScores);
            }
            return ret;
        }

        public List<int> GetPersonalRecord() {
            List<int> ret;
            lock (_recordLock) {
                ret = new List<int>(_personalScores);
            }
            return ret;
        }

        public Action OnLoginAct;
        public Action OnLogoutAct;
        public Action OnWrongIdAct;
        public Action OnWrongPasswordAct;
    }

    public NetworkMatchMaker Match = new NetworkMatchMaker();
    public class NetworkMatchMaker  {
        private NetworkManager _netRef;
        protected GameType _matchGameType = GameType.None;
        protected readonly object _matchGameTypeLock = new object();
        protected int _isMatchRequesting = 0;

        List<string> _ingamePlayerIds = new List<string>();
        public List<string> GetIngamePlayerIds() { return new List<string>(_ingamePlayerIds); }
        public int _gameId;

        public void Init(NetworkManager netRef) {
            _netRef = netRef;
        }

        public void TryMatchMake(GameType gameType) {
            lock (_matchGameTypeLock) {
                if (_isMatchRequesting == 1 || _matchGameType != 0) {
                    return;
                }

                _isMatchRequesting = 1;
            }
            C_Encrypted pkt = PacketMaker.MakeCMatchMakeRequest(_netRef.GetSession(), (int)gameType);
            _netRef.Send(pkt);
        }

        public void ProcessMatchMake(int gameId) {
            lock (_matchGameTypeLock) {
                _matchGameType = IntToGameType(gameId);
                _isMatchRequesting = 0;
            }
            //TODO : 현재 매칭중이라는 것을 UI로 표시하고, 매칭 취소버튼을 UI로 제공
            Managers.ExecuteAtMainThread(() => {
                OnMatchmakeRequestSucceedAct.Invoke();
            });
        }

        public void ProcessMatchMake(int gameId, string err) {
            lock (_matchGameTypeLock) {
                _isMatchRequesting = 0;
            }
            Managers.ExecuteAtMainThread(() => { Managers.UI.ShowErrorUIOnlyConfirm(err); });
        }

        public void TryMatchMakeCancel() {
            int gameId = 0;
            lock (_matchGameTypeLock) {
                if (_isMatchRequesting == 1) {
                    return;
                }
                if (_matchGameType == GameType.InProcess) {
                    Managers.ExecuteAtMainThread(() => {
                        Managers.UI.ShowErrorUIOnlyConfirm("잠시 후에 다시 시도해 주세요.");
                    });
                    return;
                }
                _isMatchRequesting = 1;
                gameId = (int)_matchGameType;
            }
            C_MatchmakeCancel pkt = PacketMaker.MakeCMatchMakeCancel(_netRef.GetSession(), gameId);
            _netRef.Send(pkt);
        }

        public void ProcessMatchMakeCancel(int gameId) {
            lock (_matchGameTypeLock) {
                _isMatchRequesting = 0;
                if (_matchGameType != IntToGameType(gameId)) {
                    Managers.ExecuteAtMainThread(() => { Debug.Log($"gameId 불일치 ( {_matchGameType} != {IntToGameType(gameId)} )"); });
                    return;
                }

                _matchGameType = GameType.None;
            }
            Managers.ExecuteAtMainThread(() => {
                Debug.Log($"{gameId}번 게임 매칭 대기열 취소");
                OnMatchmakeCancelSucceedAct.Invoke();
            });
        }

        public void ProcessMatchMakeCancel(int gameId, string err) {
            lock (_matchGameTypeLock) {
                _isMatchRequesting = 0;
            }
            Managers.ExecuteAtMainThread(() => { Debug.Log($"{gameId}번 게임 매칭 대기열 취소 실패"); });
        }

        public void ResponseExcludedFromMatch() {
            Managers.ExecuteAtMainThread(() => { OnExcludedFromMatchAct.Invoke(); });
        }

        //KeepAlive handler가 전해준 id가 현재 상태와 일치하는지 확인.
        //일치한 경우, InProcess로 변경하고 true를 리턴
        public bool ResponseKeepAlive(int gameId) {
            lock (_matchGameTypeLock) {
                if ((int)_matchGameType == gameId) {
                    _matchGameType = GameType.InProcess;
                    Managers.ExecuteAtMainThread(() => { OnResponseKeepAliveAct.Invoke(); });
                    return true;
                }
            }
            return false;
        }

        public void ResponseMatchmakeCompleted(int gameId, List<string> playerIds) {
            //TODO : TestLoadingScene말고 진짜 LoadingScene쓰기 혹은 gameId마다 다른 GameScene준비하기
            _gameId = gameId;
            _ingamePlayerIds = playerIds;

            Managers.ExecuteAtMainThread(() => {
                Managers.Scene.LoadSceneWithLoadingScene(IntToGameScene(gameId), Define.Scene.LoadingScene1);
            });
        }

        public void ResetMatchState() {
            lock(_matchGameTypeLock) {
                _isMatchRequesting = 0;
                _matchGameType = GameType.None;
            }
        }

        public Action OnMatchmakeRequestSucceedAct;
        public Action OnMatchmakeCancelSucceedAct;
        public Action OnResponseKeepAliveAct;
        public Action OnExcludedFromMatchAct;
    }

    public NetworkLoadingManager Loading = new NetworkLoadingManager();
    public class NetworkLoadingManager  {
        private NetworkManager _netRef;
        public void Init(NetworkManager netRef) {
            _netRef = netRef;
        }

        public void TrySendLoadingProgressRate(float progressRate) {
            C_GameSceneLoadingProgress pkt = PacketMaker.MakeCGameSceneLoadingProgress(progressRate);
            _netRef.Send(pkt);
        }

        public void ResponseGameStarted(int gameId) {
            Managers.ExecuteAtMainThread(() => {
                OnResponseGameStartedAct.Invoke();
            });
        }

        public Action OnResponseGameStartedAct;
    }

    public NetworkPingPongManager PingPong = new NetworkPingPongManager();
    public class NetworkPingPongManager {
        private NetworkManager _netRef;
        public void Init(NetworkManager netRef) {
            _netRef = netRef;
        }

        public void ProcessPState(IMessage packet) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is PingPongScene pingPongScene) {
                S_P_State pkt = packet as S_P_State;
                if (pkt == null) return;
                int playerId = pkt.PlayerId;
                pingPongScene.SetId(playerId);
            }
        }

        public void ResponseSPRequestPlayerBarPosition(IMessage recvPkt) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is PingPongScene pingPongScene) {
                S_P_RequestPlayerBarPosition positionPkt = recvPkt as S_P_RequestPlayerBarPosition;
                if (positionPkt == null)
                    return;
                pingPongScene.RenewPlayerBarPosition(positionPkt);
                Vector3 pos = pingPongScene.GetPlayerBarPosition();
                XYZ serializedPosition = new() {
                    X = pos.x,
                    Y = pos.y,
                    Z = pos.z
                };

                C_P_ResponsePlayerBarPosition pkt = new C_P_ResponsePlayerBarPosition() {
                    Position = serializedPosition
                };

                _netRef.Send(pkt);
            }
        }

        public void ProcessSPBullet(UnityGameObject serializedBullet, float moveDirX, float moveDirZ, float speed, int lastColider) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is PingPongScene pingPongScene) {
                GameObject goBullet = null;
                PingPongBulletController bulletController = null;

                goBullet = Managers.Object.FindByObjectId(serializedBullet.ObjectId);
                if (goBullet == null) {
                    goBullet = Managers.Object.CreateObject(serializedBullet);
                    if (goBullet == null)
                        return;
                }

                bulletController = goBullet.GetComponent<PingPongBulletController>();

                if (bulletController == null)
                    return;

                Vector3 moveDir = new Vector3(moveDirX, 0, moveDirZ);
                Vector3 position = new Vector3(serializedBullet.Position.X, 0.2f, serializedBullet.Position.Z);
                bulletController.SetPositionVector(position);
                bulletController.SetMoveDir(moveDir);
                bulletController.SetSpeed(speed);
                bulletController.SetLastColider(lastColider);
            }
        }

        public void ResponseSPResult(bool isWinner, List<int> scores) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is PingPongScene pingPongScene) {
                pingPongScene.EndGame(isWinner, scores);
            }
        }

        public void ResponseSPScores(List<int> scores) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is PingPongScene pingPongScene)
                pingPongScene.RenewScores(scores);
        }
    }

    public NetworkMoleManager Mole = new NetworkMoleManager();
    public class NetworkMoleManager {
        private NetworkManager _netRef;
        float _lastFailedTick = 0.0f;

        public void Init(NetworkManager netRef) {
            _netRef = netRef;
        }

        public void ProcessSMState(int playerIdx, List<string> playerIds) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is MoleScene moleScene)
                moleScene.LoadState(playerIdx, playerIds);
        }

        public void ProcessSMSetSlotState(int slotIdx, int state, int slotControllerIdx, int point) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is MoleScene moleScene)
                moleScene.SetSlotState(slotIdx, state, slotControllerIdx, point);
        }

        public void TryHitSlot(int slotNum, float tick) {
            if (tick < _lastFailedTick + 1.0f)
                return;

            C_M_HitSlot pkt = new() { SlotIdx = slotNum };
            _netRef.Send(pkt);
        }

        public void ResponseSMHitSlot(bool isStunned) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is MoleScene moleScene) {
                if (isStunned) {
                    _lastFailedTick = Time.time;
                    moleScene.Stun();
                }
                else {

                }
            } 
        }

        public void ResponseSMRenewScores(List<int> scores) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is MoleScene moleScene) {
                moleScene.RenewScores(scores);
            }
        }

        public void ResponseSMResult(bool isWinner, List<int> scores) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is MoleScene moleScene) {
                moleScene.EndGame(isWinner, scores);
            }
        }

        public void ResponseSMReadyForStart(int countdown) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is MoleScene moleScene) {
                moleScene.CountdownBeforeStart(countdown);
            }
        }
    }

    public NetworkRaceManager Race = new NetworkRaceManager();
    public class NetworkRaceManager {
        private NetworkManager _netRef;
        private readonly object _collisionHashLock = new object();
        private HashSet<int> _collidingObjectIdxs = new HashSet<int>();

        public void Init(NetworkManager netRef) {
            _netRef = netRef;
        }

        public void ResponseSRState(List<UnityGameObject> serializedObjs) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.OffTheTempCam();
                foreach (UnityGameObject uObj in serializedObjs) {
                    GameObject obj = Managers.Object.CreateObject(uObj);
                    if (uObj.ObjectType == (int)Define.ObjectType.RacePlayer)
                        raceScene.RegisterMyController(obj);
                    else if (uObj.ObjectType == (int)Define.ObjectType.RaceOpponent)
                        raceScene.RegisterOppoController(uObj.ObjectId, obj);  
                }
            }
        }

        public void ResponseCollision(Vector3 nestedForce) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.UpdateCollision(nestedForce);
            }
        }

        public void ResponseMovement(int objectId, Vector3 pos, Vector3 front, Vector3 vel, int state) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.UpdateMovement(objectId, pos, front, vel, state);
            }
        }

        public void SendMyMovementAndCollision() {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                GameObjectMovementInfo myMovementInfo = raceScene.SerializeMyMovementStateAndCollision();
                C_R_ResponseMovementAndCollision pkt = new() {
                    MovementInfo = myMovementInfo,
                };
                _netRef.Send(pkt);
            }
        }

        public void ResponseSRTriggerObstacle(int obstacleId, int triggerId) { 
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.OperateObstacle(obstacleId, triggerId);
            }
        }

        public void FallDown() {
            C_R_FallDown pkt = new() { };
            _netRef.Send(pkt);
        }

        public void ResponseFallDown(Vector3 pos) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.ForceMovePlayer(pos);
            }
        }

        public void ArrivedInLine(int lineId) {
            C_R_ArriveInNextLine pkt = new() { LineId = lineId, };
            _netRef.Send(pkt);
        }

        public void ResponseArriveInNextLine(int lineId) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.ConfirmArrivedLine(lineId);
            }
        }

        public bool OnCollisionEnter(int objectIdx) {
            lock (_collisionHashLock) {
                return _collidingObjectIdxs.Add(objectIdx);
            }
        }

        public bool OnCollisionExit(int objectIdx) {
            lock (_collisionHashLock) {
                return _collidingObjectIdxs.Remove(objectIdx);
            }
        }

        public void SetReadyCommandHandler(int countdown) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.CountdownBeforeStart(countdown);
            }
        }

        public void ResponseSRResult(bool isWinner, int winnerIdx) {
            BaseScene scene = Managers.Scene.CurrentScene;
            if (scene == null)
                return;

            if (scene is RaceScene raceScene) {
                raceScene.EndGame(isWinner, winnerIdx);
            }
        }
    }

    public void TryRequestGameState(int gameId) {
        C_RequestGameState pkt = PacketMaker.MakeCRequestGameState(gameId);
        Send(pkt);
    }

    #region TestGame

    public void ProcessTestGameState(IMessage packet) {
        S_TestGameState recvPkt = packet as S_TestGameState;
		foreach(UnityGameObject uObj in recvPkt.Objects) {
			Managers.Object.CreateObject(uObj);
		}
    }
    #endregion

    public void Update() {

	}

#region Session의 통신 결과를 Client에게 널리 알릴 델리게이터
    //FM대로하면, private로 선언하고 구독 및 구취하는 함수를 public으로 열어야 함.
    public Action OnConnectedAct;
	public Action OnConnectedFailedAct;
#endregion

}

