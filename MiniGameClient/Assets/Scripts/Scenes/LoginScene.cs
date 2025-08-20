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
    
    //Scene이 바뀔 때, 이 친구가 대표로 나서서 모든 초기화 작업을 해 줄거임.
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;

        //사용할 UI를 미리 메모리에 올려둔다.
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
            Debug.LogError("OptionSelecter가 Scene에 없습니다링");
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
            //Managers.UI.ShowErrorUI("서버와의 연결에 실패했습니다.", false);
            Managers.UI.ShowErrorUIOnlyConfirm("서버와의 연결에 실패했습니다.");
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
        Managers.UI.ShowErrorUIOnlyConfirm("없는 아이디입니다.");
    }

    public void WrongPassword() {
        Managers.UI.ShowErrorUIOnlyConfirm("비밀번호가 맞지 않습니다.");
    }

    public void BackToPreviousMenu() {
        switch (_stage) {
            case Stage.Connect:
                Managers.UI.ShowErrorUIConfirmOrCancel("게임을 종료하시겠습니까?", () => {
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
