#default build platform
#valid values: linux, win32, darwin
PLATFORM := linux

ifeq ($(PLATFORM),linux)
    DIR_NAME := OpenDeckConfigurator-linux-x64
    FILE_PATH := $(DIR_NAME)/resources
else ifeq ($(PLATFORM),win32)
    DIR_NAME := OpenDeckConfigurator-win32-x64
    FILE_PATH := $(DIR_NAME)/resources
else ifeq ($(PLATFORM),darwin)
    DIR_NAME := OpenDeckConfigurator-darwin-x64
    FILE_PATH := $(DIR_NAME)/OpenDeckConfigurator.app/Contents/Resources
else
    $(error Unsupported platform. Valid options are: linux, win32, darwin)
endif

elektron-pkg:
	@electron-packager ./ OpenDeckConfigurator --platform=$(PLATFORM) --arch=x64 --electron-version=4.0.5 --overwrite --ignore=build
	@cp ../index.html ./$(FILE_PATH)/app/index.html
	@mkdir -p ./$(FILE_PATH)/app/webui
	@mv ./$(FILE_PATH)/app/app/ ./$(FILE_PATH)/app/webui
	@mv ./$(FILE_PATH)/app/css/ ./$(FILE_PATH)/app/webui
	@mv ./$(FILE_PATH)/app/fonts/ ./$(FILE_PATH)/app/webui
	@mv ./$(FILE_PATH)/app/images/ ./$(FILE_PATH)/app/webui
	@mv ./$(FILE_PATH)/app/vendor/ ./$(FILE_PATH)/app/webui
	@rm ./$(FILE_PATH)/app/Makefile
	zip -r $(DIR_NAME).zip $(DIR_NAME)
	@mkdir -p build
	@mv ./$(DIR_NAME)/ ./build/$(DIR_NAME)
	@mv ./$(DIR_NAME).zip ./build/$(DIR_NAME).zip

clean:
	@echo Cleaning up.
	@rm -rf build/