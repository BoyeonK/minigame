using UnityEngine;

public class TestPlayerBody : MonoBehaviour {
    private const float _rotationSpeed = 4f;

    public void RotateToAccelerationDirection(Vector3 direction) {
        if (direction.sqrMagnitude > 0.001f) {
            Quaternion targetRotation = Quaternion.LookRotation(direction);
            transform.rotation = Quaternion.Lerp(transform.rotation, targetRotation, Time.deltaTime * _rotationSpeed);
        }
    }

    void OnTriggerEnter(Collider other) {
        Debug.Log($"面倒");
        if (other.gameObject.TryGetComponent<InteractableCollider>(out var Icol)) {
            int opponentsIdx = Icol.GetObjectIdx();
            /*
            if (opponentsIdx == -1)
                return;
            */
            Managers.Network.Marathon.OnCollisionEnter(opponentsIdx);
            Debug.Log($"{opponentsIdx} 面倒");
        }
    }

    void OnTriggerExit(Collider other) {
        Debug.Log($"面cnehf");
        if (other.gameObject.TryGetComponent<InteractableCollider>(out var Icol)) {
            int opponentsIdx = Icol.GetObjectIdx();
            /*
            if (opponentsIdx == -1)
                return;
            */
            Managers.Network.Marathon.OnCollisionExit(opponentsIdx);
            Debug.Log($"{opponentsIdx} 面倒秒家");
        }
    }
}
