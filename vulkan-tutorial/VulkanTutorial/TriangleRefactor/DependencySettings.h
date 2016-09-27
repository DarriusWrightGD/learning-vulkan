#pragma once
#include "di.h"
#include "Systems\Graphics\IVulkanGraphicsSystem.h"
#include "Application\HelloTriangle.h"
namespace di = boost::di;

static auto  getInjector() {
	auto injector = di::make_injector(di::bind<IVulkanGraphicsSystem>().to<VulkanGraphicsSystem>());
	return injector;
}