#include <unistd.h>
#include <string>
#include <iostream>
#include <memory>

#include "framebuffer.h"
#include "imageIO.h"


std::string imageFile = "";
std::string frameBufferDevice = "";
std::shared_ptr<Framebuffer> frameBuffer;

bool oneshot = false;
//bool autozoom = false;


void printUsage()
{
	std::cout << "Usage:" << std::endl;
	std::cout << "sfivt " << "[OPTIONS] <FRAMEBUFFER> <IMAGEFILE>" << "." << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "-1" << " - One-shot. Display image and quit." << std::endl;
	//std::cout << "-a" << " - Auto-zoom. Fit image to framebuffer." << std::endl;
	std::cout << "e.g. \"sfivt -1 /dev/fb1 ~/foo/bar.png\"." << std::endl;
	std::cout << "svift can read all formats that FreeImage can, so more or less: JPG/PNG/TIFF/BMP/TGA/GIF." << std::endl;
}

bool parseCommandLine(int argc, char * argv[])
{
	//parse command line arguments
	for(int i = 1; i < argc; ++i) {
		//read argument from list
		std::string argument = argv[i];
		//check what it is
		if (argument == "?" || argument == "--help") {
			//print help
			printUsage();
			return false;
		}
		else if (argument == "-1") {
			oneshot = true;
		}
		/*else if (argument == "-a") {
			autozoom = true;
		}*/
		else {
			//must be something else
			if (frameBufferDevice.empty()) {
				frameBufferDevice = argument;
			}
			else if (imageFile.empty()) {
				imageFile = argument;
			}
			else {
				std::cout << "Too many options!" << std::endl;
				printUsage();
				return false;
			}
		}
	}
	return true;
}

int main(int argc, char * argv[])
{
	std::cout << "sfivt - A Simple Frambuffer Image viewing Tool." << std::endl;
	
	if (argc < 3 || argc > 5) {
		printUsage();
		return -1;
	}
	
	if (!parseCommandLine(argc, argv)) {
		return -1;
	}
	
	//create framebuffer
	frameBuffer = std::make_shared<Framebuffer>(frameBufferDevice);
	if (!frameBuffer->isAvailable()) {
		std::cout << "Failed to initialize framebuffer!" << std::endl;
		return -2;
	}
	
	//try loading the image
	uint32_t width = frameBuffer->getWidth();
	uint32_t height = frameBuffer->getHeight();
	std::vector<uint8_t> data = ImageIO::loadFile_RGBA32(imageFile, width, height);
	if (data.empty()) {
		std::cout << "Failed to load image!" << std::endl;
		return -3;
	}
	
	//wait for input?
	if (!oneshot) {
		//hide cursor
		std::cout << "\e[?1;0;127c";
	}
	
	//clear framebuffer to black
	uint32_t inColor = 0;
	uint8_t * clearColor = frameBuffer->convertToFramebufferFormat((const uint8_t *)&inColor, Framebuffer::X8R8G8B8);
	frameBuffer->clear(clearColor);
	delete [] clearColor;
	
	//display the image centered on screen
	uint32_t x = width < frameBuffer->getWidth() ? (frameBuffer->getWidth() - width) / 2 : 0;
	uint32_t y = height < frameBuffer->getHeight() ? (frameBuffer->getHeight() - height) / 2 : 0;
	frameBuffer->blit(x, y, data.data(), width, height, Framebuffer::X8R8G8B8);

	//wait for input?
	if (!oneshot) {
		//wait for user return
		std::cin.get();
		//unhide cursor
		std::cout << "\e[?0;0;0c";
	}

	return 0;
}

