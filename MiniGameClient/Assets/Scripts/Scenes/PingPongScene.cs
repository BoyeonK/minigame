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
    List<EnemyPlayerBarController> _enemyPlayerBars = new();
    PingPongCameraController _pingPongCameraController;

    protected override void Init() {
        //Base - EventSystem등록.
        base.Init();

        //SceneManager에 비 동기적으로 로딩하는데 사용된 자원들 초기화
        SceneType = Scene.TestGame;
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
        GameObject goSouthPlayerBar = GameObject.Find("SPlayerBar");
        GameObject goNorthPlayerBar = GameObject.Find("NPlayerBar");
        if (goEastPlayerBar != null) {
            EnemyPlayerBarController ebar = goEastPlayerBar.GetComponent<EnemyPlayerBarController>();
            _enemyPlayerBars.Add(ebar);
        }
        if (goWestPlayerBar != null) {
            EnemyPlayerBarController ebar = goWestPlayerBar.GetComponent<EnemyPlayerBarController>();
            _enemyPlayerBars.Add(ebar);
        }
        if (goSouthPlayerBar != null) {
            EnemyPlayerBarController ebar = goSouthPlayerBar.GetComponent<EnemyPlayerBarController>();
            _enemyPlayerBars.Add(ebar);
        }
        if (goNorthPlayerBar != null) {
            EnemyPlayerBarController ebar = goNorthPlayerBar.GetComponent<EnemyPlayerBarController>();
            _enemyPlayerBars.Add(ebar);
        }

        //CameraController참조
        GameObject cam = GameObject.Find("TopViewCamera");
        if (cam != null) { 
            _pingPongCameraController = cam.GetComponent<PingPongCameraController>();
        }

        //델리게이터를 구독
        Managers.Network.OnPingPongEndAct += EndGame;

        //서버에게 Scene에 로딩이 완료되었음을 통지. Game정보를 요청
        Managers.Network.TryRequestGameState((int)GameType.PingPong);

        //Invoke(nameof(TestMakeBulletFunc), 5f);
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

    /*
    private void TestMakeBulletFunc() {
        Debug.Log("아잉 코루틴 쓰기 시져시져");
        Vector3 dir = new Vector3(0f, 0f, -1f);
        for (int i = 0; i < 5; i++) {
            UnityGameObject bullet = new UnityGameObject();
            bullet.ObjectId = i;
            bullet.ObjectType = 4;
            XYZ xyz = new XYZ();
            xyz.X = -2.5f + i * 1.25f;
            xyz.Y = 0.2f;
            xyz.Z = 0f;
            bullet.Position = xyz;

            GameObject b1 = Managers.Object.CreateObject(bullet);
            PingPongBullet1Controller p1 = b1.GetComponent<PingPongBullet1Controller>();
            p1.SetMoveDir(dir);
            p1.SetSpeed(1f + 0.2f * i);
        }   
    }
    */

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
