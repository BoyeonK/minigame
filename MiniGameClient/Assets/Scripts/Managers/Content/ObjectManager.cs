using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;
using UnityEngine;

public class ObjectManager {
    Dictionary<int, GameObject> _objects = new Dictionary<int, GameObject>();

	//Object 분류 별로 따로 dictionary를 파도 된다.
	//Dictionary<int, GameObject> _players = new Dictionary<int, GameObject>();
	//Dictionary<int, GameObject> _monsters = new Dictionary<int, GameObject>();
	//Dictionary<int, GameObject> _envs = new Dictionary<int, GameObject>();

	public void Remove(int id) {
		GameObject go = FindById(id);
		if (go == null)
			return;

		_objects.Remove(id);
		Managers.Resource.Destroy(go);
	}

	public GameObject FindById(int id) {
		GameObject go = null;
		_objects.TryGetValue(id, out go);
		return go;
	}

	public GameObject Find(Func<GameObject, bool> condition) {
		foreach (GameObject obj in _objects.Values) {
			if (condition.Invoke(obj))
				return obj;
		}

		return null;
	}

	public void Clear()	{
		foreach (GameObject obj in _objects.Values)
			Managers.Resource.Destroy(obj);
		_objects.Clear();
	}
}
