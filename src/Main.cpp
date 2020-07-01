#include <Platform/Platform.h>
#include "Server.h"
#include <Utilities/Logger.h>

#if CURRENT_PLATFORM == PLATFORM_PSP
PSP_MODULE_INFO("Craft Server", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU | THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);
#endif

using namespace Stardust;
using namespace Stardust::Utilities;
using namespace Minecraft::Server;

int main() {
	Platform::initPlatform("Craft-Server");

#ifdef CRAFT_SERVER_DEBUG
	app_Logger->currentLevel = LOGGER_LEVEL_TRACE;
	detail::core_Logger->currentLevel = LOGGER_LEVEL_TRACE;

#if CURRENT_PLATFORM == PLATFORM_PSP
	pspDebugScreenInit();
#endif
	utilityPrint("Debug enabled.", LOGGER_LEVEL_DEBUG);
#endif

	try {
		g_Server->init();
	}catch(std::runtime_error e){
		utilityPrint(e.what(), LOGGER_LEVEL_ERROR);


#if CURRENT_PLATFORM == PLATFORM_PSP
		sceKernelDelayThread(1000 * 1000 * 3);
#else
		std::this_thread::sleep_for(std::chrono::seconds(3));
#endif
		Platform::exitPlatform();
	}

	while (g_Server->isRunning()) {
		Platform::platformUpdate();

		g_Server->update();

		//Note: Should actually count this out - but will do
#if CURRENT_PLATFORM == PLATFORM_PSP
		sceKernelDelayThread(1000 * 50);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
	}


	g_NetworkDriver.Cleanup();

	Platform::exitPlatform();
	return 0;
}