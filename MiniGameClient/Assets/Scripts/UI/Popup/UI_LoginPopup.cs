using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_LoginPopup : UI_Popup {
    enum InputFields {
        IdInputField,
        PasswordInputField
    }

    enum Buttons {
        LoginButton
    }

    //�� ģ������ �������̴�. ������ �־ �޸������� ���� ���� ���� ����.
    //C++ó�� �����ؼ�, �ش� ��ü�� �ϳ� �� �޸𸮿� �ε�Ǵ� ������ �ƴϴ�.
    private Button _loginButton;
    private TMP_InputField _idField;
    private TMP_InputField _pwField;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    private void Clear() {
        _loginButton.onClick.RemoveAllListeners();
        Managers.Input.RemoveKeyListener(KeyCode.Return, TryLogin, InputManager.KeyState.Down);
    }

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
        if (_pwField != null) {
            _pwField.contentType = TMP_InputField.ContentType.Alphanumeric;
            _pwField.inputType = TMP_InputField.InputType.Password;
        }
        Managers.Input.AddKeyListener(KeyCode.Return, TryLogin, InputManager.KeyState.Down);
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

        if (id == "" || password == "") {
            Managers.UI.ShowErrorUIOnlyConfirm("�Է°��� �߸��Ǿ����ϴ�.", () => { });
            return;
        }

        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("�α��� �õ�");
            C_Encrypted pkt = PacketMaker.MakeCLogin(Managers.Network.GetSession(), id, password);
            Managers.Network.Send(pkt);
        }
    }
}
