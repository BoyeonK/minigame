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
        //Managers.UI.ShowPopupUI<UI_TestStartInfo>();
        Managers.UI.ShowSceneUI<UI_StartGame>();
        GameObject go = GameObject.Find("OptionSelecter");
        if (go != null) {
            _optionSelecter = go.GetComponent<OptionSelecterController>();
        }
        else {
            Debug.LogError("OptionSelecter�� Scene�� �����ϴٸ�");
        }

        Managers.Input.AddKeyListener(KeyCode.UpArrow, PrOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, NxtOpt, InputManager.KeyState.Up);
        Managers.Network.OnConnectedAct += ConnectToServerSucceed;
        Managers.Network.OnConnectedFailedAct += ConnectToServerFailed;
        Managers.Network.OnWrongIdAct += WrongId;
        Managers.Network.OnWrongPasswordAct += WrongPassword;
        Managers.Network.OnLoginAct += LoginSucceed;
    }

    private void PrOpt() {
        if (_isLogined) {
            _opt = (_opt + 1) % 5;
            _optionSelecter.SetOpt(_opt);
            Debug.Log("Pre");
        }
    }

    private void NxtOpt() {
        if (_isLogined) {
            _opt = (_opt + 4) % 5;
            _optionSelecter.SetOpt(_opt);
            Debug.Log("Next");
        }
    }

    private void ConnectToServerSucceed() {
        if (_isConnected == false) {
            _opt = 6;
            _optionSelecter.SetOpt(_opt);
            _isConnected = true;
            Managers.ExecuteAtMainThread(() => {
                Managers.UI.DisableUI("UI_StartGame");
                Managers.UI.ShowPopupUI<UI_TestLoginPopup>();
            });
        }
    }

    private void ConnectToServerFailed() {
        Managers.ExecuteAtMainThread(() => {
            Managers.UI.ShowErrorUI("�������� ���ῡ �����߽��ϴ�.", false);
        });
    }

    public void LoginSucceed() {
        if (_isConnected && !_isLogined) {
            _opt = 0;
            _optionSelecter.SetOpt(_opt);
            _isLogined = true;
        }   
    }

    public void WrongId() {

    }

    public void WrongPassword() {

    }

    public override void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, PrOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, NxtOpt, InputManager.KeyState.Up);
        Managers.Network.OnConnectedAct -= ConnectToServerSucceed;
        Managers.Network.OnConnectedFailedAct -= ConnectToServerFailed;
        Managers.Network.OnWrongIdAct -= WrongId;
        Managers.Network.OnWrongPasswordAct -= WrongPassword;
        Managers.Network.OnLoginAct -= LoginSucceed;
        Debug.Log("Login Scene Cleared");
    }
}
