using UnityEngine;

public class MyGoalLine : MonoBehaviour {
    void OnTriggerEnter(Collider other) {
        PingPongBulletController specificComp = other.gameObject.GetComponent<PingPongBulletController>();
        if (specificComp != null) {
            Debug.Log("1�� �����ٵ���");
            //TODO : ������ ����
        }
    }
}
