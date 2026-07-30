using System;
using System.Collections.Generic;

using Hazel;

namespace LD51
{
	public struct TileKey
	{
		public int X;
		public int Z;
	}

	public class TileData
	{
		public Entity? TileEntity;
		public TileType Type;
		public bool IsVisible;
		public TrapType TrapType;
		public ItemType ItemType;
	}

	public class LevelSection
	{
		public Entity? SandboxEntity;
		public Entity? LevelEntity;
		public TileKey GoalTile { get; set; }

		private readonly Dictionary<TileKey, TileData> m_Tiles = new Dictionary<TileKey, TileData>();
		public IReadOnlyDictionary<TileKey, TileData> Tiles => m_Tiles;

		public void AddTile(int tileX, int tileZ, TileData tileData)
		{
			var key = new TileKey { X = tileX, Z = tileZ };
			if (m_Tiles.ContainsKey(key))
			{
				Log.Error("Duplicate tile at {0}, {1}", tileX, tileZ);
				return;
			}
			m_Tiles.Add(key, tileData);
		}

		public TileData? GetTile(int tileX, int tileZ)
		{
			var key = new TileKey { X = tileX, Z = tileZ };
			return m_Tiles.TryGetValue(key, out var tile) ? tile : null;
		}
	}
}
