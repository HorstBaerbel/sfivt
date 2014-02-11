#pragma once

#include <inttypes.h>
#include <linux/fb.h>


class Framebuffer
{
public:
	enum PixelFormat { BAD_PIXELFORMAT, R8G8B8X8, X8R8G8B8, R8G8B8, X1R5G5B5, R5G6B5, I8 }; //!<The truecolor pixel formats we support.

    /*!
    Construct framebuffer interface and switch to new mode.
    \param[in] width Width of new framebuffer mode. If 0 uses current width.
    \param[in] height Height of new framebuffer mode. If 0 uses current height.
    \param[in] bitsPerPixel Bit depth of new framebuffer mode. If 0 uses current bit depth.
    \param[in] device Optional. Name of device to open.
    */
	Framebuffer(uint32_t width, uint32_t height, uint32_t bitsPerPixel, const std::string & device = "/dev/fb0");

    /*!
    Construct framebuffer interface and open it with the current dimensions and bit depth.
    \param[in] device Optional. Name of device to open.
    */
	Framebuffer(const std::string & device = "/dev/fb0");
	
	/*!
	Try to find out internal pixel format from framerbuffer var screen info.
	\param[in] screenInfo Screen info to match.
	\return Returns a matching PixelFormat or BAD_PIXELFORMAT.
	*/
	static PixelFormat screenInfoToPixelFormat(const struct fb_var_screeninfo & screenInfo);
	
    /*!
    Check if framebuffer interface is available.
    \return Returns true if the framebuffer interface can be used.
    */
	bool isAvailable() const;
	
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint32_t getBitsPerPixel() const;
    uint32_t getBytesPerPixel() const;
    PixelFormat getPixelFormat() const;

    /*!
    Draw raw image to framebuffer at position.
    \param[in] x Horizontal position where to draw image.
    \param[in] y Vertical position where to draw image.
    \param[in] data Pointer to raw image data.
    \param[in] width Width of image in pixels.
    \param[in] height Height of image in pixels.
    \param[in] sourceFormat \sa data pixel format.
    \note Should work for 32/24/16/15 bit pixel formats.
    */
    void drawBuffer(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);
	
	~Framebuffer();
	
private:
	int frameBufferDevice; //!<Framebuffer device handle.
	uint8_t * frameBuffer; //!<Pointer to memory-mapped raw framebuffer pixel data.
	uint32_t frameBufferSize; //!<Size of whole framebuffer in Bytes.
    uint32_t bytesPerPixel; //!<Bytes per pixel on screen.
    PixelFormat pixelFormat; //!<The pixel format the framebuffer has.

	struct fb_var_screeninfo oldMode; //!<Original framebuffer mode before mode switch.
	struct fb_var_screeninfo currentMode; //!<New framebuffer mode while application is running.
	struct fb_fix_screeninfo fixedMode; //!<Fixed mode information for various needs.

    /*!
    Construct framebuffer interface and switch to new mode.
    \param[in] width Width of new framebuffer mode. If 0 uses current width.
    \param[in] height Height of new framebuffer mode. If 0 uses current height.
    \param[in] bitsPerPixel Bit depth of new framebuffer mode. If 0 uses current bit depth.
    \param[in] device Name of device to open.
    */	
	void create(uint32_t width, uint32_t height, uint32_t bitsPerPixel, const std::string & device);
	void destroy();
};
