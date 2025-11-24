using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_LobbyMenu : UI_Scene { 
    enum Texts {
        StartGame,
        Leaderboard,
        MyRecord,
        Option,
        Quit,
    }

    LoginScene _loginScene;

    private TextMeshProUGUI _startGame;
    private TextMeshProUGUI _leaderboard;
    private TextMeshProUGUI _myRecord;
    private TextMeshProUGUI _option;
    private TextMeshProUGUI _quit;

    private RectTransform _rectStartGame;
    private RectTransform _rectLeaderboard;
    private RectTransform _rectMyRecord;
    private RectTransform _rectOption;
    private RectTransform _rectQuit;

    private UI_EventHandler _startGameEventHandler;
    private UI_EventHandler _leaderboardEventHandler;
    private UI_EventHandler _myRecordEventHandler;
    private UI_EventHandler _optionEventHandler;
    private UI_EventHandler _quitEventHandler;

    private float _rectSpeed = 5f;
    private float _range = 6f;

    private Color _unSelectedColor = Color.white;
    private Color _selectedColor = new Color(128f / 255f, 0f, 0f, 1.0f);

    private int _selectedOpt;

    public void SetSelectedOpt(int value) {
        _selectedOpt = value;
        if (_rectStartGame == null || 
            _rectLeaderboard == null || 
            _rectMyRecord == null || 
            _rectOption == null || 
            _rectQuit == null)
        {
            return;
        }
        _rectStartGame.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _rectLeaderboard.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _rectMyRecord.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _rectOption.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _rectQuit.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _startGame.color = _unSelectedColor;
        _leaderboard.color = _unSelectedColor;
        _myRecord.color = _unSelectedColor;
        _option.color = _unSelectedColor;
        _quit.color = _unSelectedColor;

        switch (_selectedOpt) {
            case 0:
                _startGame.color = _selectedColor;
                break;
            case 1:
                _leaderboard.color = _selectedColor;
                break;
            case 2:
                _myRecord.color = _selectedColor;
                break;
            case 3:
                _option.color = _selectedColor;
                break;
            case 4:
                _quit.color = _selectedColor;
                break;
        }
    }

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    void Update() {
        float pingPongValue = Mathf.PingPong(Time.time * _rectSpeed, _range);
        float rot = -3f + pingPongValue;
        switch (_selectedOpt) {
            case 0:
                _rectStartGame.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
            case 1:
                _rectLeaderboard.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
            case 2:
                _rectMyRecord.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
            case 3:
                _rectOption.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
            case 4:
                _rectQuit.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
        }
    }

    private void SelectPresentOption() {
        switch (_selectedOpt) {
            case 0:
                _loginScene.SelectStartGame();
                break;
            case 1:
                _loginScene.SelectLeaderboard();
                break;
            case 2:
                _loginScene.SelectMyRecord();
                break;
            case 3:
                _loginScene.SelectOption();
                break;
            case 4:
                _loginScene.SelectQuit();
                break;
        }
    }

    public override void Init() {
        base.Init();
        GameObject go = GameObject.Find("LoginScene");
        _loginScene = go.GetComponent<LoginScene>();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _startGame = Get<TextMeshProUGUI>((int)Texts.StartGame);
        _leaderboard = Get<TextMeshProUGUI>((int)Texts.Leaderboard);
        _myRecord = Get<TextMeshProUGUI>((int)Texts.MyRecord);
        _option = Get<TextMeshProUGUI>((int)Texts.Option);
        _quit = Get<TextMeshProUGUI>((int)Texts.Quit);
        if (_startGame != null) {
            _rectStartGame = _startGame.GetComponent<RectTransform>();
            _startGameEventHandler = _startGame.GetComponent<UI_EventHandler>();
            if (_startGameEventHandler != null) {
                _startGameEventHandler.Clear();
                _startGameEventHandler.OnClickHandler += (PointerEventData data) => { 
                    _loginScene.SetLobbyOpt(0);
                    _loginScene.SelectStartGame();
                };
                _startGameEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLobbyOpt(0); };
            }
        }
        if (_leaderboard != null) {
            _rectLeaderboard = _leaderboard.GetComponent<RectTransform>();
            _leaderboardEventHandler = _leaderboard.GetComponent<UI_EventHandler>();
            if (_leaderboardEventHandler != null) {
                _leaderboardEventHandler.Clear();
                _leaderboardEventHandler.OnClickHandler += (PointerEventData data) => { 
                    _loginScene.SetLobbyOpt(1);
                    _loginScene.SelectLeaderboard();
                };
                _leaderboardEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLobbyOpt(1); };
            }
        }
        if (_myRecord != null) {
            _rectMyRecord = _myRecord.GetComponent<RectTransform>();
            _myRecordEventHandler = _myRecord.GetComponent<UI_EventHandler>();
            if (_myRecordEventHandler != null) {
                _myRecordEventHandler.Clear();
                _myRecordEventHandler.OnClickHandler += (PointerEventData data) => { 
                    _loginScene.SetLobbyOpt(2);
                    _loginScene.SelectMyRecord();
                };
                _myRecordEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLobbyOpt(2); };
            }
        }
        if (_option != null) {
            _rectOption = _option.GetComponent<RectTransform>();
            _optionEventHandler = _option.GetComponent<UI_EventHandler>();
            if (_optionEventHandler != null) {
                _optionEventHandler.Clear();
                _optionEventHandler.OnClickHandler += (PointerEventData data) => {
                    _loginScene.SetLobbyOpt(3);
                    _loginScene.SelectOption();
                };
                _optionEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLobbyOpt(3); };
            }
        }
        if (_quit != null) {
            _rectQuit = _quit.GetComponent<RectTransform>();
            _quitEventHandler = _quit.GetComponent<UI_EventHandler>();
            if (_quitEventHandler != null) {
                _quitEventHandler.Clear();
                _quitEventHandler.OnClickHandler += (PointerEventData data) => { 
                    _loginScene.SetLobbyOpt(4);
                    _loginScene.SelectQuit();
                };
                _quitEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLobbyOpt(4); };
            }
        }
        Managers.Input.AddKeyListener(KeyCode.Return, SelectPresentOption, InputManager.KeyState.Down);
        SetSelectedOpt(0);
    }

    private void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Return, SelectPresentOption, InputManager.KeyState.Down);
    }
}
