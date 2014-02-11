#include <unistd.h>
#include <string.h>
#include <memory>

#include "framebuffer.h"


std::string imageFile = "";
std::string frameBufferDevice = "";
std::shared_ptr<Framebuffer> frameBuffer;

bool oneshot = false;
bool autozoom = false;


void printUsage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "sfivt " << ConsoleStyle(ConsoleStyle::CYAN) << "[OPTIONS] <FRAMEBUFFER> <IMAGEFILE>" << ConsoleStyle() << "." << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << ConsoleStyle(ConsoleStyle::CYAN) << "-1" << ConsoleStyle() << " - One-shot. Display image and quit." << std::endl;
    std::cout << ConsoleStyle(ConsoleStyle::CYAN) << "-a" << ConsoleStyle() << " - Auto-zoom. Fit image to framebuffer." << std::endl;
    std::cout << "e.g. \"sfivt -a /dev/fb1 ~/foo/bar.png\"." << std::endl;
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
        else if (argument == "-a") {
			autozoom = true;
        }
        else {
            //must be something else
            if (frameBufferDevice.empty()) {
				frameBufferDevice = argument;
			}
            else if (imageFile.empty()) {
                imageFile = argument;
            }
            else {
                std::cout << ConsoleStyle(ConsoleStyle::RED) << "Too many options!" << ConsoleStyle() << std::endl;
                printUsage();
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char * argv[])
{
    std::cout << ConsoleStyle(ConsoleStyle::CYAN) << "sfivt - A Simple Frambuffer Image viewing Tool." << ConsoleStyle() << std::endl;
    
    if (!parseCommandLine(argc, argv)) {
        return -1;
    }
    
    //create framebuffer
    frameBuffer = std::make_shared<Framebuffer>();
	if (!frameBuffer->isAvailable()) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Failed to initialize framebuffer!" << ConsoleStyle() << std::endl;
		return -2;
	}

	return 0;
}

