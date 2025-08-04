using UnityEngine;
using UnityEngine.SceneManagement;

public class LoginScene : BaseScene {
    bool cnt = false;
    bool lgn = false;

    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;
    }

    private void Update() {
        if (Input.GetKeyDown(KeyCode.Q)) {
            if (!cnt) {
                Managers.Network.TryConnectToServer();
                cnt = true;
                lgn = true;
            }
        }
    }

    public override void Clear() {
        Debug.Log("Login Scene Cleared");
    }
}
