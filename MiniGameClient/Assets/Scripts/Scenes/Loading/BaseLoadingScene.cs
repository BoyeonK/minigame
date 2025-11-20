using UnityEngine;

public class BaseLoadingScene : BaseScene {
    protected float _progressRate = 0f;
    protected bool _isReady = false;
    protected bool _hadProgress = false;

    protected override void Init() {
        base.Init();
        _progressRate = 0f;
        _isReady = false;
        Managers.Network.Match.OnResponseGameStartedAct += SceneChange;
    }

    private void UpdateProgressRate() {
        float progress = Managers.Scene.GetLoadingProgressRate();
        while (progress >= _progressRate + 0.099f) {
            _hadProgress = true;
            _progressRate += 0.1f;
            Debug.Log("+0.1");
        }
    }

    protected void OnStart() {
        Debug.Log("로딩씬 스타트");
        Managers.Scene.LoadSceneAsync();
    }

    protected void OnUpdate() {
        if (!_isReady) {
            UpdateProgressRate();
            if (_hadProgress) {
                Debug.Log($"진전이 있었다. 진행률 : {_progressRate}");
                Managers.Network.TrySendLoadingProgressRate(_progressRate);
                _hadProgress = false;
            }
        }
    }

    protected void SceneChange() {
        Managers.Scene.CompleteLoadSceneAsync();
    }

    public override void Clear() {
        Managers.Network.Match.OnResponseGameStartedAct -= SceneChange;
    }
}