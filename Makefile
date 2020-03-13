VULKAN_SDK_PATH = ./vulkan
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan -lpthread
SOURCES = main.cpp app.cpp

export LD_LIBRARY_PATH="$(VULKAN_SDK_PATH)"/lib
export VK_LAYER_PATH="$(VULKAN_SDK_PATH)"/etc/vulkan/explicit_layer.d

ShadedCubeApp: main.cpp
	@echo "$(LD_LIBRARY_PATH)"
	@echo "$(VK_LAYER_PATH)"
	g++ $(CFLAGS) -g -o ShadedCubeApp $(SOURCES) $(LDFLAGS)

.PHONY: test clean

test: ShadedCubeApp
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) VK_LAYER_PATH=$(VK_LAYER_PATH) ./ShadedCubeApp $(shader)

clean:
	rm -r ShadedCubeApp

