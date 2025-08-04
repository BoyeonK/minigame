using Google.Protobuf.Protocol;
using UnityEngine;
using static Define;

public class CreatureController : MonoBehaviour {
	public int Id { get; set; }

	[SerializeField]
	public float _speed = 5.0f;

	protected bool _updated = false;

	protected Animator _animator;
	protected SpriteRenderer _sprite;

	protected virtual void UpdateAnimation() { }

	void Start() {
		Init();
	}

	void Update() {
		UpdateController();
	}

	protected virtual void Init() {
		_animator = GetComponent<Animator>();
		_sprite = GetComponent<SpriteRenderer>();
		UpdateAnimation();
	}

	protected virtual void UpdateController() {	}

	protected virtual void UpdateIdle() { }

	// ������ �̵��ϴ� ���� ó��
	protected virtual void UpdateMoving() {	}
}
