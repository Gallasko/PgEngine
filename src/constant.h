#ifndef CONSTANT_H
#define CONSTANT_H

namespace constant
{
	//Screen Const
	constexpr int SCREEN_WIDTH = 640;
	constexpr int SCREEN_HEIGHT = 480;

	//Camera Const
	constexpr float YAW         = -90.0f;
	constexpr float PITCH       =  0.0f;
	constexpr float SPEED       =  2.5f;
	constexpr float SENSITIVITY =  1.5f;
	constexpr float ZOOM        =  45.0f;

	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum class Camera_Movement 
	{
	    FORWARD,
	    BACKWARD,
	    LEFT,
	    RIGHT
	};

	//pragma pack data so it doesnt get padded
	#pragma pack(push, 0)

    //Vector struct
	struct Vector3D
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		Vector3D() {}

		Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}

		inline void operator=(const Vector3D &rhs)
		{
			this->x = rhs.x;
			this->y = rhs.y;
			this->z = rhs.z;
		}

		inline Vector3D operator+(const Vector3D &rhs) const
		{
			Vector3D vec;
			vec.x = this->x + rhs.x;
			vec.y = this->y + rhs.y;
			vec.z = this->z + rhs.z;

			return vec;
		}

		inline Vector3D operator-(const Vector3D &rhs) const
		{
			Vector3D vec;
			vec.x = this->x - rhs.x;
			vec.y = this->y - rhs.y;
			vec.z = this->z - rhs.z;

			return vec;
		}

		inline Vector3D& operator+=(const Vector3D &rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;
			this->z += rhs.z;

			return *this;
		}

		inline bool operator==(const Vector3D &rhs)
		{
			return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
		}
	};

	#pragma pack(pop)

	struct ModelInfo
	{
		float *vertices = nullptr;
		unsigned int *indices = nullptr;

		unsigned int nbVertices = 0;
		unsigned int nbIndices = 0;
	};

	struct SquareInfo : public ModelInfo
	{
		SquareInfo()
		{
			vertices = new float[20];
			//              x                     y                    z                    texpos x             texpos y
			vertices[0] =  -0.5f; vertices[1] =   0.5f; vertices[2] =  0.0f; vertices[3] =  0.0f; vertices[4] =  1.0f;   
			vertices[5] =   0.5f; vertices[6] =   0.5f; vertices[7] =  0.0f; vertices[8] =  1.0f; vertices[9] =  1.0f;
			vertices[10] = -0.5f; vertices[11] = -0.5f; vertices[12] = 0.0f; vertices[13] = 0.0f; vertices[14] = 0.0f;
			vertices[15] =  0.5f; vertices[16] = -0.5f; vertices[17] = 0.0f; vertices[18] = 1.0f; vertices[19] = 0.0f;

			indices = new unsigned int[6];
			indices[0] = 0; indices[1] = 1; indices[2] = 2;
			indices[3] = 1; indices[4] = 2; indices[5] = 3;

			nbVertices = 20;
			nbIndices = 6;
		}

		~SquareInfo() { delete vertices; delete indices; }
	};
}

#endif
