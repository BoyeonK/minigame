using UnityEditor;
using UnityEngine;
using System.IO;

public class CustomBuildScript
{
    // 1. MenuItem: Tools/Run Multiplayer/4 Players
    [MenuItem("Tools/Run Muitiplayer/4 Players")] // ��Ÿ: "Muitiplayer" -> "Multiplayer"
    static void PerformWin64Build4()
    {
        PerformWin64Build(4);
    }

    // 2. PerformWin64Build �Լ�
    static void PerformWin64Build(int playerCount)
    {
        // ���� Ÿ�� �׷� ��ȯ
        // BuildTarget.StandaloneWindows�� 32��Ʈ/64��Ʈ ��� ����������, BuildTarget.StandaloneWindows64�� ����ϴ� ���� �����ϴ�.
        // �̹� �Ʒ� BuildPlayer���� StandaloneWindows64�� �����ϰ� �����Ƿ�, �� ���� ��ü�� ��� �۵��� �� �ֽ��ϴ�.
        // ������ Ư�� �÷������� ��ȯ�ϴ� ������ �����ϴ�.
        EditorUserBuildSettings.SwitchActiveBuildTarget(
            BuildTargetGroup.Standalone, BuildTarget.StandaloneWindows64); // BuildTarget.StandaloneWindows -> BuildTarget.StandaloneWindows64

        string projectName = GetProjectName();
        string baseBuildPath = "Builds/Win64/"; // ��� Ŭ���̾�Ʈ ������ ���� ����

        for (int i = 0; i < playerCount; i++)
        {
            // �� Ŭ���̾�Ʈ�� ���� ��� ����
            // Path.Combine�� ����Ͽ� OS�� ��� �����ڸ� ó���ϴ� ���� �����ϰ� �����ϴ�.
            string clientFolderName = $"{projectName}{i.ToString()}"; // ��: MyGame0, MyGame1
            string specificBuildPath = Path.Combine(baseBuildPath, clientFolderName);
            string executablePath = Path.Combine(specificBuildPath, $"{projectName}{i.ToString()}.exe");

            // ���� ������ ������ ���� (�ʼ��������� ������, Ȯ�Ǽ��� ���� ����)
            if (!Directory.Exists(specificBuildPath))
            {
                Directory.CreateDirectory(specificBuildPath);
            }

            Debug.Log($"���� ����: {executablePath}");

            BuildPipeline.BuildPlayer(GetScenePaths(),
                                     executablePath,
                                     BuildTarget.StandaloneWindows64, // BuildTargetGroup�� ��ġ
                                     BuildOptions.AutoRunPlayer // BuildOptions.None���� ���� ���
                                    );
            Debug.Log($"���� �Ϸ�: {executablePath}");
        }
        Debug.Log("��� Ŭ���̾�Ʈ ���尡 �Ϸ�Ǿ����ϴ�!");
        EditorUtility.RevealInFinder(baseBuildPath); // ���� �Ϸ� �� ���� ���� ����
    }

    // 3. GetProjectName �Լ�
    static string GetProjectName()
    {
        // Application.dataPath�� Assets ������ ����Դϴ�.
        // ���� ��� "C:/UnityProjects/MyGame/Assets"
        // Split('/') �ϸ� {"C:", "UnityProjects", "MyGame", "Assets"}
        // s[s.Length - 2]�� "MyGame"�� ��ȯ�մϴ�. ��Ȯ�� ������Ʈ �̸� ���� ����Դϴ�.
        string[] s = Application.dataPath.Split('/');
        return s[s.Length - 2];
    }

    // 4. GetScenePaths �Լ�
    static string[] GetScenePaths()
    {
        // Build Settings�� �ִ� Ȱ��ȭ�� ���� �������� ���� �� �����մϴ�.
        var scenePaths = new System.Collections.Generic.List<string>();
        foreach (EditorBuildSettingsScene scene in EditorBuildSettings.scenes)
        {
            if (scene.enabled)
            { // Ȱ��ȭ�� ���� ����
                scenePaths.Add(scene.path);
            }
        }
        return scenePaths.ToArray();
    }
}