using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_MatchMakeMenu : UI_Scene {
    enum Texts {
        Pingpong,
        Danmaku,
        VampireSurvival,
    }

    LoginScene _loginScene;

    private TextMeshProUGUI _vampireSurvival;
    private TextMeshProUGUI _pingpong;
    private TextMeshProUGUI _danmaku;

    private RectTransform _rectVampireSurvival;
    private RectTransform _rectPingpong;
    private RectTransform _rectDanmaku;

    private UI_EventHandler _vampireSurvivalEventHandler;
    private UI_EventHandler _pingpongEventHandler;
    private UI_EventHandler _danmakuEventHandler;
    
    private float _rectSpeed = 5f;
    private float _range = 6f;

    private Color _unSelectedColor = Color.white;
    private Color _selectedColor = new Color(128f / 255f, 0f, 0f, 1.0f);

    private int _selectedOpt;

    public void SetSelectedOpt(int value) {
        _selectedOpt = value;
        if (_rectPingpong == null ||
            _rectDanmaku == null ||
            _rectVampireSurvival == null)
        {
            return;
        }
        _rectPingpong.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _rectDanmaku.localRotation = Quaternion.Euler(0f, 0f, 0f);
        _rectVampireSurvival.localRotation = Quaternion.Euler(0f, 0f, 0f);

        _pingpong.color = _unSelectedColor;
        _danmaku.color = _unSelectedColor;
        _vampireSurvival.color = _unSelectedColor;

        switch (_selectedOpt) {
            case 0:
                _vampireSurvival.color = _selectedColor;
                break;
            case 1:
                _pingpong.color = _selectedColor;
                break;
            case 2:
                _danmaku.color = _selectedColor;
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
                _rectVampireSurvival.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
            case 1:
                _rectPingpong.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;
            case 2:
                _rectDanmaku.localRotation = Quaternion.Euler(0f, 0f, rot);
                break;   
        }
    }

    public override void Init() {
        base.Init();
        GameObject go = GameObject.Find("LoginScene");
        _loginScene = go.GetComponent<LoginScene>();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _vampireSurvival = Get<TextMeshProUGUI>((int)Texts.VampireSurvival);
        _pingpong = Get<TextMeshProUGUI>((int)Texts.Pingpong);
        _danmaku = Get<TextMeshProUGUI>((int)Texts.Danmaku);
        
        if (_vampireSurvival != null) {
            _rectVampireSurvival = _vampireSurvival.GetComponent<RectTransform>();
            _vampireSurvivalEventHandler = _vampireSurvival.GetComponent<UI_EventHandler>();
            if (_vampireSurvivalEventHandler != null) {
                _vampireSurvivalEventHandler.Clear();
                _vampireSurvivalEventHandler.OnClickHandler += (PointerEventData data) => {
                    _loginScene.SetMatchMakeOpt(0);
                    _loginScene.StartMatchMake();
                };
                _vampireSurvivalEventHandler.OnPointerEnterHandler += (PointerEventData data) => {
                    _loginScene.SetMatchMakeOpt(0);
                };
            }
        }
        if (_pingpong != null) {
            _rectPingpong = _pingpong.GetComponent<RectTransform>();
            _pingpongEventHandler = _pingpong.GetComponent<UI_EventHandler>();
            if (_pingpongEventHandler != null) {
                _pingpongEventHandler.Clear();
                _pingpongEventHandler.OnClickHandler += (PointerEventData data) => {
                    _loginScene.SetMatchMakeOpt(1);
                    _loginScene.StartMatchMake();
                };
                _pingpongEventHandler.OnPointerEnterHandler += (PointerEventData data) => {
                    _loginScene.SetMatchMakeOpt(1);
                };
            }
        }
        if (_danmaku != null) {
            _rectDanmaku = _danmaku.GetComponent<RectTransform>();
            _danmakuEventHandler = _danmaku.GetComponent<UI_EventHandler>();
            if (_danmakuEventHandler != null) {
                _danmakuEventHandler.Clear();
                _danmakuEventHandler.OnClickHandler += (PointerEventData data) => {
                    _loginScene.SetMatchMakeOpt(2);
                    _loginScene.StartMatchMake();
                };
                _danmakuEventHandler.OnPointerEnterHandler += (PointerEventData data) => {
                    _loginScene.SetMatchMakeOpt(2);
                };
            }
        }
       
        Managers.Input.AddKeyListener(KeyCode.Return, _loginScene.StartMatchMake, InputManager.KeyState.Down);
        SetSelectedOpt(0);
    }

    private void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Return, _loginScene.StartMatchMake, InputManager.KeyState.Down);
    }
}
