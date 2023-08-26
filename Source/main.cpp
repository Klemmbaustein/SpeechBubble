#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#include "stb/stb_image.hpp"
#include "stb/stb_image_write.hpp"
#include <iostream>
int imageX, imageY, ch;

struct Point
{
	float X = 0; float Y = 0;
};

const Point SpeechBubbleTriange[3] =
{
	{0.1f, 0},
	{0.3f, 0},
	{0.4f, 0.2f}
};

bool PointInTriangle(Point p, Point p0, Point p1, Point p2)
{
	auto s = (p0.X - p2.X) * (p.Y - p2.Y) - (p0.Y - p2.Y) * (p.X - p2.X);
	auto t = (p1.X - p0.X) * (p.Y - p0.Y) - (p1.Y - p0.Y) * (p.X - p0.X);

	if ((s < 0) != (t < 0) && s != 0 && t != 0)
		return false;

	auto d = (p2.X - p1.X) * (p.Y - p1.Y) - (p2.Y - p1.Y) * (p.X - p1.X);
	return d == 0 || (d < 0) == (s + t <= 0);
}

const int aaSamples = 3;

void HandlePixel(float r, float g, float b, float x, float y, float& outr, float& outg, float& outb, float& outalpha)
{
	const float speechBubbleX = (0.5f - x) / 3, speechBubbleY = 0.2f + y;

	const float speechBubbleDistance = sqrt(speechBubbleX * speechBubbleX + speechBubbleY * speechBubbleY);


	float triangleSamples = 0;
	const float pixelSizeX = 1.0f / imageX;
	const float pixelSizeY = 1.0f / imageY;

	for (int sampleX = 0; sampleX < aaSamples; sampleX++)
	{
		for (int sampleY = 0; sampleY < aaSamples; sampleY++)
		{
			Point currentPoint;
			currentPoint.X = x + (pixelSizeX * sampleX);
			currentPoint.Y = y + (pixelSizeY * sampleY);
			triangleSamples += PointInTriangle(currentPoint, SpeechBubbleTriange[0], SpeechBubbleTriange[1], SpeechBubbleTriange[2]) 
				? 0.0f
				: 1.0f / (float)(aaSamples * aaSamples);
		}
	}
	if (speechBubbleDistance < 0.25f)
	{
		outalpha = 0;
	}
	else
	{
		outalpha = std::min(std::min((speechBubbleDistance - 0.25f) * 200, 1.0f) * pow(triangleSamples, 3.0f), 1.0f);
	}
	outr = r;
	outg = g;
	outb = b;
}

void IncorrectUsage(const char* ExecName)
{
	std::cout << "Incorrect usage. Usage:" << std::endl;
	std::cout << ExecName << " [Input File] -o [Output File (must be .png)]" << std::endl;
	std::cout << "    Loads [Input File], adds a speech bubble, saves the new image as [Output File]" << std::endl << std::endl;
	std::cout << ExecName << " [Input File]" << std::endl;
	std::cout << "    Loads [Input File], adds a speech bubble, saves the new image as [Input File]_SpeechBubble.png" << std::endl << std::endl;
	exit(1);
}

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		IncorrectUsage(argv[0]);
	}

	std::string targetPicture = argv[1];

	std::string outPicture;
	if (argc == 4 && argv[2] == std::string("-o"))
	{
		outPicture = argv[3];
		if (outPicture.substr(outPicture.find_last_of(".") + 1) != "png")
		{
			outPicture.append(".png");
		}
	}
	else if (argc != 2)
	{
		IncorrectUsage(argv[0]);
	}
	else
	{
		outPicture = targetPicture.substr(0, targetPicture.find_last_of("."));
		outPicture.append("_SpeechBubble.png");
	}

	const stbi_uc* image = stbi_load(targetPicture.c_str(), &imageX, &imageY, &ch, 0);


	if (image == nullptr)
	{
		std::cout << "load fail: " << stbi_failure_reason() << " image: " << targetPicture << std::endl;
		return 1;
	}
	std::cout << "Loaded " << targetPicture << " with resolution " << imageX << "x" << imageY << " channels: " << ch << std::endl;

	uint8_t* outImage = new uint8_t[imageY * imageX * 4]();

	for (int ity = 0; ity < imageY; ity++)
	{
		for (int itx = 0; itx < imageX; itx++)
		{
			float r = 0, g = 0, b = 0, a = 1;
			r = image[ity * imageX * ch + itx * ch + 0];
			g = image[ity * imageX * ch + itx * ch + 1];
			b = image[ity * imageX * ch + itx * ch + 2];
			if (ch == 4)
			{
				a = image[ity * imageX * ch + itx * ch + 3];
			}
			float outr = 0, outg = 0, outb = 0, outapha = 0;

			float posx = (float)itx / (float)imageX;
			float posy = (float)ity / (float)imageY;

			HandlePixel(r, g, b, posx, posy, outr, outg, outb, outapha);

			outImage[ity * imageX * 4 + itx * 4 + 0] = (uint8_t)outr;
			outImage[ity * imageX * 4 + itx * 4 + 1] = (uint8_t)outg;
			outImage[ity * imageX * 4 + itx * 4 + 2] = (uint8_t)outb;
			outImage[ity * imageX * 4 + itx * 4 + 3] = (uint8_t)(outapha * 255);
		}
	}

	if (!stbi_write_png(outPicture.c_str(), imageX, imageY, 4, outImage, imageX * 4))
	{
		std::cout << "write fail: " << stbi_failure_reason() << std::endl;
	}
	else
	{
		std::cout << "Written " << outPicture << std::endl;
	}
}