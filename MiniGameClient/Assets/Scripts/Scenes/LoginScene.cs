using UnityEngine;

public class LoginScene : BaseScene {
    public enum Stage {
        Connect,
        Login,
        Lobby,
        MatchMake,
    }

    private Stage _stage = Stage.Connect;

    UI_StartGame _uiStartGame;

    UI_LoginOrCreateAccount _uiLoginOrCreateAccount;
    UI_LoginPopup _uiLoginPopup;
    UI_CreateAccountPopup _uiCreateAccountPopup;
    private int _loginOpt = 0;

    private int _lobbyOpt = 0;
    private OptionSelecterController _optionSelecter;
    
    //Scene�� �ٲ� ��, �� ģ���� ��ǥ�� ������ ��� �ʱ�ȭ �۾��� �� �ٰ���.
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;

        //����� UI�� �̸� �޸𸮿� �÷��д�.
        _uiStartGame = Managers.UI.ShowSceneUI<UI_StartGame>();
        _uiLoginOrCreateAccount = Managers.UI.ShowSceneUI<UI_LoginOrCreateAccount>();
        _uiLoginPopup = Managers.UI.ShowPopupUI<UI_LoginPopup>();
        _uiCreateAccountPopup = Managers.UI.ShowPopupUI<UI_CreateAccountPopup>();
        Managers.UI.DisableUI("UI_LoginOrCreateAccount");
        Managers.UI.DisableUI("UI_LoginPopup");
        Managers.UI.DisableUI("UI_CreateAccountPopup");

        GameObject go = GameObject.Find("OptionSelecter");
        if (go != null) {
            _optionSelecter = go.GetComponent<OptionSelecterController>();
        }
        else {
            Debug.LogError("OptionSelecter�� Scene�� �����ϴٸ�");
        }

        Managers.Input.AddKeyListener(KeyCode.UpArrow, UpLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, DownLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.UpArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.Escape, BackToPreviousMenu, InputManager.KeyState.Up);
        Managers.Network.OnConnectedAct += ConnectToServerSucceed;
        Managers.Network.OnConnectedFailedAct += ConnectToServerFailed;
        Managers.Network.OnWrongIdAct += WrongId;
        Managers.Network.OnWrongPasswordAct += WrongPassword;
        Managers.Network.OnLoginAct += LoginSucceed;
    }

    private void ChangeLoginOpt() {
        if (_stage == Stage.Login) {
            _loginOpt = (_loginOpt + 1) % 2;
            _uiLoginOrCreateAccount.SetSelectedOpt(_loginOpt);
            if (_loginOpt == 0) {
                Managers.UI.ShowPopupUI<UI_LoginPopup>();
                Managers.UI.DisableUI("UI_CreateAccountPopup");
            } else {
                Managers.UI.ShowPopupUI<UI_CreateAccountPopup>();
                Managers.UI.DisableUI("UI_LoginPopup");
            }
        }
    }

    private void UpLobbyOpt() {
        if (_stage == Stage.Lobby) {
            _lobbyOpt = (_lobbyOpt + 1) % 5;
            _optionSelecter.SetOpt(_lobbyOpt);
        }
    }

    private void DownLobbyOpt() {
        if (_stage == Stage.Lobby) {
            _lobbyOpt = (_lobbyOpt + 4) % 5;
            _optionSelecter.SetOpt(_lobbyOpt);
        }
    }

    private void ConnectToServerSucceed() {
        if (_stage == Stage.Connect) {
            Managers.ExecuteAtMainThread(GoToLoginStage);
        }
    }

    private void GoToConnectStage() {
        _lobbyOpt = 5;
        _optionSelecter.SetOpt(_lobbyOpt);
        _stage = Stage.Connect;
        Managers.UI.ShowSceneUI<UI_StartGame>();

        Managers.UI.DisableUI("UI_LoginOrCreateAccount");
        Managers.UI.DisableUI("UI_LoginPopup");
        Managers.UI.DisableUI("UI_CreateAccountPopup");
    }

    private void GoToLoginStage() {
        _lobbyOpt = 6;
        _optionSelecter.SetOpt(_lobbyOpt);
        _stage = Stage.Login;
        Managers.UI.DisableUI("UI_StartGame");
        //Managers.UI.DisableUI("UI_");

        _loginOpt = 0;
        Managers.UI.ShowPopupUI<UI_LoginPopup>();
        Managers.UI.ShowSceneUI<UI_LoginOrCreateAccount>();
    }

    private void GoToLobbyStage() {

    }

    private void GoToMatchMakeStage() {

    }

    private void ConnectToServerFailed() {
        Managers.ExecuteAtMainThread(() => {
            //Managers.UI.ShowErrorUI("�������� ���ῡ �����߽��ϴ�.", false);
            Managers.UI.ShowErrorUIOnlyConfirm("�������� ���ῡ �����߽��ϴ�.");
        });
    }

    public void LoginSucceed() {
        if (_stage == Stage.Login) {
            _lobbyOpt = 0;
            _optionSelecter.SetOpt(_lobbyOpt);
            _stage = Stage.Lobby;

            Managers.UI.DisableUI("UI_CreateAccountPopup");
            Managers.UI.DisableUI("UI_LoginPopup");
            Managers.UI.DisableUI("UI_LoginOrCreateAccount");
        }   
    }

    public void WrongId() {
        Managers.UI.ShowErrorUIOnlyConfirm("���� ���̵��Դϴ�.");
    }

    public void WrongPassword() {
        Managers.UI.ShowErrorUIOnlyConfirm("��й�ȣ�� ���� �ʽ��ϴ�.");
    }

    public void BackToPreviousMenu() {
        switch (_stage) {
            case Stage.Connect:
                Managers.UI.ShowErrorUIConfirmOrCancel("������ �����Ͻðڽ��ϱ�?", () => {
                    Managers.ExecuteAtMainThread(() => {
                        Application.Quit();
#if UNITY_EDITOR
                        UnityEditor.EditorApplication.isPlaying = false;
#endif
                    });
                });
                break;
            case Stage.Login:
                GoToConnectStage();
                Managers.Network.TryDisconnect();
                break;
            case Stage.Lobby:
                break;
            case Stage.MatchMake:
                break;
            default:
                break;
        }
    }

    public void LogOut() {

    }

    public override void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, UpLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, DownLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.Escape, BackToPreviousMenu, InputManager.KeyState.Up);
        Managers.Network.OnConnectedAct -= ConnectToServerSucceed;
        Managers.Network.OnConnectedFailedAct -= ConnectToServerFailed;
        Managers.Network.OnWrongIdAct -= WrongId;
        Managers.Network.OnWrongPasswordAct -= WrongPassword;
        Managers.Network.OnLoginAct -= LoginSucceed;
        Debug.Log("Login Scene Cleared");
    }
}
