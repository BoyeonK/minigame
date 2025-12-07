using UnityEngine;

public class TestOpponentController : GameObjectController {
    GameObject _body;
    InteractableCollider _bodyCollider;

    public override void Init() {
        SetObjectId((int)Define.ObjectType.TestOpponent);
        SetObjectId(12345);

        Transform bodyTransform = transform.Find("TestOpponentBody");
        if (bodyTransform != null) {
            _body = bodyTransform.gameObject;
            _bodyCollider = bodyTransform.GetComponent<InteractableCollider>();
        }
        if (_bodyCollider != null)
            _bodyCollider.Init(_objectId);
    } 

    void Update() {
        
    }

    private void OnDestroy() {
        
    }
}
