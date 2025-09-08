using UnityEngine;

public class TestLoadingScene : BaseScene {
    private float _progressRate = 0f;
    private float _mEpsilon = 0.001f;
    private bool _isReady = false;

    protected override void Init() {
        base.Init();
        _progressRate = 0f;
        _isReady = false;
        SceneType = Define.Scene.TestLoadingScene;
        Managers.Scene.LoadSceneAsync();
    }

    private void UpdateProgressRate() {
        float progress = Managers.Scene.GetLoadingProgressRate();
        while (progress >= _progressRate + 0.1) {
            _progressRate += 0.1f;
        }
    }

    private void Update() {
        
    }
}
