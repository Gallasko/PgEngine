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

    //Vector struct
	struct Vector3D
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

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
}

#endif
