using System;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_LoginOrCreateAccount : UI_Scene {
    enum Texts {
        LoginMenu,
        CreateAccountMenu,
    }

    LoginScene _loginScene;

    private TextMeshProUGUI _loginMenu;
    private TextMeshProUGUI _createAccountMenu;

    private RectTransform _rectLogin;
    private RectTransform _rectCreateAccount;
    private UI_EventHandler _loginEventHandler;
    private UI_EventHandler _createAccountEventHandler;

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
        GameObject go = GameObject.Find("LoginScene");
        _loginScene = go.GetComponent<LoginScene>();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _loginMenu = Get<TextMeshProUGUI>((int)Texts.LoginMenu);
        _createAccountMenu = Get<TextMeshProUGUI>((int)Texts.CreateAccountMenu);
        if (_loginMenu != null) {
            _rectLogin = _loginMenu.GetComponent<RectTransform>();
            _loginEventHandler = _loginMenu.GetComponent<UI_EventHandler>();
        }
        if (_createAccountMenu != null) {
            _rectCreateAccount = _createAccountMenu.GetComponent<RectTransform>();
            _createAccountEventHandler = _createAccountMenu.GetComponent<UI_EventHandler>();
        }
        if (_loginEventHandler != null || _createAccountEventHandler != null) {
            _loginEventHandler.Clear();
            _createAccountEventHandler.Clear();
            _loginEventHandler.OnClickHandler += (PointerEventData data) => { _loginScene.SetLoginOpt(0); };
            _loginEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLoginOpt(0); };
            _createAccountEventHandler.OnClickHandler += (PointerEventData data) => { _loginScene.SetLoginOpt(1); };
            _createAccountEventHandler.OnPointerEnterHandler += (PointerEventData data) => { _loginScene.SetLoginOpt(1); };
        }
        SetSelectedOpt(0);
    }

    private void Clear() {

    }
}
