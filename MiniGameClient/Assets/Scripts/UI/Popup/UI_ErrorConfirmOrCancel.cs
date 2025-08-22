using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_ErrorConfirmOrCancel : UI_Popup {
    enum Texts {
        ErrorText
    }

    enum Buttons {
        ConfirmButton,
        CancelButton
    }

    private TextMeshProUGUI _errorText;
    private Button _confirmButton;
    private Button _cancelButton;

    Action _confirmAct;
    Action _cancelAct;

    private void OnDestroy() {
        Clear();
        Managers.Input.DecrementCounter();
    }

    public override void Init() {
        base.Init();
    }

    public void Init(string errorDetail, Action confirmOnClickEvent, Action cancelOnClickEvent) {
        Init();
        Managers.Input.IncrementCounter();
        Bind<TextMeshProUGUI>(typeof(Texts));
        Bind<Button>(typeof(Buttons));
        _errorText = Get<TextMeshProUGUI>((int)Texts.ErrorText);
        _confirmButton = Get<Button>((int)Buttons.ConfirmButton);
        _cancelButton = Get<Button>((int)Buttons.CancelButton);

        _errorText.text = errorDetail;
        _confirmButton.onClick.RemoveAllListeners();
        _cancelButton.onClick.RemoveAllListeners();
        _confirmAct += confirmOnClickEvent;
        _confirmAct += DestroyThis;
        _cancelAct += cancelOnClickEvent;
        _cancelAct += DestroyThis;
        _confirmButton.onClick.AddListener(() => {
            _confirmAct?.Invoke();
        });
        _cancelButton.onClick.AddListener(() => {
            _cancelAct?.Invoke();
        });
    }

    private void Update() {
        if (Input.GetKeyDown(KeyCode.Return)) {
            _confirmAct?.Invoke();
        }
        if (Input.GetKeyDown(KeyCode.Escape)) {
            _cancelAct?.Invoke();
        }
    }

    private void Clear() {
        _confirmButton.onClick.RemoveAllListeners();
        _cancelButton.onClick.RemoveAllListeners();
    }

    private void DestroyThis() {
        Clear();
        GameObject go = this.gameObject;
        Managers.Resource.Destroy(go);
    }
}