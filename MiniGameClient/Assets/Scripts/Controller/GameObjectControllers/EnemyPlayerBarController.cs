using System;
using UnityEngine;

public class EnemyPlayerBarController : GameObjectController {
    public void SetObjectId(int id) {
        _objectId = id;
    }

    public void SetPosition(float x, float z) {
        Vector3 now = transform.position;
        now.x = x;
        now.z = z;
        transform.position = now;
    }
}
