VULKAN_SDK_PATH = ./vulkan
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan -lpthread
SOURCES = main.cpp app.cpp

VulkanSampleApp: main.cpp
	g++ $(CFLAGS) -g -o VulkanSampleApp $(SOURCES) $(LDFLAGS)

.PHONY: test clean

test: VulkanSampleApp
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib
	VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d
	./VulkanSampleApp

clean:
	rm -r VulkanSampleApp

