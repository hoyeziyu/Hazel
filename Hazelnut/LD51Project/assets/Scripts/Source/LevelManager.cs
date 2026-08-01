using System;
using System.Collections.Generic;
using System.IO;

using Hazel;

namespace LD51
{
	public class LevelManager : Entity
	{
		private class TileAnimationInfo
		{
			public TileData Data = null!;
			public Vector3 StartLocation;
			public Vector3 TargetLocation;
			public float AnimationSpeed = 1.0f;
			public float AnimationTime;
		}

		public Prefab EmptyTile = null!;
		public Prefab GrassTile = null!;
		public Prefab GoalItem = null!;
		public Prefab SandboxTile = null!;
		public Prefab SectionContainer = null!;
		public Prefab LevelContainer = null!;
		public Prefab SpikesTile = null!;
		public Prefab LavaTile = null!;
		public Prefab PressurePlateItem = null!;
		public Prefab BridgeUpDownItem = null!;
		public Prefab BridgeLeftRightItem = null!;
		public Prefab AxeTrapItem = null!;

		public string LevelFile = "assets/Levels/DualMechanisms.png";

		public Entity m_CameraEntity = null!;
		public AudioComponent AC = null!;

		public bool GameStarted { get; private set; }

		private static LevelManager? s_Instance;
		public static LevelManager? Instance => s_Instance;

		private readonly Dictionary<TileType, Prefab> m_TilePrefabs = new Dictionary<TileType, Prefab>();
		private readonly Dictionary<ItemType, Prefab> m_ItemPrefabs = new Dictionary<ItemType, Prefab>();
		private LevelSection? m_Section;
		private bool m_IsReplicatingMoves;
		private bool m_HasGoal;
		private float m_TrapTimer;
		private readonly List<TileAnimationInfo> m_TilesToAnimate = new List<TileAnimationInfo>();

		protected override void OnCreate()
		{
			s_Instance = this;

			if (HasComponent<AudioComponent>())
				AC = GetComponent<AudioComponent>()!;

			m_CameraEntity = FindEntityByTag("Camera") ?? this;

			m_TilePrefabs[TileType.None] = EmptyTile;
			m_TilePrefabs[TileType.Grass] = GrassTile;
			m_TilePrefabs[TileType.Spikes] = SpikesTile;
			m_TilePrefabs[TileType.Lava] = LavaTile;
			m_TilePrefabs[TileType.LavaDeadly] = LavaTile;
			m_ItemPrefabs[ItemType.Goal] = GoalItem;
			m_ItemPrefabs[ItemType.PressurePlate] = PressurePlateItem;
			m_ItemPrefabs[ItemType.BridgeUpDown] = BridgeUpDownItem;
			m_ItemPrefabs[ItemType.BridgeLeftRight] = BridgeLeftRightItem;

			string levelPath = ResolveLevelPath(LevelFile);
			if (!File.Exists(levelPath))
			{
				Log.Error("LevelManager: level file not found: {0}", levelPath);
				return;
			}

			LoadLevel(levelPath);
			GameStarted = true;
			Log.Info("LevelManager: loaded level with {0} tiles", m_Section?.Tiles.Count ?? 0);

			var replicator = FindEntityByTag("PlayerReplicator");
			if (replicator != null)
				ShowNeighbouringTiles(new Vector3Int(replicator.Translation));
		}

		protected override void OnUpdate(float ts)
		{
			UpdateTraps(ts);

			if (m_TilesToAnimate.Count == 0)
				return;

			int finished = 0;
			foreach (var anim in m_TilesToAnimate)
			{
				if (anim.Data.IsVisible || anim.Data.TileEntity == null)
					continue;

				anim.AnimationTime += ts * anim.AnimationSpeed;
				anim.Data.TileEntity.Translation = Vector3.Lerp(anim.StartLocation, anim.TargetLocation, anim.AnimationTime);

				if (Vector3.Distance(anim.Data.TileEntity.Translation, anim.TargetLocation) < 0.25f)
				{
					anim.Data.TileEntity.Translation = anim.TargetLocation;
					anim.Data.IsVisible = true;
					finished++;
				}
			}

			if (finished > 0)
				m_TilesToAnimate.RemoveAll(a => a.Data.IsVisible);
		}

		public bool IsReplicatingMoves() => m_IsReplicatingMoves;

		public void SwitchRound()
		{
			m_IsReplicatingMoves = !m_IsReplicatingMoves;

			var player = FindEntityByTag("Player")?.As<Player>();
			var replicator = FindEntityByTag("PlayerReplicator")?.As<PlayerReplicator>();

			if (player != null)
				player.IsActivePlayer = !m_IsReplicatingMoves;

			replicator?.SetReplicateEnabled(m_IsReplicatingMoves);
		}

		public bool HasValidSandboxTile(Vector3 position)
		{
			if (m_Section?.SandboxEntity == null)
				return false;

			foreach (Entity sandboxTile in m_Section.SandboxEntity.Children)
			{
				if (position.XZ.Distance(sandboxTile.Translation.XZ) < 0.2f)
					return true;
			}

			return false;
		}

		public void RevealAdjacentTilesInSandbox(Vector3 position)
		{
			int tileX = Mathf.FloorToInt(position.X);
			int tileZ = Mathf.FloorToInt(position.Z + 11.0f);
			Log.Info("LevelManager: reveal sandbox tiles around ({0}, {1})", tileX, tileZ);
		}

		public void ShowNeighbouringTiles(Vector3Int position)
		{
			ScheduleTileAnimation(position.X, position.Z);
			ScheduleTileAnimation(position.X, position.Z - 1);
			ScheduleTileAnimation(position.X + 1, position.Z);
			ScheduleTileAnimation(position.X - 1, position.Z);
			ScheduleTileAnimation(position.X, position.Z + 1);
		}

		public bool IsGoalTile(Vector3 position)
		{
			if (m_Section == null)
				return false;

			int tileX = Mathf.FloorToInt(position.X);
			int tileZ = Mathf.FloorToInt(position.Z);
			var goal = m_Section.GoalTile;
			return goal.X == tileX && goal.Z == tileZ;
		}

		public bool IsBlockedTile(Vector3 position)
		{
			TileData? tile = GetTileAt(position);
			if (tile == null)
				return true;

			return !tile.IsVisible;
		}

		public bool IsMoveBlocked(Vector3 from, Vector3 to)
		{
			TileData? tile = GetTileAt(to);
			if (tile == null || !tile.IsVisible)
				return true;

			if (!IsBridgeItem(tile.ItemType) || tile.BridgeOpen)
				return false;

			float dx = to.X - from.X;
			float dz = to.Z - from.Z;

			if (tile.ItemType == ItemType.BridgeLeftRight && Math.Abs(dx) > 0.01f)
				return true;

			if (tile.ItemType == ItemType.BridgeUpDown && Math.Abs(dz) > 0.01f)
				return true;

			return false;
		}

		public bool IsTrapDeadly(Vector3 position)
		{
			TileData? tile = GetTileAt(position);
			if (tile == null || !tile.IsVisible || tile.TrapType == TrapType.None)
				return false;

			return tile.TrapActive;
		}

		public void OnEntitySteppedTile(Vector3 position)
		{
			TileData? tile = GetTileAt(position);
			if (tile == null)
				tile = GetTileAt(new Vector3(position.X, position.Y, position.Z + 11.0f));

			if (tile == null || tile.ItemType != ItemType.PressurePlate)
				return;

			int plateX = GetTileColumn(position);
			ToggleBridgesForColumn(plateX);

			if (AC != null)
			{
				var ac = AC;
				Audio.PostEvent(new AudioCommandID("Click"), ref ac);
			}
		}

		private static int GetTileColumn(Vector3 position)
		{
			return Mathf.FloorToInt(position.X);
		}

		private void ToggleBridgesForColumn(int columnX)
		{
			if (m_Section == null)
				return;

			foreach (var kvp in m_Section.Tiles)
			{
				if (kvp.Key.X != columnX)
					continue;

				TileData tile = kvp.Value;
				if (!IsBridgeItem(tile.ItemType))
					continue;

				tile.BridgeOpen = !tile.BridgeOpen;
				UpdateBridgeVisual(tile);
			}

			UpdateBridgeHudLine();
			Log.Info("LevelManager: toggled bridges on column {0}", columnX);
		}

		private void UpdateBridgeHudLine()
		{
			int open = 0;
			int total = 0;

			if (m_Section != null)
			{
				foreach (var kvp in m_Section.Tiles)
				{
					if (!IsBridgeItem(kvp.Value.ItemType))
						continue;

					total++;
					if (kvp.Value.BridgeOpen)
						open++;
				}
			}

			HUD.SetLine(3, $"Bridges: {open}/{total}");
		}

		private static bool IsBridgeItem(ItemType item)
		{
			return item == ItemType.BridgeUpDown || item == ItemType.BridgeLeftRight;
		}

		private static void UpdateBridgeVisual(TileData tile)
		{
			if (tile.ItemEntity == null)
				return;

			float y = tile.BridgeOpen ? 0.45f : -0.25f;
			tile.ItemEntity.Translation = new Vector3(0, y, 0);
		}

		private void UpdateTraps(float ts)
		{
			if (m_Section == null)
				return;

			m_TrapTimer += ts;
			bool trapActive = ((int)(m_TrapTimer / 2.0f) % 2) == 0;

			foreach (var kvp in m_Section.Tiles)
			{
				TileData tile = kvp.Value;
				if (tile.TrapType == TrapType.None)
					continue;

				tile.TrapActive = trapActive;
				UpdateTrapVisual(tile);
			}
		}

		private static void UpdateTrapVisual(TileData tile)
		{
			if (tile.TrapEntity == null)
				return;

			float swing = tile.TrapActive ? 0.0f : 0.45f;
			if (tile.TrapType == TrapType.AxeLeftRight)
				tile.TrapEntity.Translation = new Vector3(swing, 0.85f, 0);
			else if (tile.TrapType == TrapType.AxeUpDown)
				tile.TrapEntity.Translation = new Vector3(0, 0.85f, swing);
		}

		private TileData? GetTileAt(Vector3 position)
		{
			if (m_Section == null)
				return null;

			int tileX = Mathf.FloorToInt(position.X);
			int tileZ = Mathf.FloorToInt(position.Z);
			return m_Section.GetTile(tileX, tileZ);
		}

		public bool IsDeadlyTile(Vector3 position)
		{
			if (m_Section == null)
				return false;

			int tileX = Mathf.FloorToInt(position.X);
			int tileZ = Mathf.FloorToInt(position.Z);
			TileData? tile = m_Section.GetTile(tileX, tileZ);
			if (tile == null || !tile.IsVisible)
				return false;

			return tile.Type == TileType.Spikes
				|| tile.Type == TileType.Lava
				|| tile.Type == TileType.LavaDeadly
				|| tile.Type == TileType.OceanDeadly
				|| tile.Type == TileType.DesertDeadly;
		}

		public void OnPlayerDied(Entity playerEntity)
		{
			if (AC != null)
			{
				var ac = AC;
				Audio.PostEvent(new AudioCommandID("Click"), ref ac);
			}

			var player = FindEntityByTag("Player")?.As<Player>();
			var replicator = FindEntityByTag("PlayerReplicator")?.As<PlayerReplicator>();

			if (playerEntity.As<Player>() != null && replicator != null)
				replicator.Die();
			else if (player != null)
				player.Die();

			FindEntityByTag("TimeManager")?.As<TimeManager>()?.Reset();
		}

		private void ScheduleTileAnimation(int tileX, int tileZ)
		{
			if (m_Section == null)
				return;

			TileData? tile = m_Section.GetTile(tileX, tileZ);
			if (tile == null || tile.TileEntity == null || tile.IsVisible)
				return;

			m_TilesToAnimate.Add(new TileAnimationInfo
			{
				Data = tile,
				StartLocation = tile.TileEntity.Translation,
				TargetLocation = new Vector3(tileX, 0, tileZ),
				AnimationSpeed = 2.0f,
				AnimationTime = 0.0f
			});
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

					Entity? tileEntity = levelContainer.InstantiateChild(tilePrefab, new Vector3(tx, -100, tzLevel));
					if (tileEntity == null)
						continue;

					ItemType item = level.Items[x, y];
					if (item == ItemType.Goal)
					{
						m_HasGoal = true;
						m_Section.GoalTile = new TileKey { X = (int)tx, Z = (int)tzLevel };
						sectionContainer.InstantiateChild(GoalItem, new Vector3(tx, 0.5f, tzSandbox));
					}
					else if (item == ItemType.PressurePlate)
					{
						sectionContainer.InstantiateChild(PressurePlateItem, new Vector3(tx, 0.15f, tzSandbox));
					}

					Entity? itemEntity = null;
					if (item != ItemType.None && m_ItemPrefabs.TryGetValue(item, out Prefab itemPrefab))
						itemEntity = tileEntity.InstantiateChild(itemPrefab, Vector3.Zero);

					TrapType trap = level.Traps[x, y];
					Entity? trapEntity = null;
					if (trap != TrapType.None)
						trapEntity = tileEntity.InstantiateChild(AxeTrapItem, Vector3.Zero);

					bool bridgeOpen = !IsBridgeItem(item);
					var tileData = new TileData
					{
						TileEntity = tileEntity,
						ItemEntity = itemEntity,
						TrapEntity = trapEntity,
						Type = tile,
						IsVisible = false,
						BridgeOpen = bridgeOpen,
						TrapActive = false,
						ItemType = item,
						TrapType = trap
					};

					if (IsBridgeItem(item))
						UpdateBridgeVisual(tileData);

					if (trap != TrapType.None)
						UpdateTrapVisual(tileData);

					m_Section.AddTile((int)tx, (int)tzLevel, tileData);
				}
			}

			m_TrapTimer = 0.0f;
			UpdateBridgeHudLine();
			HUD.SetLine(4, "Axe: alternating");
		}
	}
}
