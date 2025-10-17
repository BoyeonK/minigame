using UnityEngine;

public class PingPongBulletPuppleController : PingPongBulletController {
    protected override void Init() {
        _objectType = Define.ObjectType.PingPongBulletPupple;
    }
}