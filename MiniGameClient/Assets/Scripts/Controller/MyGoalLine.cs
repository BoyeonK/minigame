using UnityEngine;

public class MyGoalLine : MonoBehaviour {
    void OnTriggerEnter(Collider other) {
        PingPongBullet1Controller specificComp = other.gameObject.GetComponent<PingPongBullet1Controller>();
        if (specificComp != null) {
            Debug.Log("1�� �����ٵ���");
        }
    }
}
