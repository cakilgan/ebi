#pragma once

#include <cstring>
#include <fstream>
#include <vector>
#define EBI_VERSION 1
#define EBI_VERSION_STR "1"

#define EBI_ORIGIN_TOP_LEFT    0
#define EBI_ORIGIN_BOTTOM_LEFT 1

#define EBI_FORMAT_RGB  0
#define EBI_FORMAT_ARGB 1
#define EBI_FORMAT_RGBA 2


namespace ebi{
    using u8  = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;

#pragma pack(push,1)
    struct header
    {
        char magic[4];
        u8 origin;
        u16 width;
        u16 height;
        u8 channels;
        u8 format;
        u32 data_size;
        u16 flags;
        u8 reserved[8];
    };
#pragma pack(pop)
    static_assert(sizeof(header) == 25);

    struct file{
        ::ebi::header header;
        u8* data;
    };

    enum error{
        ok = 1,general = -1,file_not_found = -33
    };

    inline error write(
        const char* file_path,
        file& use
    )
    {
        header& hdr = use.header;
        u8* data = use.data;

        hdr.magic[0] = 'E';
        hdr.magic[1] = 'B';
        hdr.magic[2] = 'I';
        hdr.magic[3] = EBI_VERSION;

        std::ofstream stream(file_path, std::ios::binary);
        if (!stream)
            return file_not_found;


        stream.write(
            reinterpret_cast<char*>(&hdr),
            sizeof(header)
        );

        stream.write(
            reinterpret_cast<char*>(data),
            hdr.data_size
        );

        return ok;
    }


    inline error read(
        const char* file_path,
        file& result
    )
    {
        std::ifstream stream(file_path, std::ios::binary);
        if (!stream)
            return file_not_found;

        header hdr;

        stream.read(
            reinterpret_cast<char*>(&hdr),
            sizeof(header)
        );

        if (!stream)
            return general;

        if (hdr.magic[0] != 'E' ||
            hdr.magic[1] != 'B' ||
            hdr.magic[2] != 'I' ||
            hdr.magic[3] != EBI_VERSION)
        {
            return general;
        }

        if (hdr.data_size == 0)
            return general;

        result.header = hdr;

        result.data = new u8[hdr.data_size];

        stream.read(
            reinterpret_cast<char*>(result.data),
            hdr.data_size
        );

        if (!stream)
            return general;

        return ok;
    }


    inline void set_pixel(u8* data, int width, int x, int y, int channels, u8 r, u8 g, u8 b, u8 a=255)
    {
        int idx = (y*width + x)*channels;
        data[idx+0] = r;
        data[idx+1] = g;
        data[idx+2] = b;
        if(channels==4) data[idx+3] = a;
    }
    
    inline void fill_color(u8* data,int width,int height,int channels,u8 r,u8 g,u8 b,u8 a=255){
        int cur = 0;
        if(channels == 3){
            for (int idx = 0; idx<width*height; idx++) {
                data[cur++] = r;
                data[cur++] = g;
                data[cur++] = b; 
            }
        }
        else if(channels == 4){
            for (int idx = 0; idx<width*height; idx++) {
                data[cur++] = r;
                data[cur++] = g;
                data[cur++] = b; 
                data[cur++] = a;
            }
        }
    }

    inline void vertical_flip(
        unsigned char* data,
        int width,
        int height,
        int channels
    )
    {
        int row_size = width * channels;
        std::vector<unsigned char> temp(row_size);

        for (int y = 0; y < height / 2; y++)
        {
            unsigned char* row_top =
                data + y * row_size;

            unsigned char* row_bottom =
                data + (height - 1 - y) * row_size;

            memcpy(temp.data(), row_top, row_size);
            memcpy(row_top, row_bottom, row_size);
            memcpy(row_bottom, temp.data(), row_size);
        }
    }


    inline void to_ppm(const ebi::file& img, const char* out_path)
    {
        if(img.header.channels > 3) 
            printf("%s[WARNING]:%s %s\n","\e[0;33m","\e[0m",".ppm files only support RGB format, ignoring other channels.");
        
        std::ofstream out(out_path, std::ios::binary);

        out << "P6\n";
        out << img.header.width << " " << img.header.height << "\n";
        out << "255\n";

        int w = img.header.width;
        int h = img.header.height;
        int c = img.header.channels;

        const unsigned char* data = img.data;

        if (img.header.format == EBI_FORMAT_RGB && c >= 3)
        {
            for (int i = 0; i < w * h; i++)
            {
                const unsigned char* px = &data[i * c];
                unsigned char r,g,b;

                r = px[0];
                g = px[1];
                b = px[2];
                

                out.write((char*)&r, 1);
                out.write((char*)&g, 1);
                out.write((char*)&b, 1);
            }
        }
        else if (img.header.format == EBI_FORMAT_RGBA && c == 4)
        {
            for (int i = 0; i < w * h; i++)
            {
                const unsigned char* px = &data[i * c];
                unsigned char r,g,b;

                r = px[0];
                g = px[1];
                b = px[2];
                

                out.write((char*)&r, 1);
                out.write((char*)&g, 1);
                out.write((char*)&b, 1);
            }
        }
        else if (img.header.format == EBI_FORMAT_ARGB && c == 4)
        {
            for (int i = 0; i < w * h; i++)
            {
                const unsigned char* px = &data[i * c];
                unsigned char r,g,b;

                r = px[1];
                g = px[2];
                b = px[3];
                

                out.write((char*)&r, 1);
                out.write((char*)&g, 1);
                out.write((char*)&b, 1);
            }
        }
    }
}