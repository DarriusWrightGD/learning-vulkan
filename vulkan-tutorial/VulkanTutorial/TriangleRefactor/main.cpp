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
	auto app = getInjector().create<shared_ptr<HelloTriangle>>();
	try {
		app->run();
	}
	catch (const std::runtime_error & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}