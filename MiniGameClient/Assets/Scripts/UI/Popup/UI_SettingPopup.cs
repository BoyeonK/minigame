using Google.Protobuf.Protocol;
using System;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_SettingPopup : UI_Popup {
    TextMeshProUGUI _bgmValueText;
    TextMeshProUGUI _effectValueText;
    int _bgmValue = 0;
    int _effectValue = 0;
    int _newBgmValue = 0;
    int _newEffectValue = 0;
    Button _confirmButton;
    Button _cancelButton;
    Slider _bgmSlider;
    Slider _effectSlider;
    bool _isInitialized = false;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    private void Clear() {

    }

    public override void Init() {
        base.Init();
        if (!_isInitialized) BindAllComponents();
        SyncOptionValue();
    }

    private void BindAllComponents() {
        Transform settingPanel = transform.Find("SettingPanel");
        if (settingPanel != null) {
            Transform confirmBtnTrans = settingPanel.Find("ConfirmButton");
            if (confirmBtnTrans != null)
                _confirmButton = confirmBtnTrans.GetComponent<Button>();

            Transform cancelBtnTrans = settingPanel.Find("CancelButton");
            if (cancelBtnTrans != null)
                _cancelButton = cancelBtnTrans.GetComponent<Button>();

            Transform bgmSliderTrans = settingPanel.Find("BgmSoundSetting");
            if (bgmSliderTrans != null) {
                _bgmSlider = bgmSliderTrans.GetComponentInChildren<Slider>();
                if (_bgmSlider != null)
                    _bgmSlider.onValueChanged.AddListener(OnBgmSliderChanged);

                Transform bgmValueTextTrans = bgmSliderTrans.Find("Value");
                if (bgmValueTextTrans != null)
                    _bgmValueText = bgmValueTextTrans.GetComponent<TextMeshProUGUI>();
            }

            Transform effectSliderTrans = settingPanel.Find("EffectSoundSetting");
            if (effectSliderTrans != null) {
                _effectSlider = effectSliderTrans.GetComponentInChildren<Slider>();
                if (_effectSlider != null)
                    _effectSlider.onValueChanged.AddListener(OnEffectSliderChanged);

                Transform effectValueTextTrans = effectSliderTrans.Find("Value");
                if (effectValueTextTrans != null)
                    _effectValueText = effectValueTextTrans.GetComponent<TextMeshProUGUI>();
            }
        }

        if (_confirmButton != null) {
            _confirmButton.onClick.AddListener(() => { Managers.Sound.Play("button"); });
            _confirmButton.onClick.AddListener(ConfirmSettingBtn);
            _confirmButton.onClick.AddListener(() => { Managers.UI.DisableUI("UI_SettingPopup"); });
        }
        if (_cancelButton != null) {
            _cancelButton.onClick.AddListener(() => { Managers.Sound.Play("button"); });
            _cancelButton.onClick.AddListener(CancelSettingBtn);
            _cancelButton.onClick.AddListener(() => { Managers.UI.DisableUI("UI_SettingPopup"); });
        }
        _isInitialized = true;
    }

    public void AddListenerToConfirmBtn(Action action) {
        if (_confirmButton != null)
            _confirmButton.onClick.AddListener(() => { action?.Invoke(); });
    }

    public void AddListenerToCancelBtn(Action action) {
        if (_cancelButton != null)
            _cancelButton.onClick.AddListener(() => { action?.Invoke(); });
    }

    private void SyncOptionValue() {
        _bgmValue = Managers.Setting.GetBgmVolume();
        _effectValue = Managers.Setting.GetEffectVolume();
        _newBgmValue = _bgmValue;
        _newEffectValue = _effectValue;

        if (_bgmSlider != null) _bgmSlider.value = _bgmValue / 100f;
        if (_effectSlider != null) _effectSlider.value = _effectValue / 100f;
    }

    private void OnBgmSliderChanged(float value) {
        _newBgmValue = Mathf.RoundToInt(value * 100);

        if (_bgmValueText != null) 
            _bgmValueText.text = _newBgmValue.ToString();

        Managers.Setting.SetVolume(_newBgmValue, Define.Sound.Bgm);
    }

    private void OnEffectSliderChanged(float value) {
        _newEffectValue = Mathf.RoundToInt(value * 100);

        if (_effectValueText != null) 
            _effectValueText.text = _newEffectValue.ToString();

        Managers.Sound.SetVolume(_newBgmValue, Define.Sound.Effect);
    }

    private void ConfirmSettingBtn() {
        _bgmValue = _newBgmValue;
        _effectValue = _newEffectValue;
        Managers.Setting.SetVolume(_bgmValue, Define.Sound.Bgm);
        Managers.Setting.SetVolume(_effectValue, Define.Sound.Effect);
    }

    private void CancelSettingBtn() {
        Managers.Setting.SetVolume(_bgmValue, Define.Sound.Bgm);
        Managers.Setting.SetVolume(_effectValue, Define.Sound.Effect);
        _newBgmValue = _bgmValue;
        _newEffectValue = _effectValue;
        if (_bgmSlider != null)
            _bgmSlider.value = _bgmValue / 100f;
        if (_effectSlider != null)
            _effectSlider.value = _effectValue / 100f;
    }
}
