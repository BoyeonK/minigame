using UnityEngine;

public class MyPlayerBarController : GameObjectController {
    private bool _isMoveDirX = true;
    private int _playerIdx = -1;

    protected override void Init() {
        _objectType = Define.ObjectType.MyPlayerBar;
    }

    public void SetMoveDir(bool isX) { _isMoveDirX = isX; }
    public void SetPlayerIdx(int playerIdx) { _playerIdx = playerIdx; }

    public void MoveToPoint(Vector3 dir) {
        if (_isMoveDirX) {
            Vector3 now = transform.position;
            now.x = dir.x;
            transform.position = now;
        } 
        else {
            Vector3 now = transform.position;
            now.z = dir.z;
            transform.position = now;
        }
    }

    public Vector3 GetPosition() {
        return transform.position;
    }

    void OnTriggerEnter(Collider other) {
        PingPongBulletController specificComp = other.gameObject.GetComponent<PingPongBulletController>();
        if (specificComp != null) {
            Debug.Log("특정 컴포넌트를 가진 오브젝트와 충돌!");
            specificComp.OnCollision();                   
        }
    }
}
