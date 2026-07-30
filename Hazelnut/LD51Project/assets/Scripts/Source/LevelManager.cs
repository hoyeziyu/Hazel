using System;
using System.Collections.Generic;
using System.IO;

using Hazel;

namespace LD51
{
	public class LevelManager : Entity
	{
		public Prefab EmptyTile = null!;
		public Prefab GrassTile = null!;
		public Prefab GoalItem = null!;
		public Prefab SandboxTile = null!;
		public Prefab SectionContainer = null!;
		public Prefab LevelContainer = null!;

		public string LevelFile = "assets/Levels/Starter.png";

		private readonly Dictionary<TileType, Prefab> m_TilePrefabs = new Dictionary<TileType, Prefab>();
		private readonly Dictionary<ItemType, Prefab> m_ItemPrefabs = new Dictionary<ItemType, Prefab>();
		private LevelSection? m_Section;

		protected override void OnCreate()
		{
			m_TilePrefabs[TileType.None] = EmptyTile;
			m_TilePrefabs[TileType.Grass] = GrassTile;

			m_ItemPrefabs[ItemType.Goal] = GoalItem;

			string levelPath = ResolveLevelPath(LevelFile);
			if (!File.Exists(levelPath))
			{
				Log.Error("LevelManager: level file not found: {0}", levelPath);
				return;
			}

			LoadLevel(levelPath);
			Log.Info("LevelManager: loaded level with {0} tiles", m_Section?.Tiles.Count ?? 0);
		}

		private static string ResolveLevelPath(string path)
		{
			if (File.Exists(path))
				return path;

			string[] candidates =
			{
				Path.Combine("LD51Project", path),
				Path.Combine(AppContext.BaseDirectory, path),
				Path.Combine(AppContext.BaseDirectory, "LD51Project", path),
			};

			foreach (string candidate in candidates)
			{
				if (File.Exists(candidate))
					return candidate;
			}

			return path;
		}

		private void LoadLevel(string filepath)
		{
			LevelReader level = new LevelReader(filepath);
			m_Section = new LevelSection();

			const float sandboxSectionZOffset = 0.0f;
			const float levelSectionZOffset = 11.0f;

			Entity? sectionContainer = Instantiate(SectionContainer, new Vector3(0, 0, 0));
			Entity? levelContainer = Instantiate(LevelContainer, new Vector3(0, 0, levelSectionZOffset));
			if (sectionContainer == null || levelContainer == null)
			{
				Log.Error("LevelManager: failed to instantiate section containers");
				return;
			}

			m_Section.SandboxEntity = sectionContainer;
			m_Section.LevelEntity = levelContainer;

			for (int y = 0; y < 11; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					float tx = x - 5;
					float tzSandbox = y - 11 + sandboxSectionZOffset;
					sectionContainer.InstantiateChild(SandboxTile, new Vector3(tx, 0, tzSandbox));

					float tzLevel = y - 11 + levelSectionZOffset;
					TileType tile = level.Tiles[x, y];
					if (!m_TilePrefabs.TryGetValue(tile, out Prefab tilePrefab))
						continue;

					Entity? tileEntity = levelContainer.InstantiateChild(tilePrefab, new Vector3(tx, 0, tzLevel));
					if (tileEntity == null)
						continue;

					ItemType item = level.Items[x, y];
					if (item == ItemType.Goal)
					{
						m_Section.GoalTile = new TileKey { X = (int)tx, Z = (int)tzLevel };
						sectionContainer.InstantiateChild(GoalItem, new Vector3(tx, 0.5f, tzSandbox));
					}

					if (item != ItemType.None && m_ItemPrefabs.TryGetValue(item, out Prefab itemPrefab))
						tileEntity.InstantiateChild(itemPrefab, Vector3.Zero);

					m_Section.AddTile((int)tx, (int)tzLevel, new TileData
					{
						TileEntity = tileEntity,
						Type = tile,
						IsVisible = true,
						ItemType = item,
						TrapType = level.Traps[x, y]
					});
				}
			}
		}
	}
}
