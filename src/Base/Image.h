#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include "Foundations.h"
#include "TypeUtils.h"

#include "glm/glm.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>


class Image;


DECLARE_PTR_TYPE(Image);
DECLARE_CONST_PTR_TYPE(Image);


// Pseudo color management
enum class ColorSpace
{
    None = 0,
    Raw,
    sRGB
};


class Image
{
public:
    ~Image() = default;

    inline uint32_t GetWidth() const { return m_width; }
    inline uint32_t GetHeight() const { return m_height; }

    inline const glm::vec4* GetPixels() const { return m_pixels.data(); }
    inline glm::vec4* GetPixels() { return m_pixels.data(); }

    inline const glm::vec4& GetPixel(const uint32_t& x, const uint32_t& y) const { return m_pixels[y * m_width + x]; }
    inline glm::vec4 GetPixel(const uint32_t& x, const uint32_t& y) { return m_pixels[y * m_width + x]; }

    static ImagePtr Read(const std::string& path, 
                         const ColorSpace& inputColorSpace=ColorSpace::None);

private:
    Image(const uint32_t& width, const uint32_t& height) : 
            m_width(width), m_height(height), m_pixels(width * height) {}

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    ColorSpace m_inputColorSpace;
    std::string m_filePath;

    // TODO: This variable could be templated to fit the image channels
    std::vector<glm::vec4> m_pixels;
};


#endif // IMAGEUTILS_H
