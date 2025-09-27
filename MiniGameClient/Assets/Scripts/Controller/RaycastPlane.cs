using UnityEngine;

public class RaycastPlane : MonoBehaviour {
    private LayerMask raycastPlaneLayer; // "RaycastPlane" 레이어만 선택하도록 Inspector에서 설정
    private const string LAYER_NAME = "RaycastPlanesLayer";
    private Vector3 _hp;

    PingPongScene _scene;

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start() {
        int layerIdx = LayerMask.NameToLayer(LAYER_NAME);
        if (layerIdx == -1) {
            Debug.Log("Raycast에 사용할 Layer를 찾지 못하였습니다.");
            return;
        }
        raycastPlaneLayer = 1 << layerIdx;

        GameObject go = GameObject.Find("GameScene");
        if (go != null) {
            _scene = go.GetComponent<PingPongScene>();
        }
        else {
            Debug.LogError("RaycastPlane이 Scene 컴포넌트를 로드하는데 실패");
        }
    }

    // Update is called once per frame
    void Update() {
        Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        RaycastHit hit;

        // raycastPlaneLayer (RaycastPlane 레이어)만 감지하도록 LayerMask 적용
        if (Physics.Raycast(ray, out hit, Mathf.Infinity, raycastPlaneLayer)) {
            Vector3 hitPoint = hit.point;
            _scene.OnMouseMove(hitPoint);
        }
    }
}
