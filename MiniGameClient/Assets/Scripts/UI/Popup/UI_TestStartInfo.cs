using UnityEngine;
using TMPro;

public class UI_TestStartInfo : UI_Popup {
    enum Texts {
        InfoText
    }

    private TextMeshProUGUI _infoText;
    private float minAlpha = 0.45f;
    private float range = 0.36f;
    private float speed = (0.36f) / 1.0f;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    void Update() {
        if (_infoText == null)
            return;
        float pingPongValue = Mathf.PingPong(Time.time * speed, range);
        float alpha = minAlpha + pingPongValue;
        _infoText.color = new Color(225f / 255f, 0, 0, alpha);
    }

    public override void Init() {
        base.Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _infoText = Get<TextMeshProUGUI>((int)Texts.InfoText);
        if (_infoText != null) {
            _infoText.text = "Press Enter To Start..";
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
        if (_infoText != null) {
            _infoText.text = "Connecting To Server..";
        }   
    }

    private void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Return, TryConnectToServer, InputManager.KeyState.Up);
    }
}
