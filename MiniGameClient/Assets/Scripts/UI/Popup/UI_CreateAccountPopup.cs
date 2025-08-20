using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
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

    //�� ģ������ �������̴�. ������ �־ �޸������� ���� ���� ���� ����.
    //C++ó�� �����ؼ�, �ش� ��ü�� �ϳ� �� ����ִ� ������ �ƴϴ�.
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
        Managers.Input.AddKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
    }
    private void TryCreateAccount() {
        //TODO : �н�����Ȯ���̶� �н������ ��ġ�ϴ��� ���������� Ȯ��
        //�ٸ��� ���� �޼��� ���
        string id = "", pw = "", pwc = "";
        if (_idField != null && _pwField != null && _pwConfirmField != null) {
            id = _idField.text;
            pw = _pwField.text;
            pwc = _pwConfirmField.text;
        }
        else { return; }

        if (id == "" || pw == "" || pwc == "" || pw != pwc) {
            Managers.UI.ShowErrorUIOnlyConfirm("�Է°��� �߸��Ǿ����ϴ�.", () => { });
            return;
        }

        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("�������� �õ�");
            C_Encrypted pkt = PacketMaker.MakeCCreateAccount(Managers.Network.GetSession(), id, pw);
            Managers.Network.Send(pkt);
        }
    }
}
