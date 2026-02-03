using Google.Protobuf.Protocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class PingPongScene : BaseScene {
    RaycastPlane _raycastPlane;
    private Vector3 _lastMousePointerPosition;
    private int _playerIdx = -1;
    MyPlayerBarController _myPlayerBar;
    EnemyPlayerBarController _eastPlayerBar;
    EnemyPlayerBarController _westPlayerBar;

    PingPongCameraController _pingPongCameraController;
    UI_PingPongScoreBoard _PingPongScoreBoard;
    UI_PingPong_EndGame _uiEndGame;

    protected override void Init() {
        //Base - EventSystem등록.
        base.Init();

        //SceneManager에 비 동기적으로 로딩하는데 사용된 자원들 초기화
        SceneType = Scene.PingPong;
        Managers.Scene.ResetLoadSceneOp();
        
        //Mouse위치를 추적해줄 RaycastPlane을 참조.
        GameObject goRaycastPlane = GameObject.Find("RaycastPlane");
        if (goRaycastPlane != null) {
            _raycastPlane = goRaycastPlane.GetComponent<RaycastPlane>();
        }
        else {
            Debug.LogError("RaycastPlane이 Scene에 없습니다.");
        }

        //적 Bar를 참조.
        GameObject goEastPlayerBar = GameObject.Find("EPlayerBar");
        GameObject goWestPlayerBar = GameObject.Find("WPlayerBar");
        if (goEastPlayerBar != null) {
            _eastPlayerBar = goEastPlayerBar.GetComponent<EnemyPlayerBarController>();
        }
        if (goWestPlayerBar != null) {
            _westPlayerBar = goWestPlayerBar.GetComponent<EnemyPlayerBarController>();
        }
        GameObject scoreBoard = GameObject.Find("UI_PingPongScoreBoard");
        if (scoreBoard != null) {
            _PingPongScoreBoard = scoreBoard.GetComponent<UI_PingPongScoreBoard>();
        }
        _uiEndGame = Managers.UI.CacheSceneUI<UI_PingPong_EndGame>();

        //CameraController참조
        GameObject cam = GameObject.Find("TopViewCamera");
        if (cam != null) { 
            _pingPongCameraController = cam.GetComponent<PingPongCameraController>();
        }

        Managers.Setting.ApplyPreviousSceneSetting();
        Managers.Sound.GetOrAddAudioClip("PingPongImpact0");
        Managers.Sound.GetOrAddAudioClip("PingPongImpact1");
        Managers.Sound.GetOrAddAudioClip("PingPongImpact2");
        Managers.Sound.GetOrAddAudioClip("PingPongImpact3");
        Managers.Sound.GetOrAddAudioClip("gameEnd");
        Managers.Sound.Play("PingPongScene", Define.Sound.Bgm);

        //서버에게 Scene에 로딩이 완료되었음을 통지. Game정보를 요청
        Managers.Network.TryRequestGameState((int)GameType.PingPong);
    }

    //플레이어가 어느 방위의 수호자인지 정보가 서버로부터 전달되었을 때.
    public void SetId(int playerIdx) {
        _playerIdx = playerIdx;

        Debug.Log(playerIdx);

        MakeMyPlayerBar(_playerIdx);
        if (_pingPongCameraController != null) { 
            _pingPongCameraController.SetPlayerIdx(_playerIdx);
        }
    }

    public void MakeMyPlayerBar(int playerIdx) {
        if (_myPlayerBar == null) {
            GameObject playerBar = Managers.Resource.Instantiate("GameObjects/PlayerBar");
            _myPlayerBar = playerBar.AddComponent<MyPlayerBarController>();
            _myPlayerBar.SetPlayerIdx(_playerIdx);
            Quaternion rotationForCases01 = Quaternion.Euler(0f, 90f, 0f);
            GameObject disableBar = null;
            GameObject goalLine = null;

            switch (playerIdx) {
                case 0:
                    playerBar.transform.position = new Vector3(6.4f, 0.2f, 0f);
                    playerBar.transform.rotation = rotationForCases01;
                    _myPlayerBar.SetMoveDir(false);

                    disableBar = GameObject.Find("EPlayerBar");
                    goalLine = GameObject.Find("EastGoalLine");

                    break;
                case 1:
                    playerBar.transform.position = new Vector3(-6.4f, 0.2f, 0f);
                    playerBar.transform.rotation = rotationForCases01;
                    _myPlayerBar.SetMoveDir(false);

                    disableBar = GameObject.Find("WPlayerBar");
                    goalLine = GameObject.Find("WestGoalLine");

                    break;
                default:
                    break;
            }
            if (disableBar != null)
                disableBar.SetActive(false);
            if (goalLine != null)
                goalLine.AddComponent<MyGoalLine>();
        }
    }

    public void OnMouseMove(Vector3 mousePointerPosition) {
        if (mousePointerPosition.x > 3.2f)
            mousePointerPosition.x = 3.2f;
        if (mousePointerPosition.x < -3.2f)
            mousePointerPosition.x = -3.2f;
        if (mousePointerPosition.z > 3.2f)
            mousePointerPosition.z = 3.2f;
        if (mousePointerPosition.z < -3.2f)
            mousePointerPosition.z = -3.2f;
        if (mousePointerPosition != _lastMousePointerPosition) {
            _lastMousePointerPosition = mousePointerPosition;
            if (_myPlayerBar != null)
                _myPlayerBar.MoveToPoint(mousePointerPosition);
        }
    }

    public Vector3 GetPlayerBarPosition() {
        if (_myPlayerBar != null)
            return _myPlayerBar.GetPosition();
        else
            return new Vector3(0f, 0f, 0f);
    }

    public void RenewPlayerBarPosition(S_P_RequestPlayerBarPosition positionPkt) {
        if (_playerIdx != 0) {
            _eastPlayerBar.NewSetPosition(positionPkt.Ex, positionPkt.Ez);
        }
        if (_playerIdx != 1) {
            _westPlayerBar.NewSetPosition(positionPkt.Wx, positionPkt.Wz);
        }
    }

    public void RenewScores(List<int> scores) {
        _PingPongScoreBoard.RenewScores(scores);
    }

    private void Update() {
        if (_raycastPlane != null) {
            Vector3 mousePointerPosition = _raycastPlane.GetRaycastPoint();
            OnMouseMove(mousePointerPosition);
        }
    }

    public void EndGame(bool isWinner, List<int> scores) {
        Managers.UI.ShowSceneUI<UI_PingPong_EndGame>();
        Managers.Sound.Play("gameEnd");

        StartCoroutine(EndGameRoutine(isWinner, scores));
    }

    private IEnumerator EndGameRoutine(bool isWinner, List<int> scores) {
        Time.timeScale = 0f;

        yield return new WaitForSecondsRealtime(2f);

        Time.timeScale = 1f;

        Managers.Network.Match.ResetMatchState();
        Managers.Scene.EndGame(isWinner, scores);
    }

    public override void Clear() {

    }
}
