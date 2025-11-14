using UnityEngine;

public class LoadingScene1 : BaseLoadingScene {
    protected override void Init() {
        base.Init();
    }

    private void Start() {
        OnStart();
    }

    private void Update() {
        OnUpdate();
    }

    public override void Clear() {
        base.Clear();
    }
}
