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

    //이 친구들은 포인터이다. 가지고 있어도 메모리적으로 손해 조금 보는 정도.
    //C++처럼 생각해서, 해당 객체를 하나 더 들고있는 개념이 아니다.
    private Button _createAccountButton;
    private TMP_InputField _idField;
    private TMP_InputField _pwField;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    private void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
    }

    public override void Init() {
        base.Init();
        Bind<Button>(typeof(Buttons));
        Bind<TMP_InputField>(typeof(InputFields));

        _createAccountButton = Get<Button>((int)Buttons.CreateAccountButton);
        _idField = Get<TMP_InputField>((int)InputFields.IdInputField);
        _pwField = Get<TMP_InputField>((int)InputFields.PasswordInputField);

        if (_createAccountButton != null) {
            _createAccountButton.onClick.AddListener(TryCreateAccount);
        }
        Managers.Input.AddKeyListener(KeyCode.Return, TryCreateAccount, InputManager.KeyState.Down);
    }
    private void TryCreateAccount() {
        //TODO : 패스워드확인이랑 패스워드랑 일치하는지 선제적으로 확인
        //다르면 에러 메세지 출력
        string id = "", password = "";
        if (_idField != null && _pwField != null) {
            Debug.Log($"ID: {_idField.text}");
            Debug.Log($"Password: {_pwField.text}");
            id = _idField.text;
            password = _pwField.text;
        }
        else { return; }
        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("계정생성 시도");
            C_Encrypted pkt = PacketMaker.MakeCCreateAccount(Managers.Network.GetSession(), id, password);
            Managers.Network.Send(pkt);
        }
    }
}
