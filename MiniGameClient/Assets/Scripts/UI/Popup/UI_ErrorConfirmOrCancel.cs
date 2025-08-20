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

    private void OnDestroy() {
        Clear();
    }

    public override void Init() {
        base.Init();
    }

    public void Init(string errorDetail, Action confirmOnClickEvent, Action cancelOnClickEvent) {
        Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        Bind<Button>(typeof(Buttons));
        _errorText = Get<TextMeshProUGUI>((int)Texts.ErrorText);
        _confirmButton = Get<Button>((int)Buttons.ConfirmButton);
        _cancelButton = Get<Button>((int)Buttons.CancelButton);

        _errorText.text = errorDetail;
        Clear();
        _confirmButton.onClick.AddListener(() => {
            confirmOnClickEvent?.Invoke();
            DestroyThis();
        });
        _cancelButton.onClick.AddListener(() => {
            cancelOnClickEvent?.Invoke();
            DestroyThis();
        });
    }

    private void Clear() {
        _confirmButton.onClick.RemoveAllListeners();
        _cancelButton.onClick.RemoveAllListeners();
    }

    private void DestroyThis() {
        GameObject go = this.gameObject;
        Managers.Resource.Destroy(go);
    }
}