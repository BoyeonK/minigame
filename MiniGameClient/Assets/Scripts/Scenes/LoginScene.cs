using UnityEngine;
using UnityEngine.SceneManagement;

public class LoginScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;
    }

    private void Update() {
        if (Input.GetKeyDown(KeyCode.Q))
            if (!(Managers.Network.IsConnected()))
                Managers.Network.TryConnectToServer();
        if (Input.GetKeyDown(KeyCode.W)) {
            if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
                Debug.Log("�α��� �õ� ������ ���� �ϴ� �ڸ�");
                //PacketMaker.MakeCLogin("tetepiti149", "q1w2e3r4!@");
            }
        }
    }

    public override void Clear() {
        Debug.Log("Login Scene Cleared");
    }
}
