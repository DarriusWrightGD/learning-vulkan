#include "Application\HelloTriangle.h"
#include "DependencySettings.h"
#include <memory>

#include "di.h"
#include "Systems\Graphics\IVulkanGraphicsSystem.h"
#include "Application\HelloTriangle.h"
namespace di = boost::di;
#include <memory>
using namespace std;

int main() {
	//auto injector = di::make_injector(di::bind<IVulkanGraphicsSystem>().to<VulkanGraphicsSystem>());
	auto injector = di::make_injector(di::bind<IVulkanGraphicsSystem>().to<VulkanGraphicsSystem>());
	auto app = injector.create<shared_ptr<HelloTriangle>>();
	//auto app = HelloTriangle(shared_ptr<VulkanGraphicsSystem>());
	try {
		app->run();
	}
	catch (const std::runtime_error & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}