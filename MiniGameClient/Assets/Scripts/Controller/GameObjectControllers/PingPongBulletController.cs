using UnityEngine;

public class PingPongBulletController : GameObjectController {
    private int _lastColider = -1;
    private float _speed = 0;
    private Vector3 _moveDir = Vector3.zero;


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

    public void OnCollision() {
        Debug.Log("���ڿ����׷��̽������ġ��");
    }
}

public class PingPongBulletRedController : PingPongBulletController {
    protected override void Init()  {
        _objectType = Define.ObjectType.PingPongBulletRed;
    }
}

public class PingPongBulletBlueController : PingPongBulletController {
    protected override void Init() {
        _objectType = Define.ObjectType.PingPongBulletBlue;
    }
}

public class PingPongBulletPuppleController : PingPongBulletController {
    protected override void Init() {
        _objectType = Define.ObjectType.PingPongBulletPupple;
    }
}