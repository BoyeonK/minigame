using UnityEngine;
using static Define;

public class PingPongScene : BaseScene {
    RaycastPlane _raycastPlane;
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
    }

    public void OnMouseMove(Vector3 mousePosition) {
        if (_mousePointerPosition == mousePosition) { return; }
        _mousePointerPosition = mousePosition;
        Debug.Log($"{_mousePointerPosition}");
    }

    private void EndGame() {
        Managers.Scene.LoadScene(Scene.Login);
    }

    public override void Clear() {
        Managers.Network.OnPingPongEndAct -= EndGame;
    }
}
