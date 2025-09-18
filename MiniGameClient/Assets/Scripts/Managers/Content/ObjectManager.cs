using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;
using UnityEngine;

public class ObjectManager {
    Dictionary<int, GameObject> _objects = new Dictionary<int, GameObject>();

	public void Remove(int id) {
		GameObject go = FindByObjectId(id);
		if (go == null)
			return;

		_objects.Remove(id);
		Managers.Resource.Destroy(go);
	}

	public GameObject FindByObjectId(int id) {
		GameObject go = null;
		_objects.TryGetValue(id, out go);
		return go;
	}

	public GameObject CreateObject(UnityGameObject objMessage) {
		int objectId = objMessage.ObjectId;
		string objectType = Enum.GetName(typeof(Define.ObjectType), objMessage.ObjectType);
		GameObject go = Managers.Resource.Instantiate($"GameObjects/{objectType}");
		if (go != null)
			_objects[objectId] = go;

		return go;
	}

	/*
	public GameObject Find(Func<GameObject, bool> condition) {
		foreach (GameObject obj in _objects.Values) {
			if (condition.Invoke(obj))
				return obj;
		}

		return null;
	}
	*/

	public void Clear()	{
		foreach (GameObject obj in _objects.Values)
			Managers.Resource.Destroy(obj);
		_objects.Clear();
	}
}
