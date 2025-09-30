using UnityEngine;

public class RaycastPlane : MonoBehaviour {
    private LayerMask raycastPlaneLayer; // "RaycastPlane" 레이어만 선택하도록 Inspector에서 설정
    private const string LAYER_NAME = "RaycastPlanesLayer";

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start() {
        int layerIdx = LayerMask.NameToLayer(LAYER_NAME);
        if (layerIdx == -1) {
            Debug.Log("Raycast에 사용할 Layer를 찾지 못하였습니다.");
            return;
        }
        raycastPlaneLayer = 1 << layerIdx;
    }

    public Vector3 GetRaycastPoint() {
        Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, Mathf.Infinity, raycastPlaneLayer))
            return hit.point;
        return Vector3.zero;
    }
}
