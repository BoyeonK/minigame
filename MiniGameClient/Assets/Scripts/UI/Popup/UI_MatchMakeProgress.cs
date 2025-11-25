using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_MatchMakeProgress : UI_Popup {
    enum Texts {
        Minute,
        Second,
    }

    enum Buttons {
        CancelButton
    }

    LoginScene _loginScene;

    float _initTime = 0;
    int _progressTimeSecond = -1;

    private Button _cancelButton;
    private TextMeshProUGUI _minute;
    private TextMeshProUGUI _second;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    private void Update() {
        float elapsedTime = Time.time - _initTime;
        int totalSeconds = Mathf.FloorToInt(elapsedTime);

        if (totalSeconds != _progressTimeSecond) {
            _progressTimeSecond = totalSeconds;
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;
            if (_minute != null)
                _minute.text = $"{minutes:D2}";
            if (_second != null)
                _second.text = $"{seconds:D2}";
        }
    }

    private void Clear() {
        _cancelButton.onClick.RemoveAllListeners();
    }

    public override void Init() {
        base.Init();
        Bind<Button>(typeof(Buttons));
        Bind<TextMeshProUGUI>(typeof(Texts));

        _cancelButton = Get<Button>((int)Buttons.CancelButton);
        _minute = Get<TextMeshProUGUI>((int)Texts.Minute);
        _second = Get<TextMeshProUGUI>((int)Texts.Second);

        _initTime = Time.time;
        _progressTimeSecond = -1;
        if (Managers.Scene.CurrentScene is LoginScene loginScene)
            _loginScene = loginScene;

        if (_cancelButton != null) {
            _cancelButton.onClick.AddListener(OnCancelButton);
        }
    }

    public void OnCancelButton() {
        _loginScene.BackToPreviousMenu();
    }
}
