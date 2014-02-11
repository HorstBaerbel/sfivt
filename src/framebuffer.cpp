#include "framebuffer.h"

#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "consolestyle.h"

Framebuffer::Framebuffer(const std::string & device)
	: frameBufferDevice(0), frameBuffer(nullptr), frameBufferSize(0), bytesPerPixel(0), pixelFormat(BAD_PIXELFORMAT)
{
	create(0, 0, 0, device);
}

Framebuffer::Framebuffer(uint32_t width, uint32_t height, uint32_t bitsPerPixel, const std::string & device)
	: frameBufferDevice(0), frameBuffer(nullptr), frameBufferSize(0), bytesPerPixel(0), pixelFormat(BAD_PIXELFORMAT)
{
	create(width, height, bitsPerPixel, device);
}

void Framebuffer::create(uint32_t width, uint32_t height, uint32_t bitsPerPixel, const std::string & device)
{
    struct fb_var_screeninfo orig_vinfo;
    long int screensize = 0;
    
    std::cout << "Opening framebuffer " << device << "..." << std::endl;

    //open the framebuffer for reading/writing
    frameBufferDevice = open(device.c_str(), O_RDWR);
    if (frameBufferDevice <= 0) {
        std::cout << ConsoleStyle(ConsoleStyle::RED) << "Failed to open " << device << " for reading/writing!" << ConsoleStyle() << std::endl;
		frameBufferDevice = 0;
        return;
    }

    //get current mode information
    if (ioctl(frameBufferDevice, FBIOGET_VSCREENINFO, &currentMode)) {
        std::cout << ConsoleStyle(ConsoleStyle::YELLOW) << "Failed to read variable mode information!" << ConsoleStyle() << std::endl;
    }
    else {
        std::cout << "Original mode is " << currentMode.xres << "x" << currentMode.yres << "@" << currentMode.bits_per_pixel;
        std::cout << " with virtual resolution " << currentMode.xres_virtual << "x" << currentMode.yres_virtual << "." << std::endl;
    }
    //store screen mode for restoring it
    memcpy(&oldMode, &currentMode, sizeof(fb_var_screeninfo));

    //change screen mode. check if the user passed some values
    if (width != 0) {
        currentMode.xres = width;
    }
    if (height != 0) {
        currentMode.yres = height;
    }
    if (bitsPerPixel != 0) {
        currentMode.bits_per_pixel = bitsPerPixel;
    }
    currentMode.xres_virtual = currentMode.xres;
    currentMode.yres_virtual = currentMode.yres;
    if (ioctl(frameBufferDevice, FBIOPUT_VSCREENINFO, &currentMode)) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Failed to set mode to " << currentMode.xres << "x" << currentMode.yres << "@" << currentMode.bits_per_pixel << "!" << ConsoleStyle() << std::endl;
    }
    
    //get fixed screen information
    if (ioctl(frameBufferDevice, FBIOGET_FSCREENINFO, &fixedMode)) {
        std::cout << ConsoleStyle(ConsoleStyle::YELLOW) << "Failed to read fixed mode information!" << ConsoleStyle() << std::endl;
    }
    
    //try to match an internal pixel format to the mode we got
    pixelFormat = screenInfoToPixelFormat(currentMode);
    //check if we can use it
    if (pixelFormat == BAD_PIXELFORMAT) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Unusable pixel format!" << ConsoleStyle() << std::endl;
        destroy();
        return;
	}

    //map framebuffer into user memory. round up to next bigger in [8/16/24/32]
    bytesPerPixel = ((currentMode.bits_per_pixel + 7) / 8);
    frameBufferSize = currentMode.yres * fixedMode.line_length;
    frameBuffer = (unsigned char *)mmap(nullptr, frameBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, frameBufferDevice, 0);
    if (frameBuffer == MAP_FAILED) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Failed to map framebuffer to user memory!" << ConsoleStyle() << std::endl;
        destroy();
        return;
    }
    
    //dump some info
	std::cout << ConsoleStyle(ConsoleStyle::GREEN) << "Opened a " << currentMode.xres << "x" << currentMode.yres << "@" << currentMode.bits_per_pixel << " display";
    std::cout << " with virtual resolution " << currentMode.xres_virtual << "x" << currentMode.yres_virtual << "." << ConsoleStyle() << std::endl;
    std::cout << ConsoleStyle(ConsoleStyle::GREEN) << "Pixel format is ";
    switch (pixelFormat) {
		case R8G8B8X8:
			std::cout << "RGB32 with an R8G8B8X8 layout";
			break;
		case X8R8G8B8:
			std::cout << "RGB32 with an X8R8G8B8 layout";
			break;
		case R8G8B8:
			std::cout << "RGB24 with an R8G8B8 layout";
			break;
		case X1R5G5B5:
			std::cout << "RGB16 with an X1R5G5B5 layout";
			break;
		case R5G6B5:
			std::cout << "RGB16 with an R5G6B5 layout";
			break;
		default:
			std::cout << "Unknown pixel format.";
	};
	std::cout << std::endl;

    //draw blue debug rectangle
    /*for (int y = 0; y < currentMode.yres; ++y) {
        for (int x = 0; x < currentMode.xres; ++x) {
            ((unsigned int *)frameBuffer)[y * currentMode.width + x] = 0xFF1199DD;
        }
    }*/
}

Framebuffer::PixelFormat Framebuffer::screenInfoToPixelFormat(const struct fb_var_screeninfo & screenInfo)
{
	if (screenInfo.bits_per_pixel == 32) {
		if (screenInfo.transp.offset >= 24) {
			return R8G8B8X8;
		}
		else if (screenInfo.transp.offset <= 8) {
			return X8R8G8B8;
		}
	}
	else if (screenInfo.bits_per_pixel == 24) {
		return R8G8B8;
	}
	else if (screenInfo.bits_per_pixel == 16) {
		if (screenInfo.transp.length == 0) {
			if (screenInfo.red.length == 6 | screenInfo.green.length == 6 | screenInfo.blue.length == 6) {
				return R5G6B5;
			}
			else {
				return X1R5G5B5;
			}
		}
		else if (screenInfo.transp.length == 1) {
			return X1R5G5B5;
		}
	}
	else if (screenInfo.bits_per_pixel == 15) {
		return X1R5G5B5;
	}
	return BAD_PIXELFORMAT;
}

bool Framebuffer::isAvailable() const
{
	return (frameBuffer != nullptr && frameBufferDevice != 0);
}

uint32_t Framebuffer::getWidth() const
{
    return currentMode.xres;
}

uint32_t Framebuffer::getHeight() const
{
    return currentMode.yres;
}

uint32_t Framebuffer::getBitsPerPixel() const
{
    return currentMode.bits_per_pixel;
}

uint32_t Framebuffer::getBytesPerPixel() const
{
    return currentMode.bytes_per_pixel;
}

PixelFormat Framebuffer::getPixelFormat() const
{
	return pixelFormat;
}

void Framebuffer::drawBuffer(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat)
{
    //std::cout << "Frame " << width << "x" << height << "@" << bpp << std::endl;
    const uint32_t srcLineLength = width * ((bpp + 7) / 8);
    //std::cout << "Bpp " << currentMode.bits_per_pixel << ", Bytespp " << bytesPerPixel << ", Bytespsl " << bytesPerScanline << std::endl;
    uint8_t * dest = frameBuffer + (y + currentMode.yoffset) * fixedMode.line_length + (x + currentMode.xoffset) * bytesPerPixel;
    if (pixelFormat == sourceFormat) {
        for (int line = 0; line < height; ++line) {
            memcpy(dest, data, srcLineLength);
            dest += fixedMode.line_length;
            data += srcLineLength;
        }
    }
    else if (pixelFormat == R8G8B8X8) {
        if (sourceFormat == I8) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine++) {
                    *destLine = *srcLine << 24 | *srcLine << 16 | *srcLine << 8 | 0xff;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
		else if (sourceFormat == X1R5G5B5) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 2) {
                    *destLine = (*srcLine & 0x7c00) << 14 | (*srcLine & 0x3e0) << 11 | (*srcLine & 0x1f) << 8 | 0xff;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
		else if (sourceFormat == R5G6B5) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 2) {
                    *destLine = (*srcLine & 0xf800) << 14 | (*srcLine & 0x7e0) << 11 | (*srcLine & 0x1f) << 8 | 0xff;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (sourceFormat == R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 3) {
                    *destLine = srcLine[2] << 24 | srcLine[1] << 16 | srcLine[0] << 8 | 0xff;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
    }
    else if (pixelFormat == X8R8G8B8) {
        if (sourceFormat == I8) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine++) {
                    *destLine = 0xff000000 | *srcLine << 16 | *srcLine << 8 | *srcLine;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
		else if (sourceFormat == X1R5G5B5) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 2) {
                    *destLine = 0xff000000 | (*srcLine & 0x7c00) << 6 | (*srcLine & 0x3e0) << 3 | (*srcLine & 0x1f);
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
		else if (sourceFormat == R5G6B5) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 2) {
                    *destLine = 0xff000000 | (*srcLine & 0xf800) << 5 | (*srcLine & 0x7e0) << 3 | (*srcLine & 0x1f);
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (sourceFormat == R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint32_t * destLine = (uint32_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 3) {
                    *destLine = 0xff000000 | srcLine[2] << 16 | srcLine[1] << 8 | srcLine[0];
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
    }
    else if (pixelFormat == R8G8B8) {
        if (sourceFormat == I8) {
            for (int line = 0; line < height; ++line) {
                uint8_t * destLine = dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine+=3, srcLine++) {
                    destLine[0] = *srcLine;
                    destLine[1] = *srcLine;
                    destLine[2] = *srcLine;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        if (sourceFormat == X1R5G5B5) {
            for (int line = 0; line < height; ++line) {
                uint8_t * destLine = dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 2) {
                    destLine[0] = (*srcLine & 0x7c00) >> 10;
                    destLine[1] = (*srcLine & 0x03e0) >> 5;
                    destLine[2] = (*srcLine & 0x001f);
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        if (sourceFormat == R5G6B5) {
            for (int line = 0; line < height; ++line) {
                uint8_t * destLine = dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine += 2) {
                    destLine[0] = (*srcLine & 0xf800) >> 11;
                    destLine[1] = (*srcLine & 0x07e0) >> 5;
                    destLine[2] = (*srcLine & 0x001f);
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (R8G8B8X0) {
            for (int line = 0; line < height; ++line) {
                uint8_t * destLine = dest;
                const uint32_t * srcLine = (uint32_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine+=3, srcLine++) {
                    destLine[0] = srcLine[3];
                    destLine[1] = srcLine[2];
                    destLine[2] = srcLine[1];
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (X0R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint8_t * destLine = dest;
                const uint32_t * srcLine = (uint32_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine+=3, srcLine++) {
                    destLine[0] = srcLine[2];
                    destLine[1] = srcLine[1];
                    destLine[2] = srcLine[0];
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
    }
	else if (pixelFormat == X1R5G5B5) {
        if (sourceFormat == I8) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine++) {
                    *destLine = 0x8000 | (*srcLine & 0xf8) << 7 | (*srcLine & 0xf8) << 2 | (*srcLine & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        if (sourceFormat == R5G6B5) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine++) {
                    *destLine = 0x8000 | (*srcLine & 0xffc0) >> 1 | (*srcLine & 0x001f);
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine+=3) {
					*destLine = 0x8000 | (srcLine[2] & 0xf8) << 7 | (srcLine[1] & 0xf8) << 2 | (srcLine[0] & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (R8G8B8X0) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine+=4) {
                    *destLine = 0x8000 | (srcLine[3] & 0xf8) << 7 | (srcLine[2] & 0xf8) << 2 | (srcLine[1] & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (X0R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine+=4) {
                    *destLine = 0x8000 | (srcLine[2] & 0xf8) << 7 | (srcLine[1] & 0xf8) << 2 | (srcLine[0] & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
    }
	else if (pixelFormat == R5G6B5) {
        if (sourceFormat == I8) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine++) {
                    *destLine = (*srcLine & 0xf8) << 8 | (*srcLine & 0xfc) << 3 | (*srcLine & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        if (sourceFormat == X1R5G5B5) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint16_t * srcLine = (const uint16_t *)data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine++) {
                    *destLine = (*srcLine & 0x7ff0) << 1 | (*srcLine & 0x001f);
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine+=3) {
                    *destLine = (srcLine[2] & 0xf8) << 8 | (srcLine[1] & 0xfc) << 3 | (srcLine[0] & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (R8G8B8X0) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine+=4) {
                    *destLine = (srcLine[3] & 0xf8) << 8 | (srcLine[2] & 0xfc) << 3 | (srcLine[1] & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
        else if (X0R8G8B8) {
            for (int line = 0; line < height; ++line) {
                uint16_t * destLine = (uint16_t *)dest;
                const uint8_t * srcLine = data;
                for (int pixel = 0; pixel < width; ++pixel, destLine++, srcLine+=4) {
                    *destLine = (srcLine[2] & 0xf8) << 8 | (srcLine[1] & 0xfc) << 3 | (srcLine[0] & 0xf8) >> 3;
                }
                dest += fixedMode.line_length;
                data += srcLineLength;
            }
        }
    }
}

void Framebuffer::destroy()
{
	std::cout << "Closing framebuffer..." << std::endl;
    
    if (frameBuffer != nullptr && frameBuffer != MAP_FAILED) {
        munmap(frameBuffer, frameBufferSize);
    }
    frameBuffer = nullptr;
    frameBufferSize = 0;

    if (frameBufferDevice != 0) {
        //reset old screen mode
        ioctl(frameBufferDevice, FBIOPUT_VSCREENINFO, &oldMode);
        frameBufferDevice = 0;
        //close device
        close(frameBufferDevice);
    }
}

Framebuffer::~Framebuffer()
{
    destroy();
}
