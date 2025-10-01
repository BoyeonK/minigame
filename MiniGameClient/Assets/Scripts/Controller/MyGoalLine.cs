using UnityEngine;

public class MyGoalLine : MonoBehaviour {
    void OnTriggerEnter(Collider other) {
        PingPongBullet1Controller specificComp = other.gameObject.GetComponent<PingPongBullet1Controller>();
        if (specificComp != null) {
            Debug.Log("1점 따였다데스");
            //TODO : 실점을 구현
        }
    }
}
