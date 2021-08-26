#include <QGuiApplication>
#include <iostream>

#include "app.h"

#include <vector>
#include <string>

#include <unordered_map>

enum class VariableType
{
    INT,
    FLOAT,
    LONG,
    STRING,
    VEC3,
    VEC4
};

/*
class Variant
{
public:
    Variant() { new(&s) std::string; }

    inline void operator=(const int& rhs) { type = VariableType::INT; i = rhs; }
    inline void operator=(const float& rhs) { type = VariableType::FLOAT; f = rhs; }
    inline void operator=(const long& rhs) { type = VariableType::LONG; l = rhs; }
    inline void operator=(const std::string& rhs) { type = VariableType::STRING; s = rhs; }
    inline void operator=(const constant::Vector3D& rhs) { type = VariableType::VEC3; vec3 = rhs; }

    inline void print() const { 
        switch (type)
        {
        case VariableType::INT:
            std::cout << i << std::endl;
            break;
        case VariableType::FLOAT:
            std::cout << f << std::endl;
            break;
        case VariableType::LONG:
            std::cout << l << std::endl;
            break;
        case VariableType::STRING:
            std::cout << s << std::endl;
            break;
        case VariableType::VEC3:
            std::cout << "Vector 3" << std::endl;
            break;
        case VariableType::VEC4:
            std::cout << "Vector 4" << std::endl;
            break;
        }
    }

private:
    VariableType type;
    
    union
    {
        int i;
        float f;
        unsigned long long l;
        std::string s;
        constant::Vector3D vec3;
    };
};

class Shareable
{

};

class SharedParameters
{
public:
    SharedParameters() {}
    SharedParameters(void* data, void (*f)(void*)) : data(data), convertFunc(f) {}

    void setData(void* data) { this->data = data; }

    template<typename Type>
    Type* getData() { return data == nullptr ? nullptr : static_cast<Type*>(data); }

    //inline void operator=(const Vector3D &rhs)
    //{
    //    this->x = rhs.x;
    //    this->y = rhs.y;
    //    this->z = rhs.z;
    //}

    //inline SharedParameters operator+(const Vector3D &rhs) const
    //{
    //    Vector3D vec;
    //    vec.x = this->x + rhs.x;
    //    vec.y = this->y + rhs.y;
    //    vec.z = this->z + rhs.z;
    //
    //    return vec;
    //}

    template<typename Type>
    inline SharedParameters& operator+=(const Type &rhs)
    {
        if(data != nullptr)
        {
            *static_cast<Type>(data) += rhs;
        }

        return *this;
    }

    inline bool operator==(const SharedParameters &rhs)
    {
        return data == rhs.data;
    }

    void* data = nullptr;
    void (*convertFunc)(void*) = nullptr;

private:

};

class ReflectTable
{
public:

    template<typename Type>
    void registerVariable(Type data, std::string name) { std::cout << "a" << std::endl; }

    template<typename Type>
    void registerVariable(Type* data, std::string name) { auto convertFunc = [](void* data) { return static_cast<Type*>(data); }; std::cout << "b" << std::endl; }

private:
    std::unordered_map<std::string, SharedParameters> parameterMap;

};
*/

class Variant
{
public:
    template < typename T >
    Variant(T const& t) : ptr(new impl<T>(t)) {}

    struct PlaceHolder
    {
        virtual ~PlaceHolder() {}
        virtual void data() const = 0;
    };

    template < typename T >
    struct impl : PlaceHolder
    {
        impl(T const& t) : val(t) {}
        void data() const { val.data(); }
        T& operator*() const { return val; }
        T val;
    };
    // ptr can now point to the entire family of 
    // struct types generated from impl<T>
    PlaceHolder *ptr;
};

/*
template <typename Type>
struct Numerical
{
    Numerical(Type v) : value(v) {}

    template <typename Other>
    Type operator+(const Numerical<Other>& rhs);

//protected:
    //const Type& getValue() { return value; };
    Type value;
};

struct NumericalInt : public Numerical<int>
{
    NumericalInt(int i) : Numerical(i) {}

    template <typename Other>
    int operator+(const Numerical<Other>& rhs) { return value + rhs.value; }
};

struct NumericalFloat : public Numerical<float>
{
    NumericalFloat(float f) : Numerical(f) {}

    template <typename Other>
    float operator+(const Numerical<Other>& rhs) { return value + rhs.value; }
};
*/

/*

enum class NumericalTags
{
    INT,
    FLOAT
};

union Numerics
{
    int i;
    float f;
};

struct Numerical
{
    Numerical(int i) : tag(NumericalTags::INT) { value.i = i; }
    Numerical(float f) : tag(NumericalTags::FLOAT) { value.f = f; }

    Numerical operator+(const Numerical& rhs)
    {
        switch (tag)
        {
        case NumericalTags::INT:
            switch (rhs.tag)
            {
            case NumericalTags::INT:
                return value.i + rhs.value.i;
                break;
            
            case NumericalTags::FLOAT:
                return value.i + rhs.value.f;
                break;
            }
            break;
        
        case NumericalTags::FLOAT:
            switch (rhs.tag)
            {
            case NumericalTags::INT:
                return value.f + rhs.value.i;
                break;
            
            case NumericalTags::FLOAT:
                return value.f + rhs.value.f;
                break;
            }
            break;
        }

        return 0;
    }

    NumericalTags tag;
    Numerics value;
};

*/

struct Base
{

};

/*
struct Numerical
{
    Base *object;

    //void (Base::*onPressed)(Input*, double, ...) = nullptr;

    //void (*onPressedLambda)(Input*, double) = nullptr;

    //template<typename Func>
    //void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj) { onPressed = static_cast<void (Base::*)(Input*, double, ...)>(f); object = static_cast<Base* >(obj); }
    //void registerFunc(void (*f)(Input*, double)) { onPressedLambda = f; }

    Numerical() {}
    Numerical(Base obj) { object = new Base(obj); }

    Numerical operator+(const Numerical& rhs) { return Numerical(); }

    template <typename Type>
    Numerical operator+(const Type& rhs) { return Numerical(); }

    //template<typename... Args>
    //void call(Input* inputHandler, double deltaTime, Args... args) { if(onPressed != nullptr) (*object.*onPressed)(inputHandler, deltaTime, args...); if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

    virtual ~Numerical() {}
};

template<typename ObjectType>
struct Numerics : public Numerical
{
    ObjectType *object;

	//void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;

    Numerics() {}
    Numerics(ObjectType obj) { std::cout << obj << std::endl; object = new ObjectType(obj); }

    Numerical operator+(const Numerical& rhs) { return (*static_cast<ObjectType*>(object) + *(rhs.object)); }

    template <typename Type>
    Numerical operator+(const Type& rhs) { return (*static_cast<ObjectType*>(object) + rhs); }

    ~Numerics() {}
};
*/

template<typename Type>
Type getType(void *obj) { return static_cast<Type>(obj); }

class BigNum
{
public:
    BigNum() { digits.push_back(0); }
    BigNum(const BigNum& bigNum) : digits(bigNum.digits), sign(bigNum.sign) {}
    BigNum(const long long int& value) { *this = value; }
    BigNum(const std::string& value) { *this = value; }
    BigNum(const char* value) { *this = std::string(value); }

    void operator=(const BigNum& rhs)
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

    BigNum operator+(const BigNum& rhs) const
    {
        if(sign == rhs.sign)
        {
            BigNum result(rhs);
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

    BigNum operator-() const 
    {
		BigNum res = *this;
		res.sign = !sign;
		return res;
	}

    BigNum operator-(const BigNum &rhs) const 
    {
		if (sign == rhs.sign) 
        {
			if (abs() >= rhs.abs()) 
            {
				BigNum res = *this;

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

    void operator+=(const BigNum &rhs) { *this = *this + rhs; }
	void operator-=(const BigNum &rhs) { *this = *this - rhs; }

    bool operator<(const BigNum &rhs) const 
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
 
	bool operator>(const BigNum &rhs) const  { return rhs < *this; }
	bool operator<=(const BigNum &rhs) const { return !(rhs < *this); }
	bool operator>=(const BigNum &rhs) const { return !(*this < rhs); }
	bool operator==(const BigNum &rhs) const { return !(*this < rhs) && !(rhs < *this); }
    bool operator!=(const BigNum &rhs) const { return *this < rhs || rhs < *this; }

    std::string toString() const { std::string res = ""; if(sign == 0) res += "-"; auto it = digits.rbegin(); if(it != digits.rend()) res += std::to_string(*it++); while(it != digits.rend()) { res += numberWithLeadingZero(*it++); } return res; }

    friend std::ostream& operator<<(std::ostream& out, const BigNum& n) { return out << n.toString(); }

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

    BigNum abs() const 
    {
		BigNum res = *this;
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

/*
class ReflectionTable
{
public:
    struct HolderBase
    {
        virtual HolderBase* operator+(HolderBase* rhs) = 0;
        virtual ~HolderBase() { }

        void data;
    };

    template <typename Type>
    struct Holder : public HolderBase
    {
        Holder() {}
        Holder(Type value) : data(value) {}

        Holder<Type>* operator+(HolderBase* rhs) { return new Holder<Type>(data + rhs->data); }

        Type data;
    };

    struct ReflectionItem
    {
        template <typename HolderType>
        void operator=(HolderType data) { holdenItem = new Holder<HolderType>(data); }

        //template <typename RhsType>
        //HolderBase operator+(RhsType rhs) const { return data + rhs.data; }

        //template <typename RhsType>
        //Holder<Type> operator-(RhsType rhs) const { return data - rhs.data; } 

        //template <typename RhsType>
        //Holder<Type>& operator+=(RhsType rhs) { return data += rhs.data; } 

        HolderBase* holdenItem = nullptr;
    };

    ReflectionItem& operator[](const char* name) { return table[std::string(name)]; }
    //const ReflectionItem& operator[](const char* name) const { return table[std::string(name)]; }    

private:
    std::unordered_map<std::string, ReflectionItem> table;
};
*/

#include <functional>

struct Numerical
{
    struct Op
    {
        Op() {}

        virtual void operator()(int val) = 0;
        virtual void operator()(float val) = 0;
        //virtual void operator()(BigNum val) = 0;
        virtual void operator()(std::string val) = 0;

        virtual Numerical* add() = 0;

        std::function<Numerical*()> addFunc;

        virtual ~Op() {}
    };

    Op *op;
    bool empty;

    bool isEmpty() const { return empty; }
    
    template<typename T>
    Numerical* operator+(T value) { (*op)(value) ; return op->add(); }

    virtual Numerical* operator+(Numerical *rhs) = 0;

    //template <typename Other>
    //int operator+(Numerical *rhs) { return value + rhs.value; }

    //std::function<Numerical*(Numerical*)> add() { return [&](Numerical *rhs) { return rhs + value; }; }

    virtual void print() = 0;

    virtual ~Numerical() { delete op; }
};

template<typename Type>
struct Numerics : public Numerical
{
    Type value;

    struct Op : public Numerical::Op
    {
        Type *value;

        Op() {}
        Op(Type *value) : value(value) {}

        template<typename T>
        Op(T val) { (*this)(val); }

        virtual void operator()(int) { addFunc = [=]() { return new Numerics<Type> (); }; }
        virtual void operator()(float) { addFunc = [=]() { return new Numerics<Type> (); }; }
        //virtual void operator()(BigNum) { addFunc = [=]() { return new Numerics<Type> (); }; }
        virtual void operator()(std::string) { addFunc = [=]() { return new Numerics<Type> (); }; }

        virtual Numerical* add() { return addFunc(); } 
    };

    Numerics() { empty = true; }
    Numerics(Type i) : value(i) { empty = false; }

    //Numerical* operator+(Numerical *rhs) { return *rhs + this->value; }
    Numerical* operator+(Numerical *rhs) { return *rhs + this->value; }

    void print() { std::cout << value << std::endl; }
};

struct NumericalFloat : public Numerics<float>
{
    struct Op : public Numerics<float>::Op
    {
        Op(float *value) : Numerics<float>::Op(value) {}

        void operator()(int val) { addFunc = [=]() { return new NumericalFloat (*value + val); }; }
        void operator()(float val) { addFunc = [=]() { return new NumericalFloat (*value + val); }; }

        Numerical* add() { std::cout << "Float add: " << std::endl; return addFunc(); } 

    };

    NumericalFloat(float i) : Numerics<float>(i) { std::cout << "Float" << std::endl; op = new Op(&value); }
};

struct NumericalInt : public Numerics<int>
{
    struct Op : public Numerics<int>::Op
    {
        Op(int *value) : Numerics<int>::Op(value) {}

        //template<typename T>
        //void setValue(T val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }
        void operator()(int val) { addFunc = [=]() { return new NumericalInt (*value + val); }; }
        void operator()(float val) { addFunc = [=]() { return new NumericalFloat (*value + val); }; }

        Numerical* add() { std::cout << "Int add: " << std::endl; return addFunc(); } 

        //std::function<Numerical*()> addFunc;
    };

    //friend void Op::operator()(Op &op, float val) { op.addFunc = [=]() { return new NumericalInt (*op.value + val); }; }

    NumericalInt(int i) : Numerics<int>(i) { std::cout << "Int" << std::endl;  op = new Op(&value); }
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

        void operator=(const Ref& ref) { value = ref.value; }
        void operator=(Numerical *value) { this->value = value; }

        Ref operator+(const Ref &r) const { return Ref(*r.value + this->value, true); }
    };

    Ref& operator[](const std::string &name) { if(table.find(name) != table.end()) return table[name]; else { table[name] = Ref(nullptr); table[name].scoped = false; return table[name]; } }

    ~RefracTable() { for(auto ref : table) delete ref.second.value; }

private:
    std::unordered_map<std::string, Ref> table;
};

/*
struct NumericalInt : public Numerical
{
    int value;

    struct Op : public Numerical::Op
    {
        int *value;

        Op(int *value) : value(value) {}

        template<typename T>
        Op(T val) { setValue(val); }

        //template<typename T>
        //void setValue(T val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }
        void setValue(int val) { addFunc = [=]() { return new NumericalInt(*value + val); }; }
        //void setValue(BigNum val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }
        //void setValue(std::string val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }

        Numerical* add() { std::cout << "Int add: " << std::endl; return addFunc(); } 

        std::function<Numerical*()> addFunc;
    };

    NumericalInt(int i) : value(i) { op = new NumericalInt::Op(&value); }

    void print() { std::cout << value << std::endl; }
};

struct NumericalFloat : public Numerical
{
    float value;

    struct Op : public Numerical::Op
    {
        float *value;

        Op(float *value) : value(value) {}

        template<typename T>
        Op(T val) { setValue(val); }

        //template<typename T>
        //void setValue(T val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }
        void setValue(int val) override { addFunc = [=]() { return new NumericalFloat(*value + val); }; }
        //void setValue(BigNum val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }
        //void setValue(std::string val) { addFunc = [=]() { return new Numerics<Type> (value + val); }; }

        Numerical* add() { std::cout << "Int add: " << std::endl; return addFunc(); } 

        std::function<Numerical*()> addFunc;
    };

    NumericalFloat(float i) : value(i) { op = new NumericalFloat::Op(&value); }

    void print() { std::cout << value << std::endl; }
};
*/

/*
struct NumericalInt : public Numerical
{
    int value;

    class Op : public Numerical::Op
    {
    public:
        Op() {}

        template<typename T>
        Op(T val) { setValue(val); }

        template<typename T>
        void setValue(T val) { addFunc = [=]() { return new NumericalInt (value + val); }; }

        Numerical* add() { std::cout << "Int add: " << std::endl; return addFunc(); } 

        std::function<Numerical*()> addFunc;
    };

    NumericalInt(int i) : value(i) { op = new NumericalInt::Op(); }


    //template<typename T>
    //class Op : public Numerical::Op<T>
    //{
    //public:
    //    Op(T val) : Numerical::Op<T>(val) { std::cout << " Int Init " << std::endl; }
//
    //    Numerical* add() { std::cout << "Int add: " << value + Numerical::Op<T>::val << std::endl; return new NumericalInt(value + Numerical::Op<T>::val); } 
    //};


    void print() { std::cout << value << std::endl; }

    //template <typename Other>
    //int operator+(Numerical *rhs) { return value + rhs.value; }

    //std::function<Numerical*(Numerical*)> add() { return [&](Numerical *rhs) { return rhs + value; }; }

    //template<typename T>
    //std::function<void(T)> getHello() { return [&](T hello) { std::cout << hello + value << std::endl; }; }
};

struct NumericalFloat : public Numerical
{
    float value;

    class Op : public Numerical::Op
    {
    public:
        Op() {}

        template<typename T>
        Op(T val) { setValue(val); }

        template<typename T>
        void setValue(T val) { addFunc = [=]() { return new NumericalFloat (value + val); }; }

        Numerical* add() { std::cout << "Float add: " << std::endl; return addFunc(); } 

        std::function<Numerical*()> addFunc;
    };

    NumericalFloat(float i) : value(i) { op = new NumericalFloat::Op(); }


    //template<typename T>
    //class Op : public Numerical::Op<T>
    //{
    //public:
    //    Op(T val) : Numerical::Op<T>(val) { std::cout << " Float Init " << std::endl;  }
//
    //    Numerical* add() override { std::cout << "Float add: " << value + Numerical::Op<T>::val << std::endl; return new NumericalFloat(value + Numerical::Op<T>::val); } 
    //};


    void print() { std::cout << value << std::endl; }

    //template <typename Other>
    //int operator+(Numerical *rhs) { return value + rhs.value; }

    //std::function<Numerical*(Numerical*)> add() { return [&](Numerical *rhs) { return rhs + value; }; }

    //template<typename T>
    //std::function<void(T)> getHello() { return [&](T hello) { std::cout << hello + value << std::endl; }; }
};
*/

template<typename DataType, typename NextType>
struct Node
{
    DataType data;
    NextType next;

    Node(DataType data, NextType next) : data(data), next(next) { }
};

template<typename DataType, typename NextType>
Node<DataType, NextType>* createNode(DataType data, NextType next) { return new Node<DataType, NextType>(data, next); }

//struct RefracTable
//{
//    
//}

std::function<void(int)> lCreate()
{
    int i = 5;
    return [=](int val) { std::cout << val * i << std::endl; };
}

struct Object
{
    int x; 
    int y;

    int xMove;
    int yMove;

    Object(int x, int y, int xMove, int yMove) : x(x), y(y), xMove(xMove), yMove(yMove) { }

    std::function<void()> onTick = [=]() { x += xMove; y += yMove; };

    template<typename T>
    std::function<void(T)> getHello() { return [&](T hello) { std::cout << hello + x << std::endl; }; }
};

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setSwapInterval(0);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);

    QSurfaceFormat::setDefaultFormat(format);
	QGuiApplication app(argc, argv);

	//GameWindow a;
    //a.resize(640, 480);
    //a.setAnimating(true);
    //a.show();
    
    //QObject::connect(&a, SIGNAL(quitApp()), &app, SLOT(quit()));

	//return app.exec();

    std::vector< std::function<void()> > tickFunction; 

    //auto l = lCreate();
    //l(10);

    Object obj1(10,10,1,1);
    Object obj2(20,10,5,1);

    auto hel = obj1.getHello<int>();
    hel(5);

    //NumericalInt bark = 10;
    //NumericalFloat bork = 15.0f;

    //std::vector<Numerical*> list;
    //list.push_back(&bark);
    //list.push_back(&bork);

    RefracTable table;

    table["Width"] = new NumericalInt(800);
    table["Height"] = new NumericalInt(600);
    table["WidthDelta"] = new NumericalFloat(0.8f);

    auto ref2 = table["Width"];
    ref2.value->print();

    auto ref3 = table["Width"] + table["Height"];
    ref3.value->print();

    auto ref4 = table["Width"] + table["WidthDelta"];
    ref4.value->print();

/*
    Numerical *numerical1 = new NumericalInt(19);
    auto xx = *numerical1 + 1;
    xx->print();

    Numerical *numerical2 = new NumericalInt(4);
    auto xx2 = *numerical2 + 2.5f;
    xx2->print();

    Numerical *numerical3 = new NumericalFloat(13.05);
    auto xx4 = *numerical3 + 2.5f;
    xx4->print();

    auto xx5 = *numerical2 + numerical3;
    xx5->print();

    auto xx3 = *numerical1 + numerical2;
    xx3->print();
*/
    //auto ll = list[0];
    //auto xx = ll + 5;
    //xx->print();
//
    //auto x1 = list[0] + 5;
    //auto x2 = list[1] + 5;
//
    //x1->print();
    //x2->print();

    tickFunction.emplace_back(obj1.onTick);
    tickFunction.emplace_back(obj2.onTick);
    
    for(int i = 0; i < 10; i++)
        for(auto f : tickFunction)
            f();

    //obj1.onTick();
    //std::cout << obj1.x << std::endl;
    //std::cout << obj2.x << std::endl;

    BigNum aaaa(3);

    auto op = &aaaa.operator+=;

    
    std::cout << op << std::endl;

    (aaaa.*op)(5);

    std::cout << aaaa << std::endl;

    float f = 15.5f;
    int i = 2;

    auto node0 = createNode(obj1, nullptr);
    auto node1 = createNode(f, node0);
    auto node2 = createNode(i, node1);

    std::cout << node2->data * node2->next->data << std::endl; 

    return 0;

/*
    int i = 5;
    auto convertFunc = [](void* data) { return static_cast<int*>(data); };

    SharedParameters p(&i, [](void* data) { static_cast<int*>(data); });

    //p.setData(&i);
    auto j = p.getData<int>();

    std::cout << *j << std::endl;

    *j += 5;

    std::cout << i << std::endl;

    ReflectTable table;

    table.registerVariable(i, "gold");
    table.registerVariable(&i, "feur");
*/

    //Variant var1(std::string("Hello World"));

    //auto val = **var1.ptr;

    //std::cout <<  << std::endl;

    //var1 = "Berber";
    //var1.print();

    BigNum firstNb("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    BigNum secondNb(5);

    std::cout << sizeof(int) << "," << sizeof(firstNb) << ", " << sizeof(secondNb) << std::endl;

    auto res = secondNb - firstNb;

    std::cout << res << std::endl;

    //ReflectionTable table;

    //table["Nb1"] = firstNb;

    //Numerical i = 30;
    //Numerical f = 15.2f;

    //Numerics<int> i(5);
    //Numerics<float> f(2.2);

    //auto res1 = i + f;

    //auto res2 = i + f;

    BigNum n1(100050030045);
    BigNum n2(-2345100);
    BigNum n3(10090450);
    BigNum n4(n1);

    BigNum n5(1502);
    BigNum n6(8939);

    std::cout << n5 + n6 << std::endl;

    auto n7 = n6;
    n7 = n7 + n7;
    std::cout << n7 << std::endl;

    std::cout << n1 << std::endl << n2 << std::endl << n3 << std::endl << n4 << std::endl;

    BigNum num = "-10234";
    BigNum num2 = "50234";
    BigNum num3 = "234";
    BigNum num4 = "0000530000";
    auto num5 = num4;

    std::cout << num << ", " << num2 << ", " << num3 << ", " << num4 << ", " << num5 << ", " << num + num2 << std::endl;
    std::cout << num4 + num2 << std::endl;
    std::cout << num4 - num2 << std::endl;
    std::cout << num2 - num4 << std::endl;

    BigNum temp1(1);
    BigNum temp2(1);

    BigNum result;

    std::cout << "[1]: " << temp1 << std::endl << "[2]: " << temp2 << std::endl;

    for(int i = 0; i < 100; i++)
    {
        result = temp1 + temp2;
        temp1 = temp2;
        temp2 = result;

        std::cout << "[" << i + 3 << "]: " << result << std::endl;
    }    

    std::cout << result << std::endl;

    return 0;
}