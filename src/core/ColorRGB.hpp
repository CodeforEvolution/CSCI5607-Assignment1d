// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef COLOR_RGB_H
#define COLOR_RGB_H

#include <cmath>
#include <cstdint>
#include <string>

struct ColorRGB {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

public:
	enum struct Component {
		RED,
		GREEN,
		BLUE
	};

	ColorRGB()
			:
			red(0),
			green(0),
			blue(0)
	{
	}

	// Description: Works on integer scale of 0 to 255
	explicit
	ColorRGB(uint8_t red, uint8_t green, uint8_t blue)
	{
		this->red = red;
		this->green = green;
		this->blue = blue;
	}

	// Description: Works on decimal scale of 0.0 to 1.0
	explicit
	ColorRGB(float red, float green, float blue)
	{
		this->red = static_cast<uint8_t>(std::clamp(roundf(red * 255.f), 0.f, 255.f));
		this->green = static_cast<uint8_t>(std::clamp(roundf(green * 255.f), 0.f, 255.f));
		this->blue = static_cast<uint8_t>(std::clamp(roundf(blue * 255.f), 0.f, 255.f));
	}

	[[nodiscard]] float
	ToFloat(Component colorComponent) const
	{
		switch (colorComponent)
		{
			case Component::RED:
				return kUInt8ToFloat * static_cast<float>(red);
			case Component::GREEN:
				return kUInt8ToFloat * static_cast<float>(green);
			case Component::BLUE:
				return kUInt8ToFloat * static_cast<float>(blue);
		}

		// It "shouldn't" be possible to get here!
		__builtin_unreachable();
	}

	[[nodiscard]] std::tuple<float, float, float>
	ToFloat() const
	{
		return {kUInt8ToFloat * static_cast<float>(red), kUInt8ToFloat * static_cast<float>(green), kUInt8ToFloat * static_cast<float>(blue)};
	}

	constexpr ColorRGB&
	operator+=(const ColorRGB& other)
	{
		const auto clampColor = [](const uint32_t& value, const uint8_t& min, const uint8_t& max) -> uint8_t {
			if (value > max)
				return max;
			else if (value < min)
				return min;
			else
				return value;
		};

		red = clampColor(red + other.red, 0, 255);
		green = clampColor(green + other.green, 0, 255);
		blue = clampColor(blue + other.blue, 0, 255);

		return *this;
	}

	auto operator<=>(const ColorRGB& other) const = default;

private:
	static constexpr float kUInt8ToFloat = 1.f / 255.f;
};

static std::ostream&
operator<<(std::ostream& out, const ColorRGB& color)
{
	out << "(red: " << std::to_string(color.red) << ", green: " << std::to_string(color.green) << ", blue: " << std::to_string(color.blue) << ")";
	return out;
}

#endif // COLOR_RGB_H
