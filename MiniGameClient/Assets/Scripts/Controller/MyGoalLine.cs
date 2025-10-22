using Google.Protobuf.Protocol;
using UnityEngine;

public class MyGoalLine : MonoBehaviour {
    void OnTriggerEnter(Collider other) {
        if (other.gameObject.TryGetComponent<PingPongBulletController>(out var bulletController)) {
            int point = bulletController.OnGoalLineCollision();
            C_P_CollisionGoalLine pkt = new() { Point = point };
            Managers.Network.Send(pkt);
        }
    }
}
