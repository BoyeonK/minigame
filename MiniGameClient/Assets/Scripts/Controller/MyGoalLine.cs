using UnityEngine;

public class MyGoalLine : MonoBehaviour {
    void OnTriggerEnter(Collider other) {
        PingPongBulletController specificComp = other.gameObject.GetComponent<PingPongBulletController>();
        if (specificComp != null) {
            Debug.Log("1점 따였다데스");
            //TODO : 실점을 구현
        }
    }
}
