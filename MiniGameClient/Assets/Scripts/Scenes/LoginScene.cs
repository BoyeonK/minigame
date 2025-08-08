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
                Debug.Log("�α��� �õ�");
                C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti149", "qwe123");
                Managers.Network.Send(pkt);
            }
        }
        if (Input.GetKeyDown(KeyCode.S))
        {
            if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined()))
            {
                Debug.Log("�α��� �õ�");
                C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti149", "qwe1234");
                Managers.Network.Send(pkt);
            }
        }
        if (Input.GetKeyDown(KeyCode.X))
        {
            if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined()))
            {
                Debug.Log("�α��� �õ�");
                C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti14", "qwe123");
                Managers.Network.Send(pkt);
            }
        }
        if (Input.GetKeyDown(KeyCode.E)) {
            if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
                Debug.Log("�������� �õ� ������ ���� �ϴ� �ڸ�");
                C_Encrypted pkt = PacketMaker.MakeCCreateAccount(Managers.Network.GetSession(), "tetepiti149", "qwe123");
                Managers.Network.Send(pkt);
            }
        }
    }

    public override void Clear() {
        Debug.Log("Login Scene Cleared");
    }
}
