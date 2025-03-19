#include "HelperFuncitons.h"

void HF::yRotation(DirectX::XMMATRIX& matrix, const float& rotation)
{
	//X rotation
	DirectX::XMMATRIX Mrot(
		std::cos(rotation), 0.0f, std::sin(rotation), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-std::sin(rotation), 0.0f, std::cos(rotation), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	matrix = matrix * Mrot;
}

void HF::xRotation(DirectX::XMMATRIX& matrix, const float& rotation)
{
	DirectX::XMMATRIX Mrot(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, std::cos(rotation), std::sin(rotation), 0.0f,
		0.0f, -std::sin(rotation), std::cos(rotation), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	matrix = matrix * Mrot;
}

void HF::zRotation(DirectX::XMMATRIX& matrix, const float& rotation)
{
	DirectX::XMMATRIX Mrot(
		std::cos(rotation), -std::sin(rotation), 0.0f, 0.0f,
		std::sin(rotation), std::cos(rotation), 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	matrix = matrix * Mrot;
}

void HF::rotationMatrix(DirectX::XMMATRIX& matrix, float xRotation, float yRotation, float zRotation)
{
	HF::yRotation(matrix, xRotation);
	HF::xRotation(matrix, yRotation);
	//HF::zRotation(matrix, zRotation);//Just gonna skip this for now
}

DirectX::XMFLOAT3 HF::add(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
	return DirectX::XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

float HF::squareDistance(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	const float dz = a.z - b.z;

	return dx * dx + dy * dy + dz * dz;
}

float HF::distance(const DirectX::XMINT3& a, const DirectX::XMINT3& b)
{
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	const float dz = a.z - b.z;

	return sqrt(dx * dx + dy * dy + dz * dz);
}

std::wstring HF::getCurrentDirectory()
{
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

void HF::dumpHexData(const void* ptr, size_t size)
{
	const uint8_t* bytePtr = static_cast<const uint8_t*>(ptr); // Treat the data as bytes
	for (size_t i = 0; i < size; ++i) {
		// Print each byte in hex format, padded with zeros
		std::cout << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(bytePtr[i]) << " ";

		// Optionally format output into rows for readability
		if ((i + 1) % 16 == 0) {
			std::cout << "\n";
		}
	}
	std::cout << std::dec << std::endl; // Reset formatting to decimal
}

