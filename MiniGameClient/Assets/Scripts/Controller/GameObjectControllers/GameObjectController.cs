using Google.Protobuf.Protocol;
using UnityEngine;
using static Define;

public class GameObjectController : MonoBehaviour {
	protected int _objectId;
	protected ObjectType _objectType { get; set; }

	public void SetObjectId(int objectId) {
		_objectId = objectId;
	}

	void Start() {
		Init();
	}

	public virtual void Init() {

	}
}
