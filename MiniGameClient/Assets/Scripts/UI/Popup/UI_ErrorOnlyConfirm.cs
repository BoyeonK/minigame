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

    private void OnDestroy() {
        Clear();
    }

    public override void Init() {
        base.Init();
    }

    public void Init(string errorDetail, Action confirmOnClickEvent) {
        Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        Bind<Button>(typeof(Buttons));
        _errorText = Get<TextMeshProUGUI>((int)Texts.ErrorText);
        _confirmButton = Get<Button>((int)Buttons.ConfirmButton);

        _errorText.text = errorDetail;
        _confirmButton.onClick.RemoveAllListeners();
        _confirmButton.onClick.AddListener(() => {
            confirmOnClickEvent?.Invoke();
            DestroyThis();
        });
    }

    private void Clear() {
        _confirmButton.onClick.RemoveAllListeners();
    }

    private void DestroyThis() {
        GameObject go = this.gameObject;
        Managers.Resource.Destroy(go);
    }
}
