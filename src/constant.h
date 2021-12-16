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
	struct Vector2D
	{
		float x = 0.0f;
		float y = 0.0f;

		Vector2D() {}

		Vector2D(const float& x, const float& y) : x(x), y(y) {}

		Vector2D(const Vector2D& vec) : x(vec.x), y(vec.y) {}

		inline void operator=(const Vector2D &rhs)
		{
			this->x = rhs.x;
			this->y = rhs.y;
		}

		inline Vector2D operator+(const Vector2D &rhs) const
		{
			Vector2D vec;
			vec.x = this->x + rhs.x;
			vec.y = this->y + rhs.y;

			return vec;
		}

		inline Vector2D& operator+=(const Vector2D &rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;

			return *this;
		}

		inline bool operator==(const Vector2D &rhs) const
		{
			return (this->x == rhs.x) && (this->y == rhs.y);
		}
	};

    //Vector struct
	struct Vector3D
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		Vector3D() {}

		Vector3D(const float& x, const float& y, const float& z) : x(x), y(y), z(z) {}

		Vector3D(const Vector2D& vec2, const float& z) : x(vec2.x), y(vec2.y), z(z) {}

		Vector3D(const float& x, const Vector2D& vec2) : x(x), y(vec2.x), z(vec2.y) {}

		Vector3D(const Vector3D& vec) : x(vec.x), y(vec.y), z(vec.z) {}

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

		inline bool operator==(const Vector3D &rhs) const
		{
			return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
		}
	};

	#pragma pack(pop)

	typedef constant::Vector2D Centroid;

	struct TriangleIndices
	{
		int indice1, indice2, indice3;

		TriangleIndices(const int& i1, const int& i2, const int& i3) : indice1(i1), indice2(i2), indice3(i3) { }
		TriangleIndices(const TriangleIndices& rhs) : indice1(rhs.indice1), indice2(rhs.indice2), indice3(rhs.indice3) { }
	};

	struct Geometry2D
	{
		std::vector<constant::Vector2D> points;
		std::vector<TriangleIndices> trianglesIndices;
		constant::Vector2D offset = {0.0f, 0.0f}; 
	};

	struct Collidable2D
	{
		Geometry2D geometry;

		bool collideWith(const Collidable2D* obj) const { if(collideFunc(obj)) return true; if(obj->collideFunc(this)) return true; return false; }
		std::function<bool(const Collidable2D*)> collideFunc;
	};

	struct Triangle2D : public Collidable2D
	{
		Triangle2D(const constant::Vector2D& p1, const constant::Vector2D& p2, const constant::Vector2D& p3, const constant::Vector2D& offset = {0.0f, 0.0f})
		{
			geometry.offset = offset;
			geometry.points.push_back(p1);
			geometry.points.push_back(p2);
			geometry.points.push_back(p3);

			geometry.trianglesIndices.push_back(TriangleIndices(0, 1, 2));

			collideFunc = [=](const Collidable2D* obj)
			{ 
				const auto point1 = geometry.points[0];
				const auto point2 = geometry.points[1];
				const auto point3 = geometry.points[2];
				//abs(x1*y2+x2*y3+x3*y1-x1*y3-x3*y2-x2*y1)/2

				const double triangleArea = getTriangleArea(point1, point2, point3);
				//std::cout << "TrianglePoint : " << point1.x << "," << point1.y << " " << point2.x << "," << point2.y << " " << point3.x << "," << point3.y << std::endl;
				//std::cout << triangleArea << std::endl;

				for(const auto& point : obj->geometry.points)
				{
					//std::cout << "Collision Check" << std::endl;
					const double Area1 = getTriangleArea(point, point1, point2);
					const double Area2 = getTriangleArea(point, point2, point3);
					const double Area3 = getTriangleArea(point, point3, point1);

					if(triangleArea == Area1 + Area2 + Area3)
					{
						return true;
					}
				}

				return false;
			};
		}

		static double getTriangleArea(const constant::Vector2D& point1, const constant::Vector2D& point2, const constant::Vector2D& point3)
		{
			return std::abs(point1.x * point2.y + point2.x * point3.y + point3.x * point1.y
							-point1.x * point3.y - point3.x * point2.y - point2.x * point1.y) / 2.0f;
		}

		static Centroid getCentroid(const constant::Vector2D& point1, const constant::Vector2D& point2, const constant::Vector2D& point3)
		{
			return Centroid((point1.x + point2.x + point3.x) / 3.0f, (point1.y + point2.y + point3.y) / 3.0f);
		}

		Triangle2D(const Triangle2D& obj) : Triangle2D(obj.geometry.points[0], obj.geometry.points[1], obj.geometry.points[2]) { }
	};

	struct Rectangle2D : public Collidable2D
	{
		constant::Vector2D pos;
		constant::Vector2D scale;

		Rectangle2D(const float& x, const float& y, const float& w, const float& h, const constant::Vector2D& offset = {0.0f, 0.0f})
		{
			geometry.offset = offset;
			pos = constant::Vector2D(x, y);
			scale = constant::Vector2D(w, h);

			geometry.points.push_back(constant::Vector2D(x, y));
			geometry.points.push_back(constant::Vector2D(x + w, y));
			geometry.points.push_back(constant::Vector2D(x, y + h));
			geometry.points.push_back(constant::Vector2D(x + w, y + h));

			geometry.trianglesIndices.push_back(TriangleIndices(0, 1, 2));
			geometry.trianglesIndices.push_back(TriangleIndices(1, 2, 3));

			collideFunc = [=](const Collidable2D *obj)
			{
				for(auto point : obj->geometry.points)
				{
					//std::cout << "Collision Check" << std::endl;
					if(point.x >= pos.x && point.x <= pos.x + scale.x && point.y >= pos.y && point.y <= pos.y + scale.y)
					{
						return true;
					}
				}

				return false;
			};
		}
	};

	struct AbstractShape2D : public Collidable2D
	{
		std::vector<Triangle2D> triangleList;

		AbstractShape2D(const Geometry2D& geometry)
		{
			this->geometry = geometry;

			for(auto triangle : geometry.trianglesIndices)
				triangleList.push_back(Triangle2D(geometry.points[triangle.indice1], geometry.points[triangle.indice2], geometry.points[triangle.indice3]));

			collideFunc = [=](const Collidable2D* obj){
				for(const auto& triangle : triangleList)
					if(triangle.collideWith(obj))
						return true;

				return false;
			};
		}

		//~AbstractShape2D() { for(auto triangle : triangleList) delete triangle; }
	};

	struct ModelInfo
	{
		float *vertices = nullptr;
		unsigned int *indices = nullptr;

		unsigned int nbVertices = 0;
		unsigned int nbIndices = 0;

		//TODO check if model info should be the one deleting vertices and indices by default
		virtual ~ModelInfo() { delete vertices; delete indices; }
	};

	struct GeometryVertices : public ModelInfo
	{
		GeometryVertices(Geometry2D geometry, const constant::Vector2D& offset = {0,0})
		{
			geometry.offset = offset;
			nbVertices = geometry.points.size() * 5;
			nbIndices = geometry.trianglesIndices.size() * 3;

			vertices = new float[nbVertices];

			int i = 0;

			//TODO create a note about how the grid is made x is positive to the right and y is positive to the bottom
			// and because of that when sending vertices here i take the y value and negate it so all the objects
			//have there origin in the top left corner with x pointing to the right and y pointing to the bottom.
			for(auto point : geometry.points)
			{
				vertices[i + 0] = point.x + geometry.offset.x; vertices[i + 1] = -point.y - geometry.offset.y; vertices[i + 2] =  0.0f; vertices[i + 3] = point.x; vertices[i + 4] = -point.y;   
				i += 5;
			}

			indices = new unsigned int[nbIndices];
			
			i = 0;
			for(auto triangle : geometry.trianglesIndices)
			{
				indices[i + 0] = triangle.indice1; indices[i + 1] = triangle.indice2; indices[i + 2] = triangle.indice3;
				i += 3;
			}
		}

		template <typename Shape2D>
		GeometryVertices(const Shape2D& shape, constant::Vector2D offset = {0.0f, 0.0f}) : GeometryVertices(shape.geometry, offset) { }
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
