#pragma once

#include <string>
#include <vector>
#include <FreeImage.h>


class ImageIO
{
public:
	/*!
	Load image from file to 32bit RGBA data and resize to given dimensions.
	\param[in] fileName Path to file to load.
	\param[in, out] width Optional. Target width of image. Pass 0 to return original image dimensions. Upon return contains the actual image width.
	\param[in, out] height Optional. Target height of image. Pass 0 to return original image dimensions. Upon return contains the actual image height.
	\param[in] keepAspectRatio Optional. Pass true to keep the aspect ratio when resizing.
	\return Returns the image data on success or an empty vector on failure.
	\note When resizing with \sa keepAspectRatio makes the image fit completely inside the rectangle \sa width x \sa height.
	*/
	static std::vector<uint8_t> loadFile_RGBA32(const std::string & fileName, uint32_t & width, uint32_t & height, bool keepAspectRatio = true);
};
