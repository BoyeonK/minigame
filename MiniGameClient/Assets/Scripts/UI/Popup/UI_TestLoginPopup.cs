using Google.Protobuf.Protocol;
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

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    //�� ģ������ �������̴�. ������ �־ �޸������� ���� ���� ���� ����.
    //C++ó�� �����ؼ�, �ش� ��ü�� �ϳ� �� ����ִ� ������ �ƴϴ�.
    private Button _loginButton;
    private TMP_InputField _idField;
    private TMP_InputField _pwField;

    public override void Init() {
        base.Init();
        Bind<Button>(typeof(Buttons));
        Bind<TMP_InputField>(typeof(InputFields));

        _loginButton = Get<Button>((int)Buttons.LoginButton);
        _idField = Get<TMP_InputField>((int)InputFields.IdInputField);
        _pwField = Get<TMP_InputField>((int)InputFields.PasswordInputField);

        if (_loginButton != null) {
            _loginButton.onClick.AddListener(TryLogin);
        }

        Managers.Input.AddKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
    }

    private void Clear() {
        if (_loginButton != null) {
            _loginButton.onClick.RemoveListener(TryLogin);
        }

        Managers.Input.RemoveKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
    }

    private void TryLogin() {
        string id = "", password = "";
        if (_idField != null && _pwField != null) {
            Debug.Log($"ID: {_idField.text}");
            Debug.Log($"Password: {_pwField.text}");
            id = _idField.text;
            password = _pwField.text;
        }
        else { return; }
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("�α��� �õ�");
            C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), id, password);
            Managers.Network.Send(pkt);
        }
    }

    private void TryCreateAccount() {
        string id = "", password = "";
        if (_idField != null && _pwField != null) {
            Debug.Log($"ID: {_idField.text}");
            Debug.Log($"Password: {_pwField.text}");
            id = _idField.text;
            password = _pwField.text;
        }
        else { return; }
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("�������� �õ�");
            C_Encrypted pkt = PacketMaker.MakeCCreateAccount(Managers.Network.GetSession(), id, password);
            Managers.Network.Send(pkt);
        }
    }
}
