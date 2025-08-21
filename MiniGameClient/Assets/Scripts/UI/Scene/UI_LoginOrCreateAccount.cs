using TMPro;
using UnityEngine;

public class UI_LoginOrCreateAccount : UI_Scene {
    enum Texts {
        LoginMenu,
        CreateAccountMenu,
    }

    private TextMeshProUGUI _loginMenu;
    private TextMeshProUGUI _createAccountMenu;

    private RectTransform _rectLogin;
    private RectTransform _rectCreateAccount;

    private float _rectSpeed = 5f;
    private float _range = 6f;

    private Color _unSelectedColor = Color.white;
    private Color _selectedColor = new Color(128f / 255f, 0f, 0f, 1.0f);

    private int _selectedOpt;

    public void SetSelectedOpt(int value) {
        _selectedOpt = value;
        if (_selectedOpt == 0) {
            if (_rectCreateAccount != null) {
                _rectCreateAccount.localRotation = Quaternion.Euler(0f, 0f, 0f);
                _loginMenu.color = _selectedColor;
                _createAccountMenu.color = _unSelectedColor;
            }   
        } else  {
            if (_rectLogin != null) {
                _rectLogin.localRotation = Quaternion.Euler(0f, 0f, 0f);
                _loginMenu.color = _unSelectedColor;
                _createAccountMenu.color = _selectedColor;
            }    
        }
    }

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    void Update() {
        float pingPongValue = Mathf.PingPong(Time.time * _rectSpeed, _range);
        float rot = -3f + pingPongValue;
        if (_selectedOpt == 0 && _loginMenu != null && _rectLogin != null) {
            _rectLogin.localRotation = Quaternion.Euler(0f, 0f, rot);
        }
        if (_selectedOpt == 1 && _createAccountMenu != null && _rectCreateAccount != null) {
            _rectCreateAccount.localRotation = Quaternion.Euler(0f, 0f, rot);
        }
    }

    public override void Init() {
        base.Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _loginMenu = Get<TextMeshProUGUI>((int)Texts.LoginMenu);
        _createAccountMenu = Get<TextMeshProUGUI>((int)Texts.CreateAccountMenu);
        if (_loginMenu != null) {
            _rectLogin = _loginMenu.GetComponent<RectTransform>();
        }
        if (_createAccountMenu != null) {
            _rectCreateAccount = _createAccountMenu.GetComponent<RectTransform>();
        }
        SetSelectedOpt(0);
    }

    private void Clear() {

    }
}
