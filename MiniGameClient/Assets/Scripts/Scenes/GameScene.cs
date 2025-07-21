using UnityEngine;

public class GameScene : BaseScene
{
    protected override void Init()
    {
        base.Init();
        /*
        SceneType = Define.Scene.Game;
        Managers.Map.LoadMap(1);
        Screen.SetResolution(640, 480, false);
        Application.runInBackground = true;
        */
    }

    public override void Clear()
    {
        Debug.Log("GameScene Cleared!");
    }
}
