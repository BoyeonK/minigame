using TMPro;
using UnityEngine;

public class UI_StartGame : UI_Scene {
    enum Texts {
        GameName,
        Enter,
        To_Start,
        Connecting
    }

    private TextMeshProUGUI _gameName;
    private TextMeshProUGUI _enter;
    private TextMeshProUGUI _toStart;
    private TextMeshProUGUI _connecting;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    void Update() {

    }

    public override void Init() {
        base.Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _gameName = Get<TextMeshProUGUI>((int)Texts.GameName);
        _enter = Get<TextMeshProUGUI>((int)Texts.Enter);
        _toStart = Get<TextMeshProUGUI>((int)Texts.To_Start);
        _connecting = Get<TextMeshProUGUI>((int)Texts.Connecting);
        if (_connecting != null) {
            Color cc = _connecting.color;
            cc.a = 0f;
            _connecting.color = cc;
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
