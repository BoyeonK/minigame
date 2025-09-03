using Google.Protobuf.Protocol;
using System;
using System.Threading;
using UnityEngine;

public class LoginScene : BaseScene {
    public enum Stage {
        Connect,
        Login,
        Lobby,
        MatchMake,
        MatchmakeRegister,
    }

    private Stage _stage = Stage.Connect;

    UI_StartGame _uiStartGame;

    UI_LoginOrCreateAccount _uiLoginOrCreateAccount;
    UI_LoginPopup _uiLoginPopup;
    UI_CreateAccountPopup _uiCreateAccountPopup;
    UI_LobbyMenu _uiLobbyMenu;
    UI_MatchMakeMenu _uiMatchMakeMenu;

    private int _loginOpt = 0;

    private int _lobbyOpt = 0;
    private OptionSelecterController _optionSelecter;

    private int _matchMakeOpt = 0;
    
    //Scene�� �ٲ� ��, �� ģ���� ��ǥ�� ������ ��� �ʱ�ȭ �۾��� �� �ٰ���.
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;

        //����� UI�� �̸� �޸𸮿� �÷��д�.
        _uiStartGame = Managers.UI.ShowSceneUI<UI_StartGame>();
        _uiLoginOrCreateAccount = Managers.UI.ShowSceneUI<UI_LoginOrCreateAccount>();
        _uiLoginPopup = Managers.UI.ShowPopupUI<UI_LoginPopup>();
        _uiCreateAccountPopup = Managers.UI.ShowPopupUI<UI_CreateAccountPopup>();
        _uiLobbyMenu = Managers.UI.ShowSceneUI<UI_LobbyMenu>();
        _uiMatchMakeMenu = Managers.UI.ShowSceneUI<UI_MatchMakeMenu>();
        Managers.UI.DisableUI("UI_LoginOrCreateAccount");
        Managers.UI.DisableUI("UI_LoginPopup");
        Managers.UI.DisableUI("UI_CreateAccountPopup");
        Managers.UI.DisableUI("UI_LobbyMenu");
        Managers.UI.DisableUI("UI_MatchMakeMenu");

        GameObject go = GameObject.Find("OptionSelecter");
        if (go != null) {
            _optionSelecter = go.GetComponent<OptionSelecterController>();
        }
        else {
            Debug.LogError("OptionSelecter�� Scene�� �����ϴٸ�");
        }

        Managers.Input.AddKeyListener(KeyCode.UpArrow, UpLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, DownLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.UpArrow, UpMatchMakeOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, DownMatchMakeOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.UpArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.Escape, BackToPreviousMenu, InputManager.KeyState.Down);
        Managers.Network.OnConnectedAct += ConnectToServerSucceed;
        Managers.Network.OnConnectedFailedAct += ConnectToServerFailed;
        Managers.Network.OnWrongIdAct += WrongId;
        Managers.Network.OnWrongPasswordAct += WrongPassword;
        Managers.Network.OnLoginAct += LoginSucceed;
        Managers.Network.OnLogoutAct += LogoutSucceed;
        Managers.Network.OnMatchmakeRequestSucceedAct += MatchmakeRequestSucceed;
        Managers.Network.OnMatchmakeCancelSucceedAct += MatchmakeCancelSucceed;
    }

    private void ChangeLoginOpt() {
        if (_stage == Stage.Login) {
            _loginOpt = (_loginOpt + 1) % 2;
            ApplyLoginOpt();
        }
    }

    public void SetLoginOpt(int opt) {
        if (_loginOpt == opt)
            return;
        _loginOpt = opt;
        if (_stage == Stage.Login) {
            _loginOpt = opt;
            ApplyLoginOpt();
        }
    }

    private void ApplyLoginOpt() {
        if (_stage == Stage.Login) {
            _uiLoginOrCreateAccount.SetSelectedOpt(_loginOpt);
            if (_loginOpt == 0) {
                Managers.UI.ShowPopupUI<UI_LoginPopup>();
                Managers.UI.DisableUI("UI_CreateAccountPopup");
            }
            else {
                Managers.UI.ShowPopupUI<UI_CreateAccountPopup>();
                Managers.UI.DisableUI("UI_LoginPopup");
            }
        }
    }

    public void SetLobbyOpt(int opt) {
        if (_lobbyOpt == opt)
            return;
        _lobbyOpt = opt;
        if (_stage == Stage.Lobby) {
            _lobbyOpt = opt;
            ApplyLobbyOpt();
        }
    }

    private void UpLobbyOpt() {
        if (_stage == Stage.Lobby) {
            _lobbyOpt = (_lobbyOpt + 4) % 5;
            ApplyLobbyOpt();
        }
    }

    private void DownLobbyOpt() {
        if (_stage == Stage.Lobby) {
            _lobbyOpt = (_lobbyOpt + 1) % 5;
            ApplyLobbyOpt();
        }
    }

    private void ApplyLobbyOpt() {
        if (_stage == Stage.Lobby) {
            _optionSelecter.SetOpt(_lobbyOpt);
            _uiLobbyMenu.SetSelectedOpt(_lobbyOpt);
        }
    }

    public void SetMatchMakeOpt(int opt) {
        if (_matchMakeOpt == opt)
            return;
        _matchMakeOpt = opt;
        if (_stage == Stage.MatchMake) {
            _matchMakeOpt = opt;
            ApplyMatchMakeOpt();
        }
    }

    private void UpMatchMakeOpt() {
        if (_stage == Stage.MatchMake) {
            _matchMakeOpt = (_matchMakeOpt + 2) % 3;
            ApplyMatchMakeOpt();
        }
    }

    private void DownMatchMakeOpt() {
        if (_stage == Stage.MatchMake) {
            _matchMakeOpt = (_matchMakeOpt + 1) % 3;
            ApplyMatchMakeOpt();
        }
    }

    private void ApplyMatchMakeOpt() {
        if (_stage == Stage.MatchMake) {
            _uiMatchMakeMenu.SetSelectedOpt(_matchMakeOpt);
        }
    }

    public void StartMatchMake() {
        if (_stage == Stage.MatchMake) {
            Managers.Network.TryMatchMake(Define.IntToGameType(_matchMakeOpt + 1));
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
        Managers.UI.DisableUI("UI_LobbyMenu");

        _loginOpt = 0;
        Managers.UI.ShowPopupUI<UI_LoginPopup>();
        Managers.UI.ShowSceneUI<UI_LoginOrCreateAccount>();
    }

    private void GoToLobbyStage() {
        _lobbyOpt = 0;
        _optionSelecter.SetOpt(_lobbyOpt);
        _stage = Stage.Lobby;

        Managers.UI.DisableUI("UI_CreateAccountPopup");
        Managers.UI.DisableUI("UI_LoginPopup");
        Managers.UI.DisableUI("UI_LoginOrCreateAccount");
        Managers.UI.DisableUI("UI_MatchMakeMenu");
        
        Managers.UI.ShowSceneUI<UI_LobbyMenu>();
    }

    private void GoToMatchMakeStage() {
        _matchMakeOpt = 0;
        _stage = Stage.MatchMake;
        Managers.UI.DisableUI("UI_LobbyMenu");
        //Managers.UI.ShowSceneUI<>();

        Managers.UI.ShowSceneUI<UI_MatchMakeMenu>();
    }

    //�� ģ���� Network WorkerThread���� �����.
    private void GoToMatchMakeRegisterStage() {
        _stage = Stage.MatchmakeRegister;
        Managers.UI.DisableUI("UI_MatchMakeMenu");

        //Managers.UI.ShowSceneUI<>();
    }

    //�� ģ���� Network WorkerThread���� �����.
    private void GoToMatchMakeStageViaCancel() {
        Managers.ExecuteAtMainThread(() => { GoToMatchMakeStage(); });
    }

    //�� ģ���� Network WorkerThread���� �����.
    private void ConnectToServerFailed() {
        Managers.ExecuteAtMainThread(() => {
            Managers.UI.ShowErrorUIOnlyConfirm("�������� ���ῡ �����߽��ϴ�.");
        });
    }

    public void LoginSucceed() {
        if (_stage == Stage.Login) {
            GoToLobbyStage();
        }   
    }

    public void LogoutSucceed() {
        //GoToLoginStage();
    }

    public void LogoutFailed() {

    }

    public void WrongId() {
        Managers.UI.ShowErrorUIOnlyConfirm("���� ���̵��Դϴ�.");
    }

    public void WrongPassword() {
        Managers.UI.ShowErrorUIOnlyConfirm("��й�ȣ�� ���� �ʽ��ϴ�.");
    }

    public void SelectStartGame() {
        GoToMatchMakeStage();
    }

    public void MatchmakeRequestSucceed() {
        GoToMatchMakeRegisterStage();
    }

    public void MatchmakeCancelSucceed() {
        GoToMatchMakeStageViaCancel();
    }

    public void SelectLeaderboard() {
        Managers.UI.ShowErrorUIOnlyConfirm("�غ����Դϴ�. �Ф�");
    }
    public void SelectMyRecord() {
        Managers.UI.ShowErrorUIOnlyConfirm("�غ����Դϴ�. �Ф�");
    }
    public void SelectOption() {
        Managers.UI.ShowErrorUIOnlyConfirm("�غ����Դϴ�. �Ф�");
    }
    public void SelectQuit() {
        QuitApplicationUI();
    }

    private void QuitApplicationUI() {
        Managers.UI.ShowErrorUIConfirmOrCancel("������ �����Ͻðڽ��ϱ�?", () => {
            Managers.ExecuteAtMainThread(() => {
                Application.Quit();
#if UNITY_EDITOR
                UnityEditor.EditorApplication.isPlaying = false;
#endif
            });
        });
    }

    public void BackToPreviousMenu() {
        switch (_stage) {
            case Stage.Connect:
                QuitApplicationUI();
                break;
            case Stage.Login:
                GoToConnectStage();
                Managers.Network.TryDisconnect();
                break;
            case Stage.Lobby:
                Managers.UI.ShowErrorUIConfirmOrCancel("�α׾ƿ� �Ͻðڽ��ϱ�?", Logout);
                break;
            case Stage.MatchMake:
                GoToLobbyStage();
                break;
            case Stage.MatchmakeRegister:
                Managers.UI.ShowErrorUIConfirmOrCancel("���", MatchmakeCancel);
                break;
            default:
                break;
        }
    }

    public void Logout() {
        GoToLoginStage();
        Managers.Network.TryLogout();
    }

    public void MatchmakeCancel() {
        Managers.Network.TryMatchMakeCancel();
    }

    public override void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, UpLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, DownLobbyOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, UpMatchMakeOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, DownMatchMakeOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, ChangeLoginOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.Escape, BackToPreviousMenu, InputManager.KeyState.Down);
        Managers.Network.OnConnectedAct -= ConnectToServerSucceed;
        Managers.Network.OnConnectedFailedAct -= ConnectToServerFailed;
        Managers.Network.OnWrongIdAct -= WrongId;
        Managers.Network.OnWrongPasswordAct -= WrongPassword;
        Managers.Network.OnLoginAct -= LoginSucceed;
        Managers.Network.OnLogoutAct -= LogoutSucceed;
        Managers.Network.OnMatchmakeRequestSucceedAct -= MatchmakeRequestSucceed;
        Managers.Network.OnMatchmakeCancelSucceedAct -= MatchmakeCancelSucceed;
        Debug.Log("Login Scene Cleared");
    }
}
