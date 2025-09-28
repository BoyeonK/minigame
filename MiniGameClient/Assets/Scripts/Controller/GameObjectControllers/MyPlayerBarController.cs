using UnityEngine;

public class MyPlayerBarController : GameObjectController {
    private bool _isMoveDirX = true;

    protected override void Init() {
        _objectType = Define.ObjectType.MyPlayerBar;
    }

    public void SetMoveDir(bool isX) { _isMoveDirX = isX; }

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
}
