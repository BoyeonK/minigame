using TMPro;
using UnityEngine;

public class UI_LobbyMenu : UI_Scene { 
    enum Texts {
        StartGame,
        Leaderboard,
        MyRecord,
        Option,
        Quit,
    }

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

    private float _rectSpeed = 5f;
    private float _range = 6f;

    private Color _unSelectedColor = Color.white;
    private Color _selectedColor = new Color(128f / 255f, 0f, 0f, 1.0f);

    private int _selectedOpt;

    public void SetSelectedOpt(int value) {
        _selectedOpt = value;
        Debug.Log(_selectedOpt);
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

    public override void Init() {
        base.Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _startGame = Get<TextMeshProUGUI>((int)Texts.StartGame);
        _leaderboard = Get<TextMeshProUGUI>((int)Texts.Leaderboard);
        _myRecord = Get<TextMeshProUGUI>((int)Texts.MyRecord);
        _option = Get<TextMeshProUGUI>((int)Texts.Option);
        _quit = Get<TextMeshProUGUI>((int)Texts.Quit);
        if (_startGame != null) {
            _rectStartGame = _startGame.GetComponent<RectTransform>();
        }
        if (_leaderboard != null) {
            _rectLeaderboard = _leaderboard.GetComponent<RectTransform>();
        }
        if (_myRecord != null) {
            _rectMyRecord = _myRecord.GetComponent<RectTransform>();
        }
        if (_option != null) {
            _rectOption = _option.GetComponent<RectTransform>();
        }
        if (_quit != null) {
            _rectQuit = _quit.GetComponent<RectTransform>();
        }
        SetSelectedOpt(0);
    }

    private void Clear()
    {

    }
}
