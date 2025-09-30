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
    List<EnemyPlayerBarController> _enemyPlayerBars;

    protected override void Init() {
        base.Init();
        SceneType = Scene.TestGame;
        Managers.Scene.ResetLoadSceneOp();
        
        Managers.Network.TryRequestGameState((int)GameType.PingPong);
        Managers.Network.OnPingPongEndAct += EndGame;

        GameObject go = GameObject.Find("RaycastPlane");
        if (go != null) {
            _raycastPlane = go.GetComponent<RaycastPlane>();
        }
        else {
            Debug.LogError("RaycastPlane이 Scene에 없습니다.");
        }

        //Invoke(nameof(TestMakeBulletFunc), 5f);
    }

    public void SetId(int playerIdx) {
        _playerIdx = playerIdx;
        MakeMyPlayerBar(_playerIdx);
        //TODO : 시점 돌리기
        //TODO : 해당 방향 벽 비 활성화
        //TODO : 해당 방향 적 PlayerBar 제거
        //TODO : 나의 GoalLine에 컴포넌트 부착
    }

    public void MakeMyPlayerBar(int playerIdx) {
        if (_myPlayerBar == null) {
            GameObject playerBar = Managers.Resource.Instantiate("GameObjects/PlayerBar");
            _myPlayerBar = playerBar.AddComponent<MyPlayerBarController>();
            _myPlayerBar.SetPlayerIdx(_playerIdx);
            Quaternion rotationForCases01 = Quaternion.Euler(0f, 90f, 0f);
            switch (playerIdx) {
                case 0:
                    playerBar.transform.position = new Vector3(6.4f, 0.2f, 0f);
                    playerBar.transform.rotation = rotationForCases01;
                    _myPlayerBar.SetMoveDir(false);
                    break;
                case 1:
                    playerBar.transform.position = new Vector3(-6.4f, 0.2f, 0f);
                    playerBar.transform.rotation = rotationForCases01;
                    _myPlayerBar.SetMoveDir(false);
                    break;
                case 2:
                    playerBar.transform.position = new Vector3(0f, 0.2f, -6.4f);
                    break;
                case 3:
                    playerBar.transform.position = new Vector3(0f, 0.2f, 6.4f);
                    break;
                default:
                    break;
            }
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
            if (_myPlayerBar != null) {
                _myPlayerBar.MoveToPoint(mousePointerPosition);
            }
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
