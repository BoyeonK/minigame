using UnityEngine;
using static Define;

public class PingPongScene : BaseScene {
    RaycastPlane _raycastPlane;
    MyPlayerBarController _myPlayerBar;
    private Vector3 _mousePointerPosition;

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

        MakeMyPlayerBar(2);
        Invoke(nameof(TestMakeBulletFunc), 5f);
    }

    public void MakeMyPlayerBar(int playerIdx) {
        if (_myPlayerBar == null) {
            GameObject playerBar = Managers.Resource.Instantiate("GameObjects/PlayerBar");
            _myPlayerBar = playerBar.AddComponent<MyPlayerBarController>();
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

    public void OnMouseMove(Vector3 mousePosition) {
        if (_mousePointerPosition == mousePosition) { return; }
        _mousePointerPosition = mousePosition;
        _myPlayerBar.MoveToPoint(_mousePointerPosition);
        Debug.Log($"{_mousePointerPosition}");
        
    }

    private void TestMakeBulletFunc() {
        Debug.Log("아잉 코루틴 쓰기 시져시져");
        Vector3 pos = new Vector3(-2.5f, 0.2f, 0f);
        Vector3 dir = new Vector3(0f, 0f, -1f);
        for (int i = 0; i < 5; i++) {
            InternalTestMakeBulletFunc(pos, dir, 1f + i*0.2f);
            pos.x = pos.x + 1.25f;
        }   
    }

    private void InternalTestMakeBulletFunc(Vector3 pos, Vector3 dir, float speed) {
        GameObject b1 = Managers.Resource.Instantiate("GameObjects/PingPongBullet1");
        PingPongBulletController p1 = b1.GetComponent<PingPongBulletController>();
        b1.transform.position = pos;
        p1.SetSpeed(speed);
        p1.SetMoveDir(dir);
    }

    private void EndGame() {
        Managers.Scene.LoadScene(Scene.Login);
    }

    public override void Clear() {
        Managers.Network.OnPingPongEndAct -= EndGame;
    }
}
