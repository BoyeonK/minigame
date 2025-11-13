using UnityEngine;

public class LoadingScene1 : BaseScene {
    /*
    private float _progressRate = 0f;
    private bool _isReady = false;
    private bool _hadProgress = false;
    */

    protected override void Init() {
        base.Init();
        /*
        _progressRate = 0f;
        _isReady = false;
        */
        SceneType = Define.Scene.LoadingScene1;
    }

    private void Start() {
        Debug.Log("로딩씬 스타트");
        //Managers.Scene.LoadSceneAsync();
    }

    private void UpdateProgressRate() {
        /*
        float progress = Managers.Scene.GetLoadingProgressRate();
        while (progress >= _progressRate + 0.099f) {
            _hadProgress = true;
            _progressRate += 0.1f;
            Debug.Log("+0.1");
        }
        */
    }

    private void Update() {
        /*
        if (!_isReady) {
            UpdateProgressRate();
            if (_hadProgress) {
                Debug.Log($"진전이 있었다. 진행률 : {_progressRate}");
                Managers.Network.TrySendLoadingProgressRate(_progressRate);
                _hadProgress = false;
            }
        }
        */
    }

    private void SceneChange() {

    }

    public override void Clear() {

    }
}
