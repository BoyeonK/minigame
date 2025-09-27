using UnityEngine;

public class RaycastPlane : MonoBehaviour {
    private LayerMask raycastPlaneLayer; // "RaycastPlane" ���̾ �����ϵ��� Inspector���� ����
    private const string LAYER_NAME = "RaycastPlanesLayer";
    private Vector3 _hp;

    PingPongScene _scene;

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start() {
        int layerIdx = LayerMask.NameToLayer(LAYER_NAME);
        if (layerIdx == -1) {
            Debug.Log("Raycast�� ����� Layer�� ã�� ���Ͽ����ϴ�.");
            return;
        }
        raycastPlaneLayer = 1 << layerIdx;

        GameObject go = GameObject.Find("GameScene");
        if (go != null) {
            _scene = go.GetComponent<PingPongScene>();
        }
        else {
            Debug.LogError("RaycastPlane�� Scene ������Ʈ�� �ε��ϴµ� ����");
        }
    }

    // Update is called once per frame
    void Update() {
        Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        RaycastHit hit;

        // raycastPlaneLayer (RaycastPlane ���̾�)�� �����ϵ��� LayerMask ����
        if (Physics.Raycast(ray, out hit, Mathf.Infinity, raycastPlaneLayer)) {
            Vector3 hitPoint = hit.point;
            _scene.OnMouseMove(hitPoint);
        }
    }
}
