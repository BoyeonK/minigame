using System;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_TestLoginPopup : UI_Popup {

    enum InputFields {
        IdInputField,
        PasswordInputField
    }

    enum Buttons {
        LoginButton
    }

    private void Start() {
        Init();
    }

    public override void Init() {
        base.Init();
        Bind<Button>(typeof(Buttons));
        Bind<TMP_InputField>(typeof(InputFields));

        Button Btn = GetButton((int)Buttons.LoginButton);
        if (Btn != null) {
            Btn.onClick.AddListener(OnButtonClicked);
        }
    }

    private void OnButtonClicked() {
        TMP_InputField idf = Get<TMP_InputField>(0);
        TMP_InputField pwf = Get<TMP_InputField>(1);
        Debug.Log(idf.text);
        Debug.Log(pwf.text);
    }
}
