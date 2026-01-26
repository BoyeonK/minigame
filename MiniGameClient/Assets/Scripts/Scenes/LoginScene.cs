using Google.Protobuf.Protocol;
using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Threading;
using TMPro;
using Unity.Animations.SpringBones.GameObjectExtensions;
using UnityEngine;

public class LoginScene : BaseScene {
    public enum Stage {
        Connect,
        Login,
        Lobby,
        PersonalRecord,
        PublicRecord,
        Setting,
        MatchMake,
        MatchmakeRegister,
    }

    Stage _stage = Stage.Connect;

    UI_StartGame _uiStartGame;

    UI_LoginOrCreateAccount _uiLoginOrCreateAccount;
    UI_LoginPopup _uiLoginPopup;
    UI_CreateAccountPopup _uiCreateAccountPopup;
    UI_LobbyMenu _uiLobbyMenu;
    UI_MatchMakeMenu _uiMatchMakeMenu;
    UI_PersonalRecord _uiPersonalRecord;
    UI_PublicRecord _uiPublicRecord;
    UI_MatchMakeProgress _uiMatchMakeProgress;
    UI_SettingPopup _uiSettingPopup;

    int _loginOpt = 0;

    int _lobbyOpt = 0;
    OptionSelecterController _optionSelecter;

    int _matchMakeOpt = 0;
    LobbyScreenRenderer _screenRenderer;
    TextMeshPro _gameExplanationText;
    
    //Scene이 바뀔 때, 이 친구가 대표로 나서서 모든 초기화 작업을 해 줄거임.
    protected override void Init() {
        base.Init();
        Screen.SetResolution(848, 477, false);
        SceneType = Define.Scene.Login;
        Managers.Scene.ResetLoadSceneOp();

        //사용할 UI를 미리 메모리에 올려둔다.
        _uiStartGame = Managers.UI.CacheSceneUI<UI_StartGame>();
        _uiLoginOrCreateAccount = Managers.UI.CacheSceneUI<UI_LoginOrCreateAccount>();
        _uiLoginPopup = Managers.UI.CachePopupUI<UI_LoginPopup>();
        _uiCreateAccountPopup = Managers.UI.CachePopupUI<UI_CreateAccountPopup>();
        _uiLobbyMenu = Managers.UI.CacheSceneUI<UI_LobbyMenu>();
        _uiMatchMakeMenu = Managers.UI.CacheSceneUI<UI_MatchMakeMenu>();
        _uiPersonalRecord = Managers.UI.CacheSceneUI<UI_PersonalRecord>();
        _uiPublicRecord = Managers.UI.CacheSceneUI<UI_PublicRecord>();
        _uiMatchMakeProgress = Managers.UI.CachePopupUI<UI_MatchMakeProgress>();
        _uiSettingPopup = Managers.UI.CachePopupUI<UI_SettingPopup>();
        _uiSettingPopup.AddListenerToConfirmBtn(() => { GoToLobbyStage(); });
        _uiSettingPopup.AddListenerToCancelBtn(() => { GoToLobbyStage(); });

        //오디오 설정
        Managers.Setting.ApplyPreviousSceneSetting();
        Managers.Sound.GetOrAddAudioClip("button");
        Managers.Sound.GetOrAddAudioClip("select");
        Managers.Sound.GetOrAddAudioClip("swipe");
        Managers.Sound.Play("LobbyScene", Define.Sound.Bgm);
        
        GameObject screen = GameObject.Find("Screen");
        if (screen != null) {
            _screenRenderer = screen.GetComponent<LobbyScreenRenderer>();
            if (_screenRenderer != null) {
                _screenRenderer.Init();
                _screenRenderer.HideThis();
            }

            screen.FindChildByName("GameExplanation")?.TryGetComponent<TextMeshPro>(out _gameExplanationText);
        }

        GameObject go = GameObject.Find("OptionSelecter");
        if (go != null) {
            _optionSelecter = go.GetComponent<OptionSelecterController>();
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
        Managers.Network.Lobby.OnWrongIdAct += WrongId;
        Managers.Network.Lobby.OnWrongPasswordAct += WrongPassword;
        Managers.Network.Lobby.OnLoginAct += LoginSucceed;
        Managers.Network.Lobby.OnLogoutAct += LogoutSucceed;
        Managers.Network.Match.OnMatchmakeRequestSucceedAct += MatchmakeRequestSucceed;
        Managers.Network.Match.OnMatchmakeCancelSucceedAct += MatchmakeCancelSucceed;
        Managers.Network.Match.OnResponseKeepAliveAct += MatchCompletedReadyToChangeScene;

        if (Managers.Network.IsConnected() || Managers.Network.IsLogined()) {
            Managers.UI.ShowSceneUI<UI_LobbyMenu>();
            _stage = Stage.Lobby;
            Managers.ExecuteAtMainThread(() => { ApplyLobbyOpt(); });
        }
        else {
            Managers.UI.ShowSceneUI<UI_StartGame>();
        }
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
            Managers.Sound.Play("select");
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
            Managers.Sound.Play("select");
            _screenRenderer.ApplyPicture(_matchMakeOpt);
            _uiMatchMakeMenu.SetSelectedOpt(_matchMakeOpt);
            SetExplanationText(_matchMakeOpt);
        }
    }

    private void SetExplanationText(int opt) {
        switch (opt) {
            case 0:
                _gameExplanationText.text = "방향키와 space로 캐릭터를 조작합니다.\n결승점에 먼저 도착하면 승리합니다.";
                break;
            case 1:
                _gameExplanationText.text = "마우스로 막대를 조작합니다.\n나의 게이트에 공이 들어오지 않도록 막으세요.";
                break;
            case 2:
                _gameExplanationText.text = "넘버패드 1~9까지의 번호를 눌러 조작합니다.\n호박을 다른사람보다 빨리 채취해세요.\r\n다른 작물을 채취할 경우 감점됩니다!";
                break;
            default:
                _gameExplanationText.text = "";
                break;
        }
    }

    public void StartMatchMake() {
        if (_stage == Stage.MatchMake) {
            Managers.Sound.Play("button");
            Managers.Network.Match.TryMatchMake(Define.IntToGameType(_matchMakeOpt + 1));
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
        Managers.UI.DisableUI("UI_PersonalRecord");
        Managers.UI.DisableUI("UI_PublicRecord");
        Managers.UI.DisableUI("UI_LoginPopup");
        Managers.UI.DisableUI("UI_LoginOrCreateAccount");
        Managers.UI.DisableUI("UI_MatchMakeMenu");
        Managers.UI.DisableUI("UI_SettingPopup");

        Managers.UI.ShowSceneUI<UI_LobbyMenu>();
    }

    private void GoToMatchMakeStage() {
        _matchMakeOpt = 0;
        _stage = Stage.MatchMake;
        _screenRenderer.ActiveThis();
        Managers.UI.DisableUI("UI_LobbyMenu");
        Managers.UI.DisableUI("UI_MatchMakeProgress");
        //Managers.UI.ShowSceneUI<>();

        Managers.UI.ShowSceneUI<UI_MatchMakeMenu>();
    }

    //이 친구는 Network WorkerThread에서 실행됨.
    private void GoToMatchMakeRegisterStage() {
        _stage = Stage.MatchmakeRegister;
        Managers.UI.DisableUI("UI_MatchMakeMenu");
        Managers.UI.ShowPopupUI<UI_MatchMakeProgress>();
    }

    //이 친구는 Network WorkerThread에서 실행됨.
    private void GoToMatchMakeStageViaCancel() {
        GoToMatchMakeStage();
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
        BackToPreviousMenu();
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
        //TODO : 현재 이 함수를 받는 상황이 반드시 MatchmakeStage라는 가정이 깔려있음.
        GoToMatchMakeRegisterStage();
    }

    public void MatchmakeCancelSucceed() {
        //TODO : 현재 이 함수를 받는 상황이 반드시 MatchmakeRegisterStage라는 가정이
        //어느정도 깔려있음. 매치 진행중의 경우 다른 stage로 전환할 수 없도록 해야함.
        GoToMatchMakeStageViaCancel();
    }

    public void MatchCompletedReadyToChangeScene() {
        //Managers.UI.ShowErrorUIOnlyConfirm("매치메이킹 완료됨");
    }

    public void SelectLeaderboard() {
        _stage = Stage.PublicRecord;
        Managers.UI.DisableUI("UI_LobbyMenu");
        Managers.UI.ShowSceneUI<UI_PublicRecord>();
    }

    public void SelectMyRecord() {
        _stage = Stage.PersonalRecord;
        Managers.UI.DisableUI("UI_LobbyMenu");
        Managers.UI.ShowSceneUI<UI_PersonalRecord>();
    }

    public void SelectOption() {
        _stage = Stage.Setting;
        Managers.UI.DisableUI("UI_LobbyMenu");
        Managers.UI.ShowPopupUI<UI_SettingPopup>();
    }

    public void SelectQuit() {
        QuitApplicationUI();
    }

    private void QuitApplicationUI() {
        Managers.UI.ShowErrorUIConfirmOrCancel("게임을 종료하시겠습니까?", () => {
            Managers.Network.TryDisconnect();
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
            case Stage.PublicRecord:
                GoToLobbyStage();
                break;
            case Stage.PersonalRecord:
                GoToLobbyStage();
                break;
            case Stage.Setting:
                GoToLobbyStage();
                break;
            case Stage.MatchMake:
                GoToLobbyStage();
                break;
            case Stage.MatchmakeRegister:
                Managers.UI.ShowErrorUIConfirmOrCancel("현재 매치를 취소하시겠습니까?", MatchmakeCancel);
                break;
            default:
                break;
        }
    }

    public void Logout() {
        GoToLoginStage();
        Managers.Network.Lobby.TryLogout();
    }

    public void MatchmakeCancel() {
        Managers.Network.Match.TryMatchMakeCancel();
    }

    public void SetPersonalRecord() {
        _uiPersonalRecord.BindRecord();
    }

    public void SetPublicRecord() {
        _uiPublicRecord.BindRecord();
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
        Managers.Network.Lobby.OnWrongIdAct -= WrongId;
        Managers.Network.Lobby.OnWrongPasswordAct -= WrongPassword;
        Managers.Network.Lobby.OnLoginAct -= LoginSucceed;
        Managers.Network.Lobby.OnLogoutAct -= LogoutSucceed;
        Managers.Network.Match.OnMatchmakeRequestSucceedAct -= MatchmakeRequestSucceed;
        Managers.Network.Match.OnMatchmakeCancelSucceedAct -= MatchmakeCancelSucceed;
        Managers.Network.Match.OnResponseKeepAliveAct -= MatchCompletedReadyToChangeScene;
    }
}
