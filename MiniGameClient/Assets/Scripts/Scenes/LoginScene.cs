using UnityEngine;
using UnityEngine.SceneManagement;

public class LoginScene : BaseScene {
    bool t = false;

    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;
    }

    private void Update() {
        if (Input.GetKeyDown(KeyCode.Q)) {
            if (!t) {
                Managers.Network.TryConnectToServer();
                t = true;
            }
        }
    }

    public override void Clear() {
        Debug.Log("Login Scene Cleared");
    }
}
