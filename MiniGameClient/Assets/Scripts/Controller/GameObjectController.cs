using Google.Protobuf.Protocol;
using UnityEngine;
using static Define;

public class GameObjectController : MonoBehaviour {
	private int _objectId { get; set; }
	private ObjectType _objectType { get; set; }

	void Start() {
		Init();
	}

	protected virtual void Init() {

	}
}
