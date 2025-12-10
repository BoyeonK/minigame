using UnityEngine;

public class SolidFloor : MonoBehaviour {
    [SerializeField]
    private int yBoundary;
    [SerializeField]
    private float _floorFriction = 0.06f;


    public float GetFloorFriction() {
        return _floorFriction;
    }
}
