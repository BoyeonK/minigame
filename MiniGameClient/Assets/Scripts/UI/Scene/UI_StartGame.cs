using TMPro;
using UnityEngine;

public class UI_StartGame : UI_Scene {
    enum Texts {
        GameName,
        Enter,
        To_Start,
        Connecting
    }

    enum GameObjects {
        EnterToStart
    }

    private TextMeshProUGUI _gameName;
    private TextMeshProUGUI _enter;
    private TextMeshProUGUI _toStart;
    private TextMeshProUGUI _connecting;
    private GameObject _enterToStart;

    private RectTransform _rectGameName;
    private float _minRectGameNamez = 7.5f;
    private float _rectSpeed = 2.5f;

    private RectTransform _rectEnterToStart;
    private float _minEnterPosY = 87.5f;
    private float _enterPosYSpeed = 2.0f;

    private float _range = 2.5f;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    void Update() {
        if (_gameName != null && _rectGameName != null) {
            float pingPongValue = Mathf.PingPong(Time.time * _rectSpeed, _range);
            float rot = -(_minRectGameNamez + pingPongValue);
            _rectGameName.localRotation = Quaternion.Euler(0f, 0f, rot);
        }

        if (_enterToStart != null) {
            Vector2 pos = _rectEnterToStart.anchoredPosition;
            float pingPongValue = Mathf.PingPong(Time.time * _enterPosYSpeed, 1.3f * _range);
            float posY = -(_minEnterPosY + pingPongValue);
            pos.y = posY;
            _rectEnterToStart.anchoredPosition = pos;
        }
    }

    public override void Init() {
        base.Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        Bind<GameObject>(typeof(GameObjects));
        _gameName = Get<TextMeshProUGUI>((int)Texts.GameName);
        _enterToStart = Get<GameObject>((int)GameObjects.EnterToStart);
        _enter = Get<TextMeshProUGUI>((int)Texts.Enter);
        _toStart = Get<TextMeshProUGUI>((int)Texts.To_Start);
        _connecting = Get<TextMeshProUGUI>((int)Texts.Connecting);
        if (_connecting != null) {
            Color cc = _connecting.color;
            cc.a = 0f;
            _connecting.color = cc;
        }
        if (_gameName != null) { 
            _rectGameName = _gameName.GetComponent<RectTransform>();
        }
        if (_enterToStart != null) {
            _rectEnterToStart = _enterToStart.GetComponent<RectTransform>();
        }

        Managers.Input.AddKeyListener(KeyCode.Return, TryConnectToServer, InputManager.KeyState.Up);
    }

    private void TryConnectToServer() {
        if (!(Managers.Network.IsConnected())) {
            Managers.Network.TryConnectToServer();
            OnConnecting();
        }
    }

    private void OnConnecting() {
        if (_enter != null && _toStart != null) {
            Color ec = _enter.color;
            Color tc = _toStart.color;
            ec.a = 0f;
            tc.a = 0f;
            _enter.color = ec;
            _toStart.color = tc;
        }
        if (_connecting != null) {
            Color cc = _connecting.color;
            cc.a = 1f;
            _connecting.color = cc;
        }
    }

    private void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Return, TryConnectToServer, InputManager.KeyState.Up);
    }
}
