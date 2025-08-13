using Google.Protobuf.Protocol;
using UnityEngine;
using UnityEngine.SceneManagement;

public class LoginScene : BaseScene {
    private bool _isConnected = false;
    private bool _isLogined = false;
    private int _opt = 0;
    private OptionSelecterController _optionSelecter;

    //Scene�� �ٲ� ��, �� ģ���� ��ǥ�� ������ ��� �ʱ�ȭ �۾��� �� �ٰ���.
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;
        Managers.UI.ShowPopupUI<UI_TestLoginPopup>();
        GameObject go = GameObject.Find("OptionSelecter");
        if (go != null) {
            _optionSelecter = go.GetComponent<OptionSelecterController>();
        }
        else {
            Debug.LogError("OptionSelecter�� Scene�� �����ϴٸ�");
        }

        Managers.Input.AddKeyListener(KeyCode.Q, TryConnectToServer, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.UpArrow, PrOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, NxtOpt, InputManager.KeyState.Up);
        Managers.Network.OnConnectedAct += ConnectToServerSucceed;
        Managers.Network.OnLoginAct += LoginSucceed;
    }

    private void PrOpt() {
        if (_isLogined) {
            _opt = (_opt + 1) % 4;
            _optionSelecter.SetOpt(_opt);
            Debug.Log("Pre");
        }
    }

    private void NxtOpt() {
        if (_isLogined) {
            _opt = (_opt + 3) % 4;
            _optionSelecter.SetOpt(_opt);
            Debug.Log("Next");
        }
    }

    private void TryConnectToServer() {
        if (!(Managers.Network.IsConnected()))
            Managers.Network.TryConnectToServer();
    }

    private void ConnectToServerSucceed() {
        if (_isConnected == false) {
            _opt = 5;
            _optionSelecter.SetOpt(_opt);
            _isConnected = true;
        }
    }

    public void LoginSucceed() {
        if (_isConnected && !_isLogined) {
            _opt = 0;
            _optionSelecter.SetOpt(_opt);
            _isLogined = true;
        }   
    }

    public override void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Q, TryConnectToServer, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, PrOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, NxtOpt, InputManager.KeyState.Up);
        Managers.Network.OnConnectedAct -= ConnectToServerSucceed;
        Managers.Network.OnLoginAct -= LoginSucceed;
        Debug.Log("Login Scene Cleared");
    }
}
