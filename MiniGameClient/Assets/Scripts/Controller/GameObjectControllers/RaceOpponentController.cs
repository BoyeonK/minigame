using UnityEngine;

public class RaceOpponentController : GameObjectController {
    enum State {
        Standing,
        Moving,
        Jumping,
    }

    State _prestate = State.Standing;
    State _state = State.Standing;
    
    private RaceOpponentSuperficialBody _body;
    private Vector3 _velocity = Vector3.zero;
    private Vector3 _realPosition = new();
    
    public override void Init() {
        SetObjectId((int)Define.ObjectType.RaceOpponent);
        _realPosition = transform.position;
        
        Transform go = transform.Find("SDUnityChan");
        if (go != null) {
            _body = go.GetComponent<RaceOpponentSuperficialBody>();
            if (_body != null) {
                _body.Init(_objectId, _realPosition);
            }
        }  
    }

    public void SetMovementInfo(Vector3 position, Vector3 front, Vector3 velocity, int state) {
        SetPositionVector(position, velocity);
        _body.SetFrontVector(front);
        _body.SetState(state);
    }

    private void SetPositionVector(Vector3 position, Vector3 velocity) {
        _body.StoreMyPos();
        _realPosition = position;
        transform.position = position;
        _velocity = velocity;
        _body.ApplyMyPos();
    }

    private void Update() {
        MoveRealPositionOnUpdate();
        _body.MoveSuperficialPositionOnUpdate(_realPosition);
        _body.RotateOnUpdate();
    }

    private void MoveRealPositionOnUpdate() {
        _realPosition = _realPosition + _velocity * Time.deltaTime;
    }
}
