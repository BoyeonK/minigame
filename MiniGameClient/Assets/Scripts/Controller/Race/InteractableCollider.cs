using UnityEngine;

public class InteractableCollider : MonoBehaviour {
    private int _objectIdx = -1;

    public void Init(int objectIdx) {
        _objectIdx = objectIdx;
    }

    public int GetObjectIdx() { 
        return _objectIdx;
    }
}
