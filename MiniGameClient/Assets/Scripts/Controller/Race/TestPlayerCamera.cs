using UnityEngine;

public class TestPlayerCamera : MonoBehaviour {
    GameObject _owner;

    public void Init(GameObject owner) {
        _owner = owner;
    }

    public void ChangePositionOnUpdate(Vector3 pos) {
        transform.position = pos;

        if (_owner != null) {
            Vector3 targetLookPosition = _owner.transform.position;
            transform.LookAt(targetLookPosition);
        }
    }
}
