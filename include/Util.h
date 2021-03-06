#pragma once
#include <string>
#include <random>
#include <opencv2/core.hpp>

namespace ark {
    struct CameraIntrin;
    namespace util {
        /**
        * Splits a string into components based on a delimiter
        * @param string_in string to split
        * @param delimiters c_str of delimiters to split at
        * @param ignore_empty if true, ignores empty strings
        * @param trim if true, trims whitespaces from each string after splitting
        * @return vector of string components
        */
        std::vector<std::string> split(const std::string & string_in,
            char const * delimiters = " ", bool ignore_empty = false, bool trim = false);

        /**
        * Splits a string into components based on a delimiter
        * @param string_in string to split
        * @param delimiters c_str of delimiters to split at
        * @param ignore_empty if true, ignores empty strings
        * @param trim if true, trims whitespaces from each string after splitting
        * @return vector of string components
        */
        std::vector<std::string> split(const char * string_in, char const * delimiters = " ",
            bool ignore_empty = false, bool trim = false);

        /** Trims whitespaces (space, newline, etc.) in-place from the left end of the string */
        void ltrim(std::string & s);

        /** Trims whitespaces (space, newline, etc.) in-place from the right end of the string */
        void rtrim(std::string & s);

        /** Trims whitespaces (space, newline, etc.) in-place from both ends of the string */
        void trim(std::string & s);

        /** Convert a string to upper case in-place */
        void upper(std::string & s);

        /** Convert a string to lower case in-place */
        void lower(std::string & s);

        std::string resolveRootPath(const std::string & root_path);

        /**
        * Get the color at index 'index' of the built-in palette
        * Used to map integers to colors.
        * @param color_index index of color
        * @param bgr if true, color is returned in BGR order instead of RGB (default true)
        * @return color in Vec3b format
        */
        cv::Vec3b paletteColor(int color_index, bool bgr = true);

        template<class T>
        /** Write binary to ostream */
        inline void write_bin(std::ostream& os, T val) {
            os.write(reinterpret_cast<char*>(&val), sizeof(T));
        }

        template<class T>
        /** Read binary from istream */
        inline void read_bin(std::istream& is, T& val) {
            is.read(reinterpret_cast<char*>(&val), sizeof(T));
        }

        /** Estimate pinhole camera intrinsics from xyz_map (by solving OLS)
         *  @return (fx, cx, fy, cy)
         */
        cv::Vec4d getCameraIntrinFromXYZ(const cv::Mat & xyz_map);

        /** Read a '.depth' raw depth map file into an OpenCV Mat
         *  @param allow_exr if true, checks if the file format is exr,
         *                   in which case does cv::imread instead
         *  */
        void readDepth(const std::string & path, cv::Mat & m,
                bool allow_exr = true);

        /** Read a '.depth' raw depth map file into an OpenCV Mat as XYZ map;
         *  if image already has 3 channels then reads directly
         *  @param allow_exr if true, checks if the file format is exr,
         *                   in which case does cv::imread instead
         *  */
        void readXYZ(const std::string & path, cv::Mat & m,
                const CameraIntrin& intrin,
                bool allow_exr = true);

        /** Write a .depth raw depth map file from an OpenCV Mat */
        void writeDepth(const std::string & image_path, cv::Mat & depth_map);
    } // util

    // Randomization utilities
    namespace random_util {
        template<class T>
        /** xorshift-based PRNG */
        inline T randint(T lo, T hi) {
            if (hi <= lo) return lo;
            static thread_local unsigned long x = std::random_device{}(), y = std::random_device{}(), z = std::random_device{}();
            thread_local unsigned long t;
            x ^= x << 16;
            x ^= x >> 5;
            x ^= x << 1;
            t = x;
            x = y;
            y = z;
            z = t ^ x ^ y;
            return z % (hi - lo + 1) + lo;
        }

        template<class T, class A>
        /** Choose k elements from a vector */
        std::vector<T, A> choose(std::vector<T, A> & source, size_t k) {
            std::vector<T, A> out;
            for (size_t j = 0; j < std::min(k, source.size()); ++j) {
                int r = randint(j, source.size() - 1); 
                out.push_back(source[r]);
                std::swap(source[j], source[r]);
            }   
            return out;
        }

        template<class T, class A>
        /** Choose k elements from an interval (inclusive on left, exclusive right) of a vector */
        std::vector<T, A> choose(std::vector<T, A> & source, size_t l, size_t r, size_t k) {
            std::vector<T, A> out;
            for (size_t j = l; j < std::min(l+k, r); ++j) {
                int ran = randint(j, r-1); 
                out.push_back(source[ran]);
                std::swap(source[j], source[ran]);
            }   
            return out;
        }   

        /** Uniform distribution */
        float uniform(float min_inc = 0., float max_exc = 1.);

        /** Gaussian distribution */
        float randn(float mean = 0, float variance = 1); 

        /** Uniform distribution with provided rng */
        float uniform(std::mt19937& rg, float min_inc = 0., float max_exc = 1.);

        /** Gaussian distribution with provided rng */
        float randn(std::mt19937& rg, float mean = 0, float variance = 1);
    } // random_util
}
