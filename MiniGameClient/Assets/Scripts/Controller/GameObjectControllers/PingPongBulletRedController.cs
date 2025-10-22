using Google.Protobuf.Protocol;
using UnityEngine;


public class PingPongBulletRedController : PingPongBulletController {
    public override void Init() {
        _objectType = Define.ObjectType.PingPongBulletRed;
    }

    public override void OnBarCollision(int playerIdx) {
        if (_lastColider == playerIdx)
            return;

        UnityGameObject serializedObj = new() {
            ObjectId = _objectId,
            ObjectType = (int)_objectType,
        };

        Vector3 pos = transform.position;
        serializedObj.Position = new XYZ { X = pos.x, Y = pos.y, Z = pos.z };

        XYZ moveDir = new() {
            X = _moveDir.x,
            Y = _moveDir.y,
            Z = _moveDir.z
        };

        C_P_CollisionBar pkt = new() {
            Bullet = serializedObj,
            MoveDir = moveDir,
            Speed = _speed
        };

        //TODO : 왠만하면 이 부분은 Network Manager쪽으로 이관하는게 유지보수 관점에서 좋음.
        //일단 테스트코드이므로 직접 호출.
        Managers.Network.Send(pkt);
    }

    public override int OnGoalLineCollision() {
        return 10;
    }
}