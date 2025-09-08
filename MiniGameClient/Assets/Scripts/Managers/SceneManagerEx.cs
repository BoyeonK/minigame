using System;
using UnityEngine;
using UnityEngine.SceneManagement;

//�� ��ü�� ����ϴ� ��� �Լ���, ����Ƽ�� �޼��带 ����ؾ� �ϹǷ� ���ν����忡�� ���ุ�� ������ ���������.
//����ȭ ����� ������� ���� ����.
public class SceneManagerEx {
    private enum LoadingState {
        None,
        Loading,
        Ready
    }
    
    private LoadingState _loadingState = LoadingState.None;
    private Define.Scene _nextScene = Define.Scene.Undefined;
    //private bool _sceneActiveImmidiately = false;
    private AsyncOperation _asyncLoadSceneOp;

    public BaseScene CurrentScene { get { return GameObject.FindFirstObjectByType<BaseScene>(); } }

    public void OnUpdate() {
        if (_loadingState == LoadingState.None)
            return;
    }
    
    public void LoadScene(Define.Scene type) {
        if (_loadingState != LoadingState.None)
            return;

        Managers.Clear();
        SceneManager.LoadScene(GetSceneName(type));
    }

    string GetSceneName(Define.Scene type) {
        string name = System.Enum.GetName(typeof(Define.Scene), type);
        return name;
    }

    public void LoadSceneWithLoadingScene(Define.Scene targetScene, Define.Scene loadingScene) {
        if (_loadingState != LoadingState.None)
            return;

        _loadingState = LoadingState.Loading;
        _nextScene = targetScene;
        Managers.Clear();
        SceneManager.LoadScene(GetSceneName(loadingScene));
    }

    public void LoadSceneAsync() {
        if (_loadingState != LoadingState.Loading || _nextScene == Define.Scene.Undefined)
            return;

        _asyncLoadSceneOp = SceneManager.LoadSceneAsync(GetSceneName(_nextScene));
        _asyncLoadSceneOp.allowSceneActivation = false;
    }

    public float GetLoadingProgressRate() {
        if (_loadingState == LoadingState.None)
            return 0f;
        else
            return _asyncLoadSceneOp.progress;
    }

    public void Clear() {
        CurrentScene.Clear();
    }
}
