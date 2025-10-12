using Google.Protobuf.Protocol;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UIElements;
using static Define;

public class PingPongScene : BaseScene {
    RaycastPlane _raycastPlane;
    private Vector3 _lastMousePointerPosition;
    private int _playerIdx = -1;
    MyPlayerBarController _myPlayerBar;
    EnemyPlayerBarController _eastPlayerBar;
    EnemyPlayerBarController _westPlayerBar;
    EnemyPlayerBarController _southPlayerBar;
    EnemyPlayerBarController _northPlayerBar;

    PingPongCameraController _pingPongCameraController;

    protected override void Init() {
        //Base - EventSystem���.
        base.Init();

        //SceneManager�� �� ���������� �ε��ϴµ� ���� �ڿ��� �ʱ�ȭ
        SceneType = Scene.TestGame;
        Managers.Scene.ResetLoadSceneOp();
        
        //Mouse��ġ�� �������� RaycastPlane�� ����.
        GameObject goRaycastPlane = GameObject.Find("RaycastPlane");
        if (goRaycastPlane != null) {
            _raycastPlane = goRaycastPlane.GetComponent<RaycastPlane>();
        }
        else {
            Debug.LogError("RaycastPlane�� Scene�� �����ϴ�.");
        }

        //�� Bar�� ����.
        GameObject goEastPlayerBar = GameObject.Find("EPlayerBar");
        GameObject goWestPlayerBar = GameObject.Find("WPlayerBar");
        GameObject goSouthPlayerBar = GameObject.Find("SPlayerBar");
        GameObject goNorthPlayerBar = GameObject.Find("NPlayerBar");
        if (goEastPlayerBar != null) {
            _eastPlayerBar = goEastPlayerBar.GetComponent<EnemyPlayerBarController>();
        }
        if (goWestPlayerBar != null) {
            _westPlayerBar = goWestPlayerBar.GetComponent<EnemyPlayerBarController>();
        }
        if (goSouthPlayerBar != null) {
            _southPlayerBar = goSouthPlayerBar.GetComponent<EnemyPlayerBarController>();
        }
        if (goNorthPlayerBar != null) {
            _northPlayerBar = goNorthPlayerBar.GetComponent<EnemyPlayerBarController>();
        }

        //CameraController����
        GameObject cam = GameObject.Find("TopViewCamera");
        if (cam != null) { 
            _pingPongCameraController = cam.GetComponent<PingPongCameraController>();
        }

        //���������͸� ����
        Managers.Network.OnPingPongEndAct += EndGame;

        //�������� Scene�� �ε��� �Ϸ�Ǿ����� ����. Game������ ��û
        Managers.Network.TryRequestGameState((int)GameType.PingPong);

        //Invoke(nameof(TestMakeBulletFunc), 5f);
    }

    //�÷��̾ ��� ������ ��ȣ������ ������ �����κ��� ���޵Ǿ��� ��.
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
                case 2:
                    playerBar.transform.position = new Vector3(0f, 0.2f, -6.4f);

                    disableBar = GameObject.Find("SPlayerBar");
                    goalLine = GameObject.Find("SouthGoalLine");

                    break;
                case 3:
                    playerBar.transform.position = new Vector3(0f, 0.2f, 6.4f);

                    disableBar = GameObject.Find("NPlayerBar");
                    goalLine = GameObject.Find("NorthGoalLine");

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
            _eastPlayerBar.SetPosition(positionPkt.Ex, positionPkt.Ez);
        }
        if (_playerIdx != 1) {
            _westPlayerBar.SetPosition(positionPkt.Wx, positionPkt.Wz);
        }
        if (_playerIdx != 2) {
            _southPlayerBar.SetPosition(positionPkt.Sx, positionPkt.Sz);
        }
        if (_playerIdx != 3) {
            _northPlayerBar.SetPosition(positionPkt.Nx, positionPkt.Nz);
        }
    }

    private void Update() {
        if (_raycastPlane != null) {
            Vector3 mousePointerPosition = _raycastPlane.GetRaycastPoint();
            OnMouseMove(mousePointerPosition);
        }
    }

    private void EndGame() {
        Managers.Scene.LoadScene(Scene.Login);
    }

    public override void Clear() {
        Managers.Network.OnPingPongEndAct -= EndGame;
    }
}
