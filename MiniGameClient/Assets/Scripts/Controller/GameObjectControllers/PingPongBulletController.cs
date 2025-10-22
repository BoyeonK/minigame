using Google.Protobuf.Protocol;
using UnityEngine;

public class PingPongBulletController : GameObjectController {
    protected int _lastColider = -1;
    protected float _speed = 0;
    protected Vector3 _moveDir = Vector3.zero;

    private void Update() {
        if (_speed > 0)
            transform.Translate(_speed * Time.deltaTime * _moveDir);
    }

    public void SetMoveDir(Vector3 moveDir) {
        _moveDir = moveDir.normalized;
    }

    public void SetSpeed(float speed) {
        _speed = speed;
    }

    public void SetLastColider(int lastColider) {
        _lastColider = lastColider;
    }

    virtual public void OnBarCollision(int playerIdx) { }
    virtual public int OnGoalLineCollision() { return 0; }
}

