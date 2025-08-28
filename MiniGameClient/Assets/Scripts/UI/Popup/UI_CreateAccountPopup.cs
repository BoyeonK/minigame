using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_CreateAccountPopup : UI_Popup {
    enum InputFields {
        IdInputField,
        PasswordInputField,
        PasswordConfirmInputField,
    }

    enum Buttons {
        CreateAccountButton
    }

    //이 친구들은 포인터이다. 가지고 있어도 메모리적으로 손해 조금 보는 정도.
    //C++처럼 생각해서, 해당 객체를 하나 더 들고있는 개념이 아니다.
    private Button _createAccountButton;
    private TMP_InputField _idField;
    private TMP_InputField _pwField;
    private TMP_InputField _pwConfirmField;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    private void Clear() {
        _createAccountButton.onClick.RemoveAllListeners();
        Managers.Input.RemoveKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.Tab, FocusToNxt, InputManager.KeyState.Down);
    }

    public override void Init() {
        base.Init();
        Bind<Button>(typeof(Buttons));
        Bind<TMP_InputField>(typeof(InputFields));

        _createAccountButton = Get<Button>((int)Buttons.CreateAccountButton);
        _idField = Get<TMP_InputField>((int)InputFields.IdInputField);
        _pwField = Get<TMP_InputField>((int)InputFields.PasswordInputField);
        _pwConfirmField = Get<TMP_InputField>((int)InputFields.PasswordConfirmInputField);

        if (_createAccountButton != null) {
            _createAccountButton.onClick.AddListener(TryCreateAccount);
        }
        if (_pwField != null) {
            _pwField.contentType = TMP_InputField.ContentType.Alphanumeric;
            _pwField.inputType = TMP_InputField.InputType.Password;
        }
        if (_pwConfirmField != null) {
            _pwConfirmField.contentType = TMP_InputField.ContentType.Alphanumeric;
            _pwConfirmField.inputType = TMP_InputField.InputType.Password;
        }
        Managers.Input.AddKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.Tab, FocusToNxt, InputManager.KeyState.Down);
    }

    private void TryCreateAccount() {
        //TODO : 패스워드확인이랑 패스워드랑 일치하는지 선제적으로 확인
        //다르면 에러 메세지 출력
        string id = "", pw = "", pwc = "";
        if (_idField != null && _pwField != null && _pwConfirmField != null) {
            id = _idField.text;
            pw = _pwField.text;
            pwc = _pwConfirmField.text;
        }
        else { return; }

        Managers.Network.TryCreateAccount(id, pw, pwc);
    }

    private void FocusToNxt() {
        GameObject current = EventSystem.current.currentSelectedGameObject;
        if (current == null) return;

        if (current == _idField.gameObject)
            _pwField.Select();
        else if (current == _pwField.gameObject)
            _pwConfirmField.Select();
    }
}
