using UnityEngine;
using static Define;

public class PingPongScene : BaseScene {
    RaycastPlane _raycastPlane;
    MyPlayerBarController _eastBar;
    MyPlayerBarController _westBar;
    MyPlayerBarController _southBar;
    MyPlayerBarController _northBar;
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

        GameObject eastBar = GameObject.Find("EPlayerBar");
        GameObject westBar = GameObject.Find("WPlayerBar");
        GameObject southBar = GameObject.Find("SPlayerBar");
        GameObject northBar = GameObject.Find("NPlayerBar");

        if (eastBar != null) {
            _eastBar = eastBar.GetComponent<MyPlayerBarController>();
            if (_eastBar != null)
                _eastBar.SetMoveDir(false);
        }
        if (westBar != null) {
            _westBar = westBar.GetComponent<MyPlayerBarController>();
            if (_westBar != null)
                _westBar.SetMoveDir(false);
        }
        if (southBar != null) {
            _southBar = southBar.GetComponent<MyPlayerBarController>();
            if (_southBar != null)
                _southBar.SetMoveDir(true);
        }
        if (northBar != null) {
            _northBar = northBar.GetComponent<MyPlayerBarController>();
            if (_northBar != null)
                _northBar.SetMoveDir(true);
        }
    }

    public void OnMouseMove(Vector3 mousePosition) {
        if (_mousePointerPosition == mousePosition) { return; }
        _mousePointerPosition = mousePosition;
        //Debug.Log($"{_mousePointerPosition}");
        _eastBar.MoveToPoint(mousePosition);
        _westBar.MoveToPoint(mousePosition);
        _southBar.MoveToPoint(mousePosition);
        _northBar.MoveToPoint(mousePosition);
    }

    private void EndGame() {
        Managers.Scene.LoadScene(Scene.Login);
    }

    public override void Clear() {
        Managers.Network.OnPingPongEndAct -= EndGame;
    }
}
