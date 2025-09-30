using UnityEngine;

public class RaycastPlane : MonoBehaviour {
    private LayerMask raycastPlaneLayer; // "RaycastPlane" ���̾ �����ϵ��� Inspector���� ����
    private const string LAYER_NAME = "RaycastPlanesLayer";

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start() {
        int layerIdx = LayerMask.NameToLayer(LAYER_NAME);
        if (layerIdx == -1) {
            Debug.Log("Raycast�� ����� Layer�� ã�� ���Ͽ����ϴ�.");
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
