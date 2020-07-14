#include "World.h"

#include "../Utilities/Utils.h"
#include "../Protocol/Play.h"

namespace Minecraft::Server::Internal{
	World::World()
	{
	}
	World::~World()
	{
	}

	void World::init()
	{
		chunkMap.clear();
		lastPos = { -1000, -1000 };

		entityManager.init();
		inventory.init();

		Protocol::Play::PacketsOut::send_initial_inventory();
	}

	void World::cleanup()
	{
		inventory.cleanup();
		entityManager.cleanup();
		

		if (chunkMap.size() > 0) {
			for (auto& [pos, chunk] : chunkMap) {
				if (chunk != nullptr) {
					delete chunk;
				}
				chunkMap.erase(pos);
			}
		}
	}

	void World::tickUpdate()
	{
		entityManager.update();
	}

	void World::chunkgenUpdate()
	{
		glm::ivec2 v = { (float)((int)(Player::g_Player.x / (16.0f))) - 0.5f, (float)((int)(Player::g_Player.z / (16.0f))) - 0.5f };

		if (v != lastPos) {
			//WORLD MANAGEMENT
			glm::vec2 topLeft = { v.x - 3, v.y - 3 };
			glm::vec2 botRight = { v.x + 3, v.y + 3 };

			std::vector <mc::Vector3i> needed;
			needed.clear();
			std::vector<mc::Vector3i> excess;
			excess.clear();

			for (int x = topLeft.x; x <= botRight.x; x++) {
				for (int z = topLeft.y; z <= botRight.y; z++) {
					needed.push_back({ x, z, 0 });
				}
			}

			for (auto& [pos, chunk] : chunkMap) {
				bool need = false;
				for (auto& v : needed) {
					if (v == pos) {
						//Is needed
						need = true;
					}
				}
				

				if (!need) {
					excess.push_back(pos);
				}
			}

			//DIE OLD ONES!
			for (auto chk : excess) {
				Protocol::Play::PacketsOut::send_unload_chunk(chunkMap[chk]->getX(), chunkMap[chk]->getZ());
				delete chunkMap[chk];
				chunkMap.erase(chk);
			}

			//Make new
			for (auto chk : needed) {
				if (chunkMap.find(chk) == chunkMap.end()) {
					ChunkColumn* chunk = new ChunkColumn(chk.x, chk.y);

					for (int i = 0; i < 5; i++) {
						ChunkSection* chks = new ChunkSection(i);
						chks->generateTestData();
						chunk->addSection(chks);
					}

					Protocol::Play::PacketsOut::send_chunk(chunk, true);

					chunkMap.emplace(chk, std::move(chunk));
				}
			}
			lastPos = v;
		}

	}

	BlockID World::getBlockAtLocationAbsolute(int x, int y, int z)
	{
		int cX = x / 16;
		int cZ = z / 16;

		int corrX = x;
		int corrZ = z;

		if (x < 0) {
			corrX = 16 - x;
		}

		if (z < 0) {
			corrZ = 16 - z;
		}

		return getBlockAtLocationRelative(cX, cZ, corrX % 16, y, corrZ % 16);
	}

	BlockID World::getBlockAtLocationRelative(int chunkX, int chunkY, int x, int y, int z)
	{
		ChunkSection* section = chunkMap[mc::Vector3i(chunkX, chunkY)]->getSection(y / 16);
		if(section == NULL){
			return 0; //Doesn't exist
		}else{
			return section->getBlockAt(x, y % 16, z);
		}
	}

	void World::setBlockAtLocationAbsolute(int x, int y, int z, BlockID block)
	{
		int cX = x / 16;
		int cZ = z / 16;

		int corrX = x;
		int corrZ = z;

		if (x < 0) {
			corrX = 16 - x;
		}

		if (z < 0) {
			corrZ = 16 - z;
		}

		setBlockAtLocationRelative(cX, cZ, corrX % 16, y, corrZ % 16, block);
		utilityPrint("Set block ID at: " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " to " + std::to_string((int)block), LOGGER_LEVEL_TRACE);


		//Send block update packet
		Protocol::Play::PacketsOut::send_change_block(x, y, z, block);
	}

	void World::setBlockAtLocationRelative(int chunkX, int chunkY, int x, int y, int z, BlockID block)
	{
		ChunkSection* section = chunkMap[mc::Vector3i(chunkX, chunkY)]->getSection(y / 16);
		if (section == NULL) {
			//Doesn't exist, do nothing - note: may need to check where we are.
			chunkMap[mc::Vector3i(chunkX, chunkY)]->addSection(new ChunkSection(y / 16));
			section = chunkMap[mc::Vector3i(chunkX, chunkY)]->getSection(y / 16);
		}

		section->changeBlock(x, y, z, block);
	}

	World* g_World = NULL;
}
