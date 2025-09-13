using Google.Protobuf.Protocol;
using UnityEngine;
using static Define;

public class GameObjectController : MonoBehaviour {
	protected int _objectId { get; set; }
	protected ObjectType _objectType { get; set; }

	void Start() {
		Init();
	}

	protected virtual void Init() {

	}
}
