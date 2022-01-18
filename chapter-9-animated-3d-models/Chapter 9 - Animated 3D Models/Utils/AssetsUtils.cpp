#include "pch.h"
#include "AssetsUtils.h"

std::wstring assetTexturePath(std::string path, std::string relativeTexturePath, std::string folderSeparator) {
	size_t folderPos = path.find_last_of(folderSeparator);
	std::string folder = path.substr(0, folderPos + 1);
	std::string texturePath = folder + relativeTexturePath;
	size_t extPos = texturePath.find_last_of(".");
	texturePath.replace(extPos, texturePath.length(), ".dds");
	return std::wstring(texturePath.begin(), texturePath.end());
}