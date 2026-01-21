using System.Collections.Generic;
using UnityEngine;

public class SettingManager {
    float _effectVolume = 1f;
    float _bgmVolume = 0.1f;

    public void Init() {
        GameObject root = GameObject.Find("@Setting");
        if (root == null) {
            root = new GameObject { name = "@Setting" };
            Object.DontDestroyOnLoad(root);
        }
    }

    public void ApplyPreviousSceneSetting() {
        Managers.Sound.SetVolume(_bgmVolume, Define.Sound.Bgm);
        Managers.Sound.SetVolume(_effectVolume, Define.Sound.Effect);
    }

    public void SetVolume(float volume, Define.Sound type) {
        float clamped = Mathf.Clamp(volume, 0f, 1f);

        if (type == Define.Sound.Effect) _effectVolume = clamped;
        else if (type == Define.Sound.Bgm) _bgmVolume = clamped;

        Managers.Sound.SetVolume(clamped, type);
    }

    public void Clear() {

    }
}
