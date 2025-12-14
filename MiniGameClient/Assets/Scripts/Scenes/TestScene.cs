using Google.Protobuf.Protocol;
using System;
using Unity.VisualScripting;
using UnityEngine;

public class TestScene : BaseScene {
    RaceOpponentController _controller;
    float lasttick = 0f;
    int asdf = 0;

    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.Login;

        UnityGameObject obj = new UnityGameObject();
        XYZ position = new XYZ();
        obj.ObjectId = 0;
        obj.ObjectType = (int)(Define.ObjectType.RaceOpponent);
        position.X = 32f;
        position.Y = 0.2f;
        position.Z = 14f;
        obj.Position = position;
        GameObject oppo = Managers.Object.CreateObject(obj);
        _controller = oppo.GetComponent<RaceOpponentController>();
    }

    private void Update() {
        if (Time.time - lasttick > 0.1f) {
            lasttick = Time.time;
            NextTick();
        }
    }

    private void NextTick() {
        Debug.Log("sex");
        asdf += 1;
        if (asdf == 20)
            asdf = 0;

        float deltax = Mathf.Abs(10 - asdf) / 10f;
        Vector3 position = new Vector3(32f+deltax, 0.2f, 14f);
        Vector3 velocity;
        if (asdf < 10)
            velocity = new(1f, 0f, 0f);
        else
            velocity = new(-1f, 0f, 0f);

        float x = Mathf.Cos(asdf / 10f * Mathf.PI);
        float z = Mathf.Sin(asdf / 10f * Mathf.PI);

        Vector3 front = new(x, 0, z);

        _controller.SetMovementInfo(position, front, velocity);
    }
}
