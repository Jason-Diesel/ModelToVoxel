#pragma once
#include "CommonHeaders.h"
#include <string>
#include <iomanip>
#include <cstdint>

static std::wstring CreateStringandIndexToWString(std::string str, int index) {
	std::string t = str + std::to_string(index);
	std::wstring temp = std::wstring(t.begin(), t.end());
	return temp;
}

static std::wstring createStringToWString(std::string str)
{
	return std::wstring(str.begin(), str.end());
}

namespace HF {
	template <typename T>
	void release(T*& resource)
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	void xRotation(DirectX::XMMATRIX& matrix, const float& rotation);
	void yRotation(DirectX::XMMATRIX& matrix, const float& rotation);
	void zRotation(DirectX::XMMATRIX& matrix, const float& rotation);
	void rotationMatrix(DirectX::XMMATRIX& matrix, float xRotation, float yRotation, float zRotation);

	DirectX::XMFLOAT3 add(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);
	float magDistance(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);
	float distance(const DirectX::XMINT3& a, const DirectX::XMINT3& b);
	float distance(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);

	std::wstring getCurrentDirectory();

	void dumpHexData(const void* ptr, size_t size);
}