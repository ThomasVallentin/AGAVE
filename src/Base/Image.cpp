#include "Image.h"

#include "Logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


ColorSpace GuessColorSpace(const std::string& ext) 
{
    if (ext == ".png")
        return ColorSpace::sRGB;
    if (ext == ".jpg")
        return ColorSpace::sRGB;
    if (ext == ".jpeg")
        return ColorSpace::sRGB;
    if (ext == ".bmp")
        return ColorSpace::sRGB;
    if (ext == ".tga")
        return ColorSpace::Raw;
    if (ext == ".hdr")
        return ColorSpace::Raw;
    if (ext == ".pgm")
        return ColorSpace::Raw;
    if (ext == ".ppm")
        return ColorSpace::Raw;

    return ColorSpace::Raw;
}


ImagePtr Image::Read(const std::string& path, 
                     const ColorSpace& inputColorSpace) {

    ColorSpace colorSpace = inputColorSpace;
    if (colorSpace == ColorSpace::None)
    {
        const size_t dot = path.find_last_of(".");
        if (dot != std::string::npos)
        {
            colorSpace = GuessColorSpace(path.substr(dot));
        }
        else
        {
            colorSpace = ColorSpace::Raw;
        }
    }

    // Simple gamma handling based on the input colorspace of the image. 
    // Further work should be achieved if we want a more advanced color system
    switch (colorSpace) {
        case ColorSpace::Raw:
            stbi_ldr_to_hdr_gamma(1.0f);
            break;
        
        case ColorSpace::sRGB:
            stbi_ldr_to_hdr_gamma(2.2f);
            break;
    }
    stbi_set_flip_vertically_on_load(true); 

    // Image doesn't exist, opening it
    int width, height, channels;
    float *img = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
    if(img == nullptr) {
        LOG_ERROR("Could not read image %s...", path.c_str());
        return ImagePtr();
    }

    Image* image = new Image(width, height);
    image->m_inputColorSpace = colorSpace;
    image->m_filePath = path;
    glm::vec4* pixels = image->GetPixels();

    switch (channels) {
        case 4:
        {
            for (size_t i = 0 ; i < width * height ; i++) {
                pixels[i] = glm::vec4(img[i * channels],
                                      img[i * channels + 1],
                                      img[i * channels + 2],
                                      img[i * channels + 3]);
            }
            break;
        }

        case 3:
        {
            for (size_t i = 0 ; i < width * height ; i++) {
                pixels[i] = glm::vec4(img[i * channels],
                                      img[i * channels + 1],
                                      img[i * channels + 2],
                                      1.0f);
            }
            break;
        }

        default:
            LOG_ERROR("Invalid image channel count : %d", channels);
            return ImagePtr();
    }

    stbi_image_free(img);

    return ImagePtr(image);
}
