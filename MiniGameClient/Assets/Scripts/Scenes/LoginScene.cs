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
    
    //Scene이 바뀔 때, 이 친구가 대표로 나서서 모든 초기화 작업을 해 줄거임.
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;

        //사용할 UI를 미리 메모리에 올려둔다.
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
            Debug.LogError("OptionSelecter가 Scene에 없습니다링");
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

    //이 친구는 Network WorkerThread에서 실행됨.
    private void GoToMatchMakeRegisterStage() {
        _stage = Stage.MatchmakeRegister;
        Managers.UI.DisableUI("UI_MatchMakeMenu");

        //Managers.UI.ShowSceneUI<>();
    }

    //이 친구는 Network WorkerThread에서 실행됨.
    private void GoToMatchMakeStageViaCancel() {
        Managers.ExecuteAtMainThread(() => { GoToMatchMakeStage(); });
    }

    //이 친구는 Network WorkerThread에서 실행됨.
    private void ConnectToServerFailed() {
        Managers.ExecuteAtMainThread(() => {
            Managers.UI.ShowErrorUIOnlyConfirm("서버와의 연결에 실패했습니다.");
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
        Managers.UI.ShowErrorUIOnlyConfirm("없는 아이디입니다.");
    }

    public void WrongPassword() {
        Managers.UI.ShowErrorUIOnlyConfirm("비밀번호가 맞지 않습니다.");
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
        Managers.UI.ShowErrorUIOnlyConfirm("준비중입니다. ㅠㅠ");
    }
    public void SelectMyRecord() {
        Managers.UI.ShowErrorUIOnlyConfirm("준비중입니다. ㅠㅠ");
    }
    public void SelectOption() {
        Managers.UI.ShowErrorUIOnlyConfirm("준비중입니다. ㅠㅠ");
    }
    public void SelectQuit() {
        QuitApplicationUI();
    }

    private void QuitApplicationUI() {
        Managers.UI.ShowErrorUIConfirmOrCancel("게임을 종료하시겠습니까?", () => {
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
                Managers.UI.ShowErrorUIConfirmOrCancel("로그아웃 하시겠습니까?", Logout);
                break;
            case Stage.MatchMake:
                GoToLobbyStage();
                break;
            case Stage.MatchmakeRegister:
                Managers.UI.ShowErrorUIConfirmOrCancel("띠용", MatchmakeCancel);
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
