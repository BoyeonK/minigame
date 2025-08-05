using Google.Protobuf.Protocol;
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
                Debug.Log("로그인 시도 로직이 들어가야 하는 자리");
                C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti149", "q1w2e3r4!@");
                Managers.Network.Send(pkt);
            }
        }
    }

    public override void Clear() {
        Debug.Log("Login Scene Cleared");
    }
}
