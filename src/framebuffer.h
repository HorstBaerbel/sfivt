#pragma once

#include <string>
#include <inttypes.h>
#include <linux/fb.h>


class Framebuffer
{
public:
	enum PixelFormat { BAD_PIXELFORMAT, R8G8B8X8, X8R8G8B8, R8G8B8, X1R5G5B5, R5G6B5, GREY8 }; //!<The truecolor pixel formats we support.
	
	/*! Structure holding some info about a pixel format. */
	struct PixelFormatInfo
	{
		PixelFormat format;
		uint32_t bitsPerPixel;
		uint32_t bytesPerPixel;
		uint32_t bitsRed; //!<How many bits the red color component has.
		uint32_t bitsGreen;
		uint32_t bitsBlue;
		uint32_t bitsAlpha;
		uint32_t shiftRed; //!<At what bit position red can be found in the pixel data.
		uint32_t shiftGreen;
		uint32_t shiftBlue;
		uint32_t shiftAlpha;
		std::string name;
	};
	
	/*! List holding information about the different pixel formats in \sa PixelFormat. */
	static const PixelFormatInfo pixelFormatInfo[];

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
	Convert colors from one pixel format to another.
	\param[in] destFormat Output color pixel format.
	\param[in] source Input color source pointer.
	\param[in] sourceFormat Input color pixel format.
	\param[in] count Number of consecutive pixels to convert.
	\return Returns a new buffer with the converted data. YOU have to delete [] it when you're done with it.
	\note This is slow. Usage scenario is to convert a single color for clear() or convert a whole image once before blit()ting it multiple times.
	*/
	static uint8_t * convertToPixelFormat(PixelFormat destFormat, const uint8_t * source, PixelFormat sourceFormat, size_t count = 1);
	
	/*!
	Convert colors from one pixel format to framebuffer format.
	\param[in] source Input color source pointer.
	\param[in] sourceFormat Input color pixel format.
	\param[in] count Number of consecutive pixels to convert.
	\return Returns a new buffer with the converted data. YOU have to delete [] it when you're done with it.
	\note This is slow. Usage scenario is to convert a single color for clear() or convert a whole image once before blit()ting it multiple times.
	*/
	uint8_t * convertToFramebufferFormat(const uint8_t * source, PixelFormat sourceFormat, size_t count = 1);
	
	/*!
	Check if framebuffer interface is available.
	\return Returns true if the framebuffer interface can be used.
	*/
	bool isAvailable() const;
	
	uint32_t getWidth() const;
	uint32_t getHeight() const;
	PixelFormat getFormat() const;
	PixelFormatInfo getFormatInfo() const;

	/*!
	Fill rect in framebuffer at position.
	\param[in] color Pointer to raw color data. MUST BE IN FRAMEBUFFER PIXEL FORMAT!
	*/
	void clear(const uint8_t * color);

	/*!
	Draw raw image to framebuffer at position.
	\param[in] x Horizontal position where to draw image in framebuffer.
	\param[in] y Vertical position where to draw image in framebuffer.
	\param[in] data Pointer to raw source image data.
	\param[in] width Width of source image in pixels.
	\param[in] height Height of source image in pixels.
	\param[in] sourceFormat Source \sa data pixel format.
	\note Should work for 32/24/16/15 bit pixel formats.
	*/
	void blit(uint32_t x, uint32_t y, const uint8_t * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);
	
	~Framebuffer();
	
private:
	void blit_copy(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height);
	void blit_R8G8B8X8(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);
	void blit_X8R8G8B8(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);
	void blit_R8G8B8(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);
	void blit_X1R5G5B5(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);
	void blit_R5G6B5(uint32_t x, uint32_t y, const unsigned char * data, uint32_t width, uint32_t height, PixelFormat sourceFormat);

	/*!
	Construct framebuffer interface and switch to new mode.
	\param[in] width Width of new framebuffer mode. If 0 uses current width.
	\param[in] height Height of new framebuffer mode. If 0 uses current height.
	\param[in] bitsPerPixel Bit depth of new framebuffer mode. If 0 uses current bit depth.
	\param[in] device Name of device to open.
	*/	
	void create(uint32_t width, uint32_t height, uint32_t bitsPerPixel, const std::string & device);
	
	void destroy();

	int m_frameBufferDevice; //!<Framebuffer device handle.
	uint8_t * m_frameBuffer; //!<Pointer to memory-mapped raw framebuffer pixel data.
	uint32_t m_frameBufferSize; //!<Size of whole framebuffer in Bytes.
	PixelFormat m_format; //!<The pixel format the framebuffer has.
	PixelFormatInfo m_formatInfo; //!<Information about the pixel format the framebuffer has.

	struct fb_var_screeninfo m_oldMode; //!<Original framebuffer mode before mode switch.
	struct fb_var_screeninfo m_currentMode; //!<New framebuffer mode while application is running.
	struct fb_fix_screeninfo m_fixedMode; //!<Fixed mode information for various needs.
};
