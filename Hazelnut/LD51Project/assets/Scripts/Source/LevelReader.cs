using System;
using System.Drawing;

using Hazel;

namespace LD51
{
	public enum TileType
	{
		None = 0,
		Grass = 1,
		Spikes = 30,
		Lava = 60,
		LavaDeadly = 90,
		Ocean = 120,
		OceanDeadly = 150,
		Desert = 180,
		DesertDeadly = 210
	}

	public enum ItemType
	{
		None = 0,
		PressurePlate = 30,
		BridgeUpDown = 60,
		BridgeLeftRight = 150,
		Goal = 250
	}

	public enum TrapType
	{
		None = 0,
		AxeLeftRight = 30,
		AxeUpDown = 80
	}

	public class LevelReader
	{
		private TileType[,] m_Tiles = new TileType[11, 11];
		private ItemType[,] m_Items = new ItemType[11, 11];
		private TrapType[,] m_Traps = new TrapType[11, 11];

		public TileType[,] Tiles => m_Tiles;
		public ItemType[,] Items => m_Items;
		public TrapType[,] Traps => m_Traps;

		public LevelReader() { }

		public LevelReader(string filepath)
		{
			using Bitmap bitmap = new Bitmap(filepath);
			if (bitmap.Width != 11 || bitmap.Height != 11)
			{
				Log.Warn($"Level '{filepath}' must be 11x11 pixels (got {bitmap.Width}x{bitmap.Height})");
				return;
			}

			for (int y = 0; y < 11; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					Color color = bitmap.GetPixel(x, y);

					if (!Enum.IsDefined(typeof(TileType), (int)color.R))
					{
						Log.Error($"Undefined red value '{color.R}' in '{filepath}' at {x}, {y}");
						m_Tiles[x, y] = TileType.None;
						continue;
					}

					if (!Enum.IsDefined(typeof(ItemType), (int)color.G))
					{
						Log.Error($"Undefined green value '{color.G}' in '{filepath}' at {x}, {y}");
						m_Items[x, y] = ItemType.None;
						continue;
					}

					if (!Enum.IsDefined(typeof(TrapType), (int)color.B))
					{
						Log.Error($"Undefined blue value '{color.B}' in '{filepath}' at {x}, {y}");
						m_Traps[x, y] = TrapType.None;
						continue;
					}

					m_Tiles[x, y] = (TileType)color.R;
					m_Items[x, y] = (ItemType)color.G;
					m_Traps[x, y] = (TrapType)color.B;
				}
			}
		}
	}
}
