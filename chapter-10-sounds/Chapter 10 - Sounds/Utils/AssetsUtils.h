#pragma once

std::wstring assetTexturePath(std::string path, std::string relativeTexturePath, std::string folderSeparator="/");

auto dumpMaterial = [](auto material)-> void {
	std::string textureMapModes[] = { "Wrap","Clamp","Mirror","Decal" };
	std::map<int, std::string> magModes = {
		{0, "UNSET"},
		{9728, "SamplerMagFilter_Nearest"} , {9729, "SamplerMagFilter_Linear"}
	};
	std::map<int, std::string> minModes = {
		{0, "UNSET"},
		{9728,"SamplerMinFilter_Nearest"},
		{9729,"SamplerMinFilter_Linear"},
		{9984,"SamplerMinFilter_Nearest_Mipmap_Nearest"},
		{9985,"SamplerMinFilter_Linear_Mipmap_Nearest"},
		{9986,"SamplerMinFilter_Nearest_Mipmap_Linear"},
		{9987,"SamplerMinFilter_Linear_Mipmap_Linear"}
	};

	OutputDebugStringA(material->GetName().C_Str());
	OutputDebugStringA("\n");
	for (UINT propertyIndex = 0U; propertyIndex < material->mNumProperties; propertyIndex++) {
		auto p = material->mProperties[propertyIndex];
		OutputDebugStringA(p->mKey.C_Str());
		OutputDebugStringA("[");
		OutputDebugStringA(std::to_string(p->mType).c_str());
		OutputDebugStringA("]:");
		switch (p->mType) {
			case aiPTI_Float: {
				for (ai_real* value = (ai_real*)p->mData; value != (ai_real*)(p->mData + p->mDataLength); value++) {
					OutputDebugStringA(std::to_string(*value).c_str());
					if (value != (ai_real*)(p->mData + p->mDataLength) - 1) {
						OutputDebugStringA(",");
					}
				}
			}
			break;
			case aiPTI_Double: {
				for (ai_real* value = (ai_real*)p->mData; value != (ai_real*)(p->mData + p->mDataLength); value++) {
					OutputDebugStringA(std::to_string(*value).c_str());
					if (value != (ai_real*)(p->mData + p->mDataLength) - 1) {
						OutputDebugStringA(",");
					}
				}
			}
			break;
			case aiPTI_String: {
				aiString* value = (aiString*)p->mData;
				OutputDebugStringA(value->C_Str());
			}
			break;
			case aiPTI_Integer: {
				for (ai_int* value = (ai_int*)p->mData; value != (ai_int*)(p->mData + p->mDataLength); value++) {
					OutputDebugStringA(std::to_string(*value).c_str());
					if (value != (ai_int*)(p->mData + p->mDataLength) - 1) {
						OutputDebugStringA(",");
					}
				}
			}
			break;
			case aiPTI_Buffer: {
				if (!strcmp(p->mKey.C_Str(), "$tex.file.texCoord")) {
					aiUVTransform* value = (aiUVTransform*)p->mData;
					OutputDebugStringA("T(");
					OutputDebugStringA((std::to_string(value->mTranslation.x) + "," + std::to_string(value->mTranslation.y)).c_str());
					OutputDebugStringA(") R(");
					OutputDebugStringA((std::to_string(value->mRotation)).c_str());
					OutputDebugStringA(") S(");
					OutputDebugStringA((std::to_string(value->mTranslation.x) + "," + std::to_string(value->mTranslation.y)).c_str());
					OutputDebugStringA(")");
				}
				else if (!strcmp(p->mKey.C_Str(), "$tex.mapmodeu") || !strcmp(p->mKey.C_Str(), "$tex.mapmodev")) {
					ai_int* value = (ai_int*)p->mData;
					OutputDebugStringA(textureMapModes[*value].c_str());
				}
				else if (!strcmp(p->mKey.C_Str(), "$mat.twosided")) {
					bool* value = (bool*)p->mData;
					OutputDebugStringA(*value ? "TRUE" : "FALSE");
				}
				else if (!strcmp(p->mKey.C_Str(), "$tex.mappingfiltermag")) {
					ai_int* value = (ai_int*)p->mData;
					OutputDebugStringA(magModes[*value].c_str());
				}
				else if (!strcmp(p->mKey.C_Str(), "$tex.mappingfiltermin")) {
					ai_int* value = (ai_int*)p->mData;
					OutputDebugStringA(minModes[*value].c_str());
				}
				else if (!strcmp(p->mKey.C_Str(), "$mat.gltf.pbrSpecularGlossiness")) {
					bool* value = (bool*)p->mData;
					OutputDebugStringA(*value ? "TRUE" : "FALSE");
				}
				else {
					aiString* value = (aiString*)p->mData;
					OutputDebugStringA(value->C_Str());
				}
			}
			break;
		}
		OutputDebugStringA("\n");
	}
};