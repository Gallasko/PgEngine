#ifndef CONSTANT_H
#define CONSTANT_H

//TODO have no include in constant

#include <unordered_map> // Included for RefractTable
#include <string> // Included for RefractTable and Big Int
#include <vector> // Included for Big Int

namespace pg
{
	namespace constant
	{
		//Screen Const
		constexpr int SCREEN_WIDTH = 624;
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
		struct Vector2D
		{
			float x = 0.0f;
			float y = 0.0f;

			Vector2D() {}

			Vector2D(const unsigned int& x, const unsigned int& y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}

			Vector2D(const int& x, const int& y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}

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

		//Vector struct
		struct Vector4D
		{
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float w = 0.0f;

			Vector4D() {}

			Vector4D(const float& x, const float& y, const float& z, const float& w) : x(x), y(y), z(z), w(w) {}

			Vector4D(const Vector3D& vec, const float& w) : x(vec.x), y(vec.y), z(vec.z), w(w)  {}

			Vector4D(const float& x, const Vector3D& vec) : x(x), y(vec.x), z(vec.y), w(vec.z)  {}

			Vector4D(const Vector4D& vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w)  {}

			inline void operator=(const Vector4D &rhs)
			{
				this->x = rhs.x;
				this->y = rhs.y;
				this->z = rhs.z;
				this->w = rhs.w;
			}

			inline Vector4D operator+(const Vector4D &rhs) const
			{
				Vector4D vec;
				vec.x = this->x + rhs.x;
				vec.y = this->y + rhs.y;
				vec.z = this->z + rhs.z;
				vec.w = this->w + rhs.w;

				return vec;
			}

			inline Vector4D& operator+=(const Vector4D &rhs)
			{
				this->x += rhs.x;
				this->y += rhs.y;
				this->z += rhs.z;
				this->w += rhs.w;

				return *this;
			}

			inline bool operator==(const Vector4D &rhs) const
			{
				return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z) && (this->w == rhs.w);
			}
		};

		class BigInt
		{
		public:
			BigInt() { digits.push_back(0); }
			BigInt(const BigInt& bigNum) : digits(bigNum.digits), sign(bigNum.sign) {}
			BigInt(const long long int& value) { *this = value; }
			BigInt(const std::string& value) { *this = value; }
			BigInt(const char* value) { *this = std::string(value); }

			void operator=(const BigInt& rhs)
			{
				digits = rhs.digits;
				sign = rhs.sign;
			}

			void operator=(long long int value) 
			{
				digits.clear();

				sign = value > 0;
				if(sign == 0)
					value = -value;

				for(; value > 0; value /= 1000)
					digits.push_back(value % 1000);
			}

			void operator=(const std::string& value)
			{
				digits.clear();
				sign = 1;

				if(value.size() < 1)
				{
					digits.push_back(0);
					return;
				}

				int pos = 0;
				if(value[0] == '-' || value[0] == '+')
				{
					pos++;

					if(value.size() < 2)
					{
						digits.push_back(0);
						return;
					}

					if(value[0] == '-')
						sign = 0;
				}

				for(int i = value.size() - 1; i >= pos; i -= 3)
				{
					int x = 0;
					for (int j = std::max(pos, i - 3 + 1); j <= i; j++)
						x = x * 10 + value[j] - '0';

					digits.push_back(x);
				}

				trim();
			}

			BigInt operator+(const BigInt& rhs) const
			{
				if(sign == rhs.sign)
				{
					BigInt result(rhs);
					unsigned short carry = 0;

					for (int i = 0; i < (int)std::max(digits.size(), rhs.digits.size()) || carry; i++)
					{
						if(i == (int)result.digits.size())
							result.digits.push_back(0);

						result.digits[i] += carry + (i < (int)digits.size() ? digits[i] : 0);
						
						carry = result.digits[i] >= 1000;
						if(carry)
							result.digits[i] -= 1000;
					}

					return result;
				}
				
				return *this - (-rhs);
			}

			BigInt operator-() const 
			{
				BigInt res = *this;
				res.sign = !sign;
				return res;
			}

			BigInt operator-(const BigInt &rhs) const 
			{
				if (sign == rhs.sign) 
				{
					if (abs() >= rhs.abs()) 
					{
						BigInt res = *this;

						for (int i = 0, carry = 0; i < (int) rhs.digits.size() || carry; i++) 
						{
							res.digits[i] -= carry + (i < (int) rhs.digits.size() ? rhs.digits[i] : 0);

							carry = res.digits[i] < 0;
							if (carry)
								res.digits[i] += 1000;
						}

						res.trim();
						return res;
					}
					return -(rhs - *this);
				}
				return *this + (-rhs);
			}

			void operator+=(const BigInt &rhs) { *this = *this + rhs; }
			void operator-=(const BigInt &rhs) { *this = *this - rhs; }

			bool operator< (const BigInt &rhs) const 
			{
				if (sign != rhs.sign)
					return sign < rhs.sign;

				if (digits.size() != rhs.digits.size())
					return digits.size() * sign < rhs.digits.size() * rhs.sign;

				for (int i = digits.size() - 1; i >= 0; i--)
					if (digits[i] != rhs.digits[i])
						return digits[i] * sign < rhs.digits[i] * sign;

				return false;
			}
		
			bool operator> (const BigInt &rhs) const { return rhs < *this; }
			bool operator<=(const BigInt &rhs) const { return !(rhs < *this); }
			bool operator>=(const BigInt &rhs) const { return !(*this < rhs); }
			bool operator==(const BigInt &rhs) const { return !(*this < rhs) && !(rhs < *this); }
			bool operator!=(const BigInt &rhs) const { return *this < rhs || rhs < *this; }

			std::string toString() const { std::string res = ""; if(sign == 0) res += "-"; auto it = digits.rbegin(); if(it != digits.rend()) res += std::to_string(*it++); while(it != digits.rend()) { res += numberWithLeadingZero(*it++); } return res; }

			friend std::ostream& operator<<(std::ostream& out, const BigInt& n) { return out << n.toString(); }

		private:
			std::vector<short> digits; // digit stored in base 1000 -> 1 slot == 3 digits
			bool sign = 1; // sign = 1 positiv integer, sign = 0 negativ integer 

			std::string numberWithLeadingZero(short n) const
			{ 
				std::string value = "";
				if(n < 100)
						value += "0";
				if(n < 10)
						value += "0";

				value += std::to_string(n);

				return value;
			}

			BigInt abs() const 
			{
				BigInt res = *this;
				res.sign = 1;
				return res;
			}

			void trim()
			{ 
				while (!digits.empty() && !digits.back())
					digits.pop_back();
				if (digits.empty())
					sign = 1;
			}

		};

		// TODO make a check on whether the numerical is empty or not
		struct Numerical
		{
			struct Op
			{
				enum class Operation
				{
					ADD,
					SUB,
					MUL,
					DIV
				};

				Op() {}

				virtual Numerical* operator()(const int&, const Operation&) const = 0;
				virtual Numerical* operator()(const float&, const Operation&) const = 0;

				virtual ~Op() {}
			};

			Op *op;
			bool empty;

			bool isEmpty() const { return empty; }

			virtual Numerical* operator+(Numerical *rhs) const = 0;
			virtual Numerical* operator-(Numerical *rhs) const = 0;
			virtual Numerical* operator*(Numerical *rhs) const = 0;
			virtual Numerical* operator/(Numerical *rhs) const = 0;
			
			template <typename T>
			Numerical* operator+(const T& value) const { return (*op)(value, Op::Operation::ADD); }
			template <typename T>
			Numerical* operator-(const T& value) const { return (*op)(value, Op::Operation::SUB); }
			template <typename T>
			Numerical* operator*(const T& value) const { return (*op)(value, Op::Operation::MUL); }
			template <typename T>
			Numerical* operator/(const T& value) const { return (*op)(value, Op::Operation::DIV); }

			virtual Numerical* clone() const = 0;

			virtual ~Numerical() { delete op; }
		};

		template <typename Type>
		struct Numerics : public Numerical
		{
			Type value;

			struct Op : public Numerical::Op
			{
				Type *value;

				Op() {}
				Op(Type *value) : value(value) {}

				virtual ~Op() {}

				virtual Numerical* operator()(const int&, const Operation&) const { return createEmpty(); }
				virtual Numerical* operator()(const float&, const Operation&) const { return createEmpty(); }

				Numerical* createEmpty() const { return new Numerics<Type> (); }

				template <typename NumericalType, typename ValueType>
				Numerical* createFunc(const ValueType& val, const Operation& operation) const {
					switch (operation)
					{
					case Operation::ADD:
						return new NumericalType(*value + val);
						break;
					case Operation::SUB:
						return new NumericalType(*value - val);
						break;
					case Operation::MUL: 
						return new NumericalType(*value * val); 
						break;
					case Operation::DIV:
						return new NumericalType(*value / val);
						break;
					default:
						return new NumericalType(*value);
					} }
			};

			Numerics() { empty = true; }
			Numerics(const Type& i) : value(i) { empty = false; }

			virtual Numerical* clone() const { return new Numerics<Type>(); }

			//Numerical* operator+(Numerical *rhs) { return *rhs + this->value; }
			Numerical* operator+(Numerical *rhs) const { return *rhs + this->value; }
			Numerical* operator-(Numerical *rhs) const { return *rhs - this->value; }
			Numerical* operator*(Numerical *rhs) const { return *rhs * this->value; }
			Numerical* operator/(Numerical *rhs) const { return *rhs / this->value; }
		};

		struct NumericalFloat : public Numerics<float>
		{
			struct Op : public Numerics<float>::Op
			{
				Op(float *value) : Numerics<float>::Op(value) {}
				~Op() {}

				Numerical* operator()(const int& val, const Operation& operation) const { return createFunc<NumericalFloat>(val, operation); }
				Numerical* operator()(const float& val, const Operation& operation) const { return createFunc<NumericalFloat>(val, operation); }
			};

			NumericalFloat(const float& i) : Numerics<float>(i) { op = new Op(&value); }
			Numerical* clone() const { return new NumericalFloat(value); }
		};

		struct NumericalInt : public Numerics<int>
		{
			struct Op : public Numerics<int>::Op
			{
				Op(int *value) : Numerics<int>::Op(value) {}
				~Op() {}

				Numerical* operator()(const int& val, const Operation& operation) const { return createFunc<NumericalInt>(val, operation); }
				Numerical* operator()(const float& val, const Operation& operation) const { return createFunc<NumericalFloat>(val, operation); }
			};

			NumericalInt(const int& i) : Numerics<int>(i) { op = new Op(&value); }
			Numerical* clone() const { return new NumericalInt(value); }

			//Numerical* operator()(Numerical* numerical, const BigInt &rhs, const Op::Operation& operation) const { return createFunc<NumericalInt>(rhs, numerical->op, operation); }
		};

		class RefracTable
		{
		public:
			struct Ref
			{
				Numerical *value;
				bool scoped;

				Ref() : value(nullptr), scoped(true) {}
				Ref(Numerical *value, bool scoped = false) : value(value), scoped(scoped) {}
				Ref(const Ref& ref) { value = ref.value; scoped = ref.scoped; } // TODO see if it scoped needs to be true or false
				~Ref() { if(scoped) delete value; }

				void operator=(const Ref& ref) { if(!scoped && ref.scoped) value = ref.value->clone(); else value = ref.value; } // TODO see if we need to deep copy when the value is not scoped
				void operator=(Numerical *value) { this->value = value; } // TODO check if we need to delete value when given a new one

				void operator=(int value) { if(this->value) delete this->value; this->value = new NumericalInt(value); }
				void operator=(float value) { if(this->value) delete this->value; this->value = new NumericalFloat(value); }

				Ref operator+(const Ref& r) const { return Ref(*r.value + this->value, true); }
				Ref operator-(const Ref& r) const { return Ref(*r.value - this->value, true); }
				Ref operator*(const Ref& r) const { return Ref(*r.value * this->value, true); }
				Ref operator/(const Ref& r) const { return Ref(*r.value / this->value, true); }

				template <typename T>
				Ref operator+(const T& rhs) const { return Ref(*this->value + rhs, true); }
				template <typename T>
				Ref operator-(const T& rhs) const { return Ref(*this->value - rhs, true); }
				template <typename T>
				Ref operator*(const T& rhs) const { return Ref(*this->value * rhs, true); }
				template <typename T>
				Ref operator/(const T& rhs) const { return Ref(*this->value / rhs, true); }

				//TODO make cast type safe
				operator int() const { return static_cast<NumericalInt*>(this->value)->value; }
				operator float() const { return static_cast<NumericalFloat*>(this->value)->value; }
			};

			Ref& operator[](const std::string& name) { if(table.find(name) != table.end()) return table[name]; else { table[name] = Ref(nullptr); table[name].scoped = false; return table[name]; } }

			void clean() { for(auto& ref : table) delete ref.second.value; }

		private:
			std::unordered_map<std::string, Ref> table;
		};

		struct ModelInfo
		{
			float *vertices = nullptr;
			unsigned int *indices = nullptr;

			unsigned int nbVertices = 0;
			unsigned int nbIndices = 0;

			ModelInfo() {}

			ModelInfo(const ModelInfo &rhs)
			{
				unsigned int i = 0;
				this->vertices = new float[rhs.nbVertices];
				for(i = 0; i < rhs.nbVertices; i++)
					this->vertices[i] = rhs.vertices[i];

				this->indices = new unsigned int[rhs.nbIndices];
				for(i = 0; i < rhs.nbIndices; i++)
					this->indices[i] = rhs.indices[i];

				this->nbVertices = rhs.nbVertices;
				this->nbIndices = rhs.nbIndices;
			}

			inline void operator=(const ModelInfo &rhs)
			{
				unsigned int i = 0;

				if(this->vertices != nullptr)
					delete[] this->vertices;
				if(this->indices != nullptr)
					delete[] this->indices;

				this->vertices = new float[rhs.nbVertices];
				for(i = 0; i < rhs.nbVertices; i++)
					this->vertices[i] = rhs.vertices[i];

				this->indices = new unsigned int[rhs.nbIndices];
				for(i = 0; i < rhs.nbIndices; i++)
					this->indices[i] = rhs.indices[i];	

				this->nbVertices = rhs.nbVertices;
				this->nbIndices = rhs.nbIndices;		
			}

			~ModelInfo() { if(vertices != nullptr) delete[] vertices; if(indices!= nullptr) delete[] indices; }
		};

		struct SquareInfo : public ModelInfo
		{
			SquareInfo() : ModelInfo()
			{
				vertices = new float[20];
				//              x                     y                    z                    texpos x             texpos y
				vertices[0] =  -0.5f; vertices[1] =   0.5f; vertices[2] =  0.0f; vertices[3] =  0.0f; vertices[4] =  0.0f;   
				vertices[5] =   0.5f; vertices[6] =   0.5f; vertices[7] =  0.0f; vertices[8] =  1.0f; vertices[9] =  0.0f;
				vertices[10] = -0.5f; vertices[11] = -0.5f; vertices[12] = 0.0f; vertices[13] = 0.0f; vertices[14] = 1.0f;
				vertices[15] =  0.5f; vertices[16] = -0.5f; vertices[17] = 0.0f; vertices[18] = 1.0f; vertices[19] = 1.0f;

				indices = new unsigned int[6];
				indices[0] = 0; indices[1] = 1; indices[2] = 2;
				indices[3] = 1; indices[4] = 2; indices[5] = 3;

				nbVertices = 20;
				nbIndices = 6;
			}

			SquareInfo(const SquareInfo &rhs) : ModelInfo()
			{
				unsigned int i = 0;
				this->vertices = new float[rhs.nbVertices];
				for(i = 0; i < rhs.nbVertices; i++)
					this->vertices[i] = rhs.vertices[i];

				this->indices = new unsigned int[rhs.nbIndices];
				for(i = 0; i < rhs.nbIndices; i++)
					this->indices[i] = rhs.indices[i];

				this->nbVertices = rhs.nbVertices;
				this->nbIndices = rhs.nbIndices;
			}

			inline void operator=(const SquareInfo &rhs)
			{
				unsigned int i = 0;

				if(this->vertices != nullptr)
					delete[] this->vertices;
				if(this->indices != nullptr)
					delete[] this->indices;

				this->vertices = new float[rhs.nbVertices];
				for(i = 0; i < rhs.nbVertices; i++)
					this->vertices[i] = rhs.vertices[i];

				this->indices = new unsigned int[rhs.nbIndices];
				for(i = 0; i < rhs.nbIndices; i++)
					this->indices[i] = rhs.indices[i];	

				this->nbVertices = rhs.nbVertices;
				this->nbIndices = rhs.nbIndices;		
			}
		};

	}
}

#endif
