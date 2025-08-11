using Google.Protobuf.Protocol;
using UnityEngine;
using UnityEngine.SceneManagement;
using static InputManager;

public class LoginScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;
        Managers.Resource.Instantiate("UI/Popup/LoginPopup");

        Managers.Input.AddKeyListener(KeyCode.Q, TryConnectToServer, KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.W, TryLoginCorrect, KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.S, TryLoginWrongPassword, KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.X, TryLoginWrongID, KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.E, TryCreateAccountCorrect, KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.D, TryCreateAccountExisting, KeyState.Up);
    }

    private void TryConnectToServer() {
        if (!(Managers.Network.IsConnected()))
            Managers.Network.TryConnectToServer();
    }

    private void TryLoginCorrect() {
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("로그인 시도 (정상)");
            C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti149", "qwe123");
            Managers.Network.Send(pkt);
        }
    }

    private void TryLoginWrongPassword() {
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("로그인 시도 (비번틀림)");
            C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti149", "qwe1234");
            Managers.Network.Send(pkt);
        }
    }

    private void TryLoginWrongID() {
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("로그인 시도 (아이디틀림)");
            C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), "tetepiti14", "qwe123");
            Managers.Network.Send(pkt);
        }
    }

    private void TryCreateAccountCorrect() {
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("계정생성 시도 (정상)");
            C_Encrypted pkt = PacketMaker.MakeCCreateAccount(Managers.Network.GetSession(), "tetepiti149", "qwe123");
            Managers.Network.Send(pkt);
        }
    }

    private void TryCreateAccountExisting() {
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("계정생성 시도 (이미존재)");
            C_Encrypted pkt = PacketMaker.MakeCCreateAccount(Managers.Network.GetSession(), "erdfttgg", "qwe123");
            Managers.Network.Send(pkt);
        }
    }

    public override void Clear() {
        Debug.Log("Login Scene Cleared");
    }
}
