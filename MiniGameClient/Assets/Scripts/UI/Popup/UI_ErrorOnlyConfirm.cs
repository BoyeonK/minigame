using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_ErrorOnlyConfirm : UI_Popup { 
    enum Texts {
        ErrorText
    }

    enum Buttons {
        ConfirmButton
    }

    private TextMeshProUGUI _errorText;
    private Button _confirmButton;

    Action _confirmAct;

    private void OnDestroy() {
        Clear();
        Managers.Input.DecrementCounter();
    }

    public override void Init() {
        base.Init();
    }

    public void Init(string errorDetail, Action confirmOnClickEvent) {
        Init();
        Managers.Input.IncrementCounter();
        Bind<TextMeshProUGUI>(typeof(Texts));
        Bind<Button>(typeof(Buttons));
        _errorText = Get<TextMeshProUGUI>((int)Texts.ErrorText);
        _confirmButton = Get<Button>((int)Buttons.ConfirmButton);

        _errorText.text = errorDetail;
        _confirmButton.onClick.RemoveAllListeners();
        _confirmAct += confirmOnClickEvent;
        _confirmAct += DestroyThis;
        _confirmButton.onClick.AddListener(() => {
            _confirmAct?.Invoke();
        });
    }

    private void Update() {
        if (Input.GetKeyDown(KeyCode.Return)) {
            _confirmAct?.Invoke();
        }
    }

    private void Clear() {
        _confirmAct = null;
        _confirmButton.onClick.RemoveAllListeners();
    }

    private void DestroyThis() {
        Clear();
        GameObject go = this.gameObject;
        Managers.Resource.Destroy(go);
    }
}
