#pragma once
#include <iostream>
using std::cout;
using std::endl;
class IVulkanGraphicsSystem
{
public:
	IVulkanGraphicsSystem()
	{

	}
	//virtual ~IVulkanGraphicsSystem() noexcept = default;
};

class VulkanGraphicsSystem : public IVulkanGraphicsSystem
{
public:
	VulkanGraphicsSystem()
	{
		cout << "using the system" << endl;
	}

};