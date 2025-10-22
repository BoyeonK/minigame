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

        //TODO : �ظ��ϸ� �� �κ��� Network Manager������ �̰��ϴ°� �������� �������� ����.
        //�ϴ� �׽�Ʈ�ڵ��̹Ƿ� ���� ȣ��.
        Managers.Network.Send(pkt);
    }

    public override int OnGoalLineCollision() {
        return 10;
    }
}