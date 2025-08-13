using System;
using System.Collections.Generic;
using UnityEngine;

public class Managers : MonoBehaviour {
    public static Managers Instance { get; private set; }

    public static int GameVersion { get { return 0; } }

    public static ObjectManager Object { get { return Instance._obj; } }
    ObjectManager _obj = new ObjectManager();

    public static NetworkManager Network { get { return Instance._network; } }
    NetworkManager _network = new NetworkManager();
    
    InputManager _input = new InputManager();
    public static InputManager Input { get { return Instance._input; } }

    PoolManager _pool = new PoolManager();
    public static PoolManager Pool { get { return Instance._pool; } }

    ResourceManager _resource = new ResourceManager();
    public static ResourceManager Resource { get { return Instance._resource; } }

    UIManager _ui = new UIManager();
    public static UIManager UI { get { return Instance._ui; } }

    SceneManagerEx _scene = new SceneManagerEx();
    public static SceneManagerEx Scene { get { return Instance._scene; } }

    SoundManager _sound = new SoundManager();
    public static SoundManager Sound { get { return Instance._sound; } }

    private static Queue<Action> _jobQueue = new Queue<Action>();
    private static readonly object _lock = new object();
    public static void ExecuteAtMainThread(Action job) {
        lock (_lock) {
            _jobQueue.Enqueue(job);
        }
    }

    private void Awake() {
        if (Instance != null) {
            Destroy(gameObject);
            return;
        }
        Instance = this;
        DontDestroyOnLoad(gameObject);
        Instance._network.Init();
        Instance._pool.Init();
        Instance._sound.Init();
        Instance._ui.Init();
        Debug.Log("Manager Initiate");
    }

    // Update is called once per frame
    void Update() {
        _input.OnUpdate();
        _network.Update();
        lock (_lock) {
            while (_jobQueue.Count > 0) {
                Action job = _jobQueue.Dequeue();
                job?.Invoke();
            }
        }
    }

    static void Init() {
        if (Instance == null) {
            GameObject go = GameObject.Find("@Managers");
            if (go == null) {
                go = new GameObject { name = "@Managers" };
                go.AddComponent<Managers>();
            }
            DontDestroyOnLoad(go);
            Instance = go.GetComponent<Managers>();
            Debug.Log("Manager Initiated by Init");
        }  
    }

    public static void Clear() {
        Sound.Clear();
        Input.Clear();
        Scene.Clear();
        UI.Clear();

        Pool.Clear();
    }
}
