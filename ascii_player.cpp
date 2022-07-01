#include "ascii_player.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <fmt/color.h>

const std::vector<std::string> densities {"@", "#", "S", "%", "?", "*", "+", ";", ":", ",", "."};
const std::vector<std::string> densities_color {"@", "#", "$", "%"};

const void AsciiPlayer::stream(std::function<void(const std::string& frame)> callback, const std::string& path, const unsigned int width, const unsigned int height) const {
  for_each_frame(path, [&] (cv::Mat& frame) {
    cv::resize(frame, frame, cv::Size(width / 2, height));
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    callback(frame_to_ascii(frame));
  });
}

const void AsciiPlayer::stream_color(std::function<void(const std::string& frame)> callback, const std::string& path, const unsigned int width, const unsigned int height) const {
  for_each_frame(path, [&] (cv::Mat& frame) {
    cv::resize(frame, frame, cv::Size(width / 2, height));
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    callback(frame_to_ascii_color(frame));
  });
}

const std::string AsciiPlayer::frame_to_ascii(const cv::Mat& frame) const {
  std::string ascii;
  for_each_pixel<const unsigned char>(frame, [&] (const unsigned char& value, const unsigned int column, const unsigned int row) {
    if (column == frame.cols - 1) ascii += "\r\n";
    const std::string density = calculate_pixel_density(densities, value);
    ascii += density + density;
  });
  return ascii;
}

const std::string AsciiPlayer::frame_to_ascii_color(const cv::Mat& frame) const {
  std::string ascii;
  for_each_pixel<const cv::Vec3b>(frame, [&] (const cv::Vec3b& value, const unsigned int column, const unsigned int row) {
    if (column == frame.cols - 1) ascii += "\r\n";
    const float brightness = (value[0] * 0.3 + value[1] * 0.6 + value[2] * 0.1);
    const std::string density = calculate_pixel_density(densities_color, brightness);
    ascii += fmt::format(fmt::fg(fmt::rgb(value[0], value[1], value[2])), "{}", density + density);
  });
  return ascii;
}

const void AsciiPlayer::for_each_frame(const std::string& path, const std::function<void(cv::Mat& frame)> callback) const {
  cv::VideoCapture capture(path);
  for (unsigned int frameCount = 0; frameCount < capture.get(cv::CAP_PROP_FRAME_COUNT); frameCount++) {
    cv::Mat frame;
    capture >> frame;
    callback(frame);
  }
}

template<typename T>
const void AsciiPlayer::for_each_pixel(const cv::Mat& frame, const std::function<void(const T& value, const unsigned int column, const unsigned int row)> callback) const {
  for (unsigned int row = 0; row < frame.rows; row++)  {
    for (unsigned int column = 0; column < frame.cols; column++) {
      callback(frame.at<T>(row, column), column, row);
    }
  }
}

const std::string AsciiPlayer::calculate_pixel_density(const std::vector<std::string>& densities, const unsigned int value) const {
  const float increment = 255.f / densities.size();
  for (float i = 0, j = densities.size() - 1; i <= 255.f; i += increment, j--) {
    if (value >= i && value <= i + increment) {
      return densities[j];
    }
  }
  return densities.back();
}
