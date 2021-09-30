#include <QGuiApplication>
#include <iostream>

#include "app.h"

#include <vector>
#include <string>

#include <unordered_map>

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

template<typename Type>
Type getType(void *obj) { return static_cast<Type>(obj); }

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

    bool operator<(const BigInt &rhs) const 
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
 
	bool operator>(const BigInt &rhs) const  { return rhs < *this; }
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

#include <functional>

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
        //virtual void operator()(BigInt val) = 0;
        virtual Numerical* operator()(const std::string&, const Operation&) const = 0;

        virtual ~Op() {}
    };

    Op *op;
    bool empty;

    bool isEmpty() const { return empty; }

    virtual Numerical* operator+(Numerical *rhs) const = 0;
    virtual Numerical* operator-(Numerical *rhs) const = 0;
    virtual Numerical* operator*(Numerical *rhs) const = 0;
    virtual Numerical* operator/(Numerical *rhs) const = 0;

    //TODO make it so (*op)(value) return directly the func pointer constantly so the whole class / operator can be made const
    //Something like std::function<Numerical*()> addFunc; (*op)(value, &addFunc, OPERATORENUM::ADD); return addFunc();
    //In (*op)() -> switch (Operator) return [=](){return new Numerical(Operation);}
    
    template<typename T>
    Numerical* operator+(const T& value) const { return (*op)(value, Op::Operation::ADD); }
    template<typename T>
    Numerical* operator-(const T& value) const { return (*op)(value, Op::Operation::SUB); }
    template<typename T>
    Numerical* operator*(const T& value) const { return (*op)(value, Op::Operation::MUL); }
    template<typename T>
    Numerical* operator/(const T& value) const { return (*op)(value, Op::Operation::DIV); }

    virtual Numerical* clone() const = 0;

    virtual void print() const = 0;

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

        virtual ~Op() {}

        virtual Numerical* operator()(const int&, const Operation&) const { return createEmpty(); }
        virtual Numerical* operator()(const float&, const Operation&) const { return createEmpty(); }
        //virtual void operator()(BigInt) { addFunc = [=]() { return new Numerics<Type> (); }; }
        virtual Numerical* operator()(const std::string& , const Operation&) const { return createEmpty(); }

        Numerical* createEmpty() const { return new Numerics<Type> (); }

        template<typename NumericalType, typename ValueType>
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

    void print() const { std::cout << value << std::endl; }
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
        void operator=(Numerical *value) { this->value = value; }

        Ref operator+(const Ref &r) const { return Ref(*r.value + this->value, true); }
        Ref operator-(const Ref &r) const { return Ref(*r.value - this->value, true); }
        Ref operator*(const Ref &r) const { return Ref(*r.value * this->value, true); }
        Ref operator/(const Ref &r) const { return Ref(*r.value / this->value, true); }

        template <typename T>
        Ref operator+(const T& rhs) const { return Ref(*this->value + rhs, true); }
        template <typename T>
        Ref operator-(const T& rhs) const { return Ref(*this->value - rhs, true); }
        template <typename T>
        Ref operator*(const T& rhs) const { return Ref(*this->value * rhs, true); }
        template <typename T>
        Ref operator/(const T& rhs) const { return Ref(*this->value / rhs, true); }
    };

    Ref& operator[](const std::string &name) { if(table.find(name) != table.end()) return table[name]; else { table[name] = Ref(nullptr); table[name].scoped = false; return table[name]; } }

    ~RefracTable() { for(auto ref : table) delete ref.second.value; }

private:
    std::unordered_map<std::string, Ref> table;
};

template<typename DataType, typename NextType>
struct Node
{
    DataType data;
    NextType next;

    Node(DataType data, NextType next) : data(data), next(next) { }
};

template<typename DataType, typename NextType>
Node<DataType, NextType>* createNode(DataType data, NextType next) { return new Node<DataType, NextType>(data, next); }

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

struct Part
{
    unsigned int x; 
    unsigned int y;
};

struct ParticleSubData {};

struct ParticleMoveSubData : public ParticleSubData 
{
    ParticleMoveSubData(unsigned int x, unsigned int y) : xVelocity(x), yVelocity(y) {}
    unsigned int xVelocity;
    unsigned int yVelocity;
};

struct ParticleComponent
{
    unsigned int count;
    std::function<void()> onTick;

    Part *particleList;
    ParticleSubData **particleSubDataList;
};

//#include "constant.h"

//using namespace constant;

//Todo change name
struct UiSize
{
    float pixelSize = 0.0f;
    float scaleValue = 0.0f;
    UiSize *refSize = nullptr;

    UiSize() {}
    UiSize(float pixelSize = 0, float scaleValue = 0, UiSize* ref = nullptr) : pixelSize(pixelSize), scaleValue(scaleValue), refSize(ref) {}
    UiSize(const UiSize& size) : pixelSize(size.pixelSize), scaleValue(size.scaleValue), refSize(size.refSize) {}

    static float returnCurrentSize(const UiSize* size)
    {
        if(size == nullptr)
            return 0;
        else
            return size->pixelSize + returnCurrentSize(size->refSize) * size->scaleValue;
    }

    void operator=(const UiSize& rhs)
    {
        this->pixelSize = rhs.pixelSize;
        this->scaleValue = rhs.scaleValue;
        this->refSize = rhs.refSize;
    }

    void operator=(UiSize *rhs) // Todo add this operator in the official release
    {
        this->pixelSize = 0;
        this->scaleValue = 1.0f;
        this->refSize = rhs;
    }

    void operator=(const int& rhs)
    {
        this->pixelSize = rhs;
        this->scaleValue = 0.0;
        this->refSize = nullptr;
    }

    UiSize operator*(const float& rhs)
    {
        return UiSize(0, rhs, this);
    }

    UiSize operator+(const int& rhs)
    {
        return UiSize(rhs, 1.0f, this); //TODO modify this in the official release
    }

    UiSize operator+(const float& rhs)
    {
        return UiSize(rhs, 1.0f, this); //TODO modify this in the official release
    }

    UiSize operator-(const int& rhs)
    {
        return UiSize(-rhs, 1.0f, this); //TODO modify this in the official release
    }

    float operator-()
    {
        return -UiSize::returnCurrentSize(this);
    }

    template<typename Type>
    friend Type operator+(const Type& lhs, const UiSize& rhs);

    template<typename Type>
    friend Type operator-(const Type& lhs, const UiSize& rhs);

    operator float()
    {
        return UiSize::returnCurrentSize(this);
    }
};

struct PositionalStruct 
{
    struct PositionElement
    {
        PositionElement(void (**updateFunc)(void), UiSize value = UiSize(0, 0, nullptr)) : value(value), updateFunc(updateFunc) {}

        UiSize value = UiSize(0, 0, nullptr);
        
        void (**updateFunc)(void); // TODO is the update function really relevent cause it takes so much space

        void operator=(const UiSize& rhs)
        {
            this->value.pixelSize = rhs.pixelSize;
            this->value.scaleValue = rhs.scaleValue;
            this->value.refSize = rhs.refSize;

            if(*updateFunc != nullptr)
                (*updateFunc)();
        }

        void operator=(const int& rhs)
        {
            this->value.pixelSize = rhs;
            this->value.scaleValue = 0.0;
            this->value.refSize = nullptr;
            
            if(*updateFunc != nullptr)
                (*updateFunc)();
        }

        operator float()
        {
            return UiSize::returnCurrentSize(&value);
        }
    };

    //TODO create ctor dtor copy and swap

    PositionalStruct() {}
    PositionalStruct(const PositionalStruct& other) : updateFunc(other.updateFunc) {
        x = PositionElement(&updateFunc);
        y = PositionElement(&updateFunc);
        z = PositionElement(&updateFunc);

        //TODO clean this copy of UiSize value by copying the assignement operator from UiSize
        x.value = other.x.value;
        y.value = other.y.value;
        z.value = other.z.value;
    }

    PositionElement x = PositionElement(&updateFunc);
    PositionElement y = PositionElement(&updateFunc);
    PositionElement z = PositionElement(&updateFunc);

    void (*updateFunc)(void) = nullptr; // TODO is this necessary? take up 32 out of 80 bytes of data in total across the struct
                                        // if I remove this i can even remove PositionElement alltogether and just use UiSize
};

struct KeyPoint // TODO create copy and swap, and test it on edge case and test for memory leaks
{
    KeyPoint() { }
    KeyPoint(const PositionalStruct& pos, const unsigned int& time) : pos(pos), time(time) {}
    KeyPoint(const UiSize& x, const UiSize& y, const UiSize& z, const unsigned int& time) : time(time) { pos.x = x; pos.y = y; pos.z = z; }

    PositionalStruct pos; 
    unsigned int time;
};

struct Sequence // TODO add interpolation support and default interpolation type
{
    template<typename... Args>
    Sequence(Args... args) { add(args...); }

    //TODO check that the keypoint time is greater than the last when adding in to the Sequence
    // or sort the whole list accordingly so that the last element is the duration of the sequence 
    // maybe replace the vector by a sorted map ? 
    template<typename... Args>
    void add(const KeyPoint& point, Args... args) { keyPoints.emplace_back(point), duration = point.time; add(args...); }

    void add(const KeyPoint& point) { keyPoints.emplace_back(point); duration = point.time; }

    //void add() { } Add this if i want to allow empty sequences

    PositionalStruct getPos(const unsigned int& elapsedTime) {
        auto keyPointsVecSize = keyPoints.size();

        if(keyPointsVecSize <= 0) // the sequence is empty <- currently it should be impossible
            return PositionalStruct();

        // TODO Make sure that current index is always in bound 
        if(keyPoints[currentIndex].time == elapsedTime)
        {
            return keyPoints[currentIndex].pos;
        }

        if(keyPoints[currentIndex].time < elapsedTime)
        {
            auto nextIndex = currentIndex + 1;

            if(nextIndex >= keyPointsVecSize) // this means that currentIndex is the pointing to the last element of the sequence
                return keyPoints[currentIndex].pos;

            if(keyPoints[nextIndex].time > elapsedTime) // TODO here we should interpolate the value between cIndex and nIndex
            {
                //This is the linear interpolation
                float delta = (elapsedTime - keyPoints[currentIndex].time) / static_cast<float>(keyPoints[nextIndex].time - keyPoints[currentIndex].time);
                float dx = keyPoints[nextIndex].pos.x - keyPoints[currentIndex].pos.x;
                float dy = keyPoints[nextIndex].pos.y - keyPoints[currentIndex].pos.y;
                float dz = keyPoints[nextIndex].pos.z - keyPoints[currentIndex].pos.z;

                PositionalStruct pos;

                pos.x = keyPoints[currentIndex].pos.x.value + (dx * delta);
                pos.y = keyPoints[currentIndex].pos.y.value + (dy * delta);
                pos.z = keyPoints[currentIndex].pos.z.value + (dz * delta);

                //TODO add support for other type of interpolation and interpolation registering

                return pos;
            }

            if(keyPoints[nextIndex].time == elapsedTime)
            {
                currentIndex = nextIndex;
                return keyPoints[nextIndex].pos;
            }

            currentIndex = nextIndex;
            return getPos(elapsedTime);
        }
        else if(keyPoints[currentIndex].time > elapsedTime)
        {
            //TODO check if this is really necessary
            if(currentIndex == 0)
                return PositionalStruct();

            currentIndex -= 1;
            return getPos(elapsedTime);
        }
    }

    std::vector<KeyPoint> keyPoints;
    unsigned int currentIndex = 0;
    unsigned int duration = 0;
};

void changed()
{
    std::cout << "changed" << std::endl;
}

class AnimationComponent
{
public:
    static std::vector<AnimationComponent*> runningQueue; 

    template <typename Object>
    AnimationComponent(Object* obj, Sequence aSeq) : pos(&(obj->pos)), animationSequence(aSeq) {}

    inline bool isRunning() const { return running; }

    void start()  { if(pos != nullptr) *pos = animationSequence.getPos(0); elapsedTime = 0; resume(); } 
    void pause()  { running = false; }
    void resume() { if(!running) runningQueue.push_back(this); running = true; }
    void stop()   { running = false; }

    //todo if running is false then cancel the animation and put it in the stopped list
    void tick(const unsigned int& tickRate) { 
        elapsedTime += tickRate;

        if(elapsedTime > animationSequence.duration && !looping) 
            running = false;
        else if (elapsedTime > animationSequence.duration && looping)
            elapsedTime = animationSequence.duration - elapsedTime;

        if(pos != nullptr) 
            *pos = animationSequence.getPos(elapsedTime);
        }

private:
    PositionalStruct *pos;
    Sequence animationSequence;

    unsigned int elapsedTime = 0;
    bool running = false;
    bool looping = false;
};

std::vector<AnimationComponent*> AnimationComponent::runningQueue;

struct UiPosition // Test Object
{
    PositionalStruct pos;
};

int main(int argc, char *argv[])
{
    /*
    QSurfaceFormat format;
    format.setSwapInterval(0);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);

    QSurfaceFormat::setDefaultFormat(format);
	QGuiApplication app(argc, argv);
	GameWindow a;
    a.resize(640, 480);
    a.setAnimating(true);
    a.show();
    
    QObject::connect(&a, SIGNAL(quitApp()), &app, SLOT(quit()));

	return app.exec();
    */

    UiPosition obj;

    {
        PositionalStruct position;

        position.updateFunc = &changed;
        
        UiSize ref = UiSize(10, 0, nullptr); 

        position.x.value = &ref; // <- make position.x.value dependent on ref
        position.y.value = position.x.value * 3.0f;
        position.z.value = position.y.value * 2.0f;

        std::cout << position.x << " " << position.y << " " << position.z << std::endl;
        
        ref = 5.0f;
        //position.x = 5.0f;

        std::cout << position.x << " " << position.y << " " << position.z << std::endl;

        ref = 2.0f;
        //position.x = 2.2f;

        std::cout << position.x << " " << position.y << " " << position.z << std::endl;
        
        obj.pos = position;
        obj.pos.updateFunc = &changed;
    }

    std::cout << obj.pos.x << " " << obj.pos.y << " " << obj.pos.z << std::endl;

    Sequence seq = Sequence( //TODO first param being the origin point of the sequence
                             KeyPoint(obj.pos.x + 1.0f, obj.pos.y + 5.0f, 0.0f, 0 ),
                             KeyPoint(obj.pos.x + 5.0f, obj.pos.y + 3.0f, 0.0f, 40),
                             KeyPoint(obj.pos.x + 3.0f, obj.pos.y - 1.0f, 0.0f, 80) );

    for(auto point : seq.keyPoints)
    {
        std::cout << point.pos.x << " " << point.pos.y << " " << point.pos.z << std::endl;
    }

    UiPosition obj2;
    UiPosition obj3;
    UiPosition obj4;
    UiPosition obj5;

    Sequence seq2 = Sequence(KeyPoint(1.0f,   5.0f, 0.0f, 0 ),
                             KeyPoint(5.0f,   3.0f, 0.0f, 120),
                             KeyPoint(3.0f, - 1.0f, 0.0f, 460) );

    Sequence seq3 = Sequence(KeyPoint( 1.0f,   3.0f, 0.0f, 0 ),
                             KeyPoint(40.0f,   2.0f, 0.0f, 20),
                             KeyPoint(-1.0f, - 1.0f, 0.0f, 40) );

    Sequence seq4 = Sequence(KeyPoint(10.0f,  53.0f, 0.0f, 0 ),
                             KeyPoint(-5.0f, -10.0f, 0.0f, 120),
                             KeyPoint(99.0f, -99.0f, 0.0f, 460) );

    AnimationComponent aComp(&obj, seq); 
    AnimationComponent aComp2(&obj2, seq2); 
    AnimationComponent aComp3(&obj3, seq2); 
    AnimationComponent aComp4(&obj4, seq3); 
    AnimationComponent aComp5(&obj5, seq4); 

    aComp.start();
    aComp2.start();
    aComp3.start();
    aComp4.start();
    aComp5.start();
    std::cout << "[0]: " << obj.pos.x << " " << obj.pos.y << " " << obj.pos.z << std::endl;

    std::vector<UiPosition> objects;
    std::vector<AnimationComponent> animations;
    
    for (int i = 0; i < 10000000; i++)
    {
        auto object = UiPosition();
        objects.emplace_back(object);
        auto comp = AnimationComponent(&object, seq);
        animations.emplace_back(comp);
        comp.start();
    }

    //TODO do this in a event loop
    //for(int i = 0; i < 20; i++)
    //{
    //    aComp.tick(4);
    //    std::cout << "[" << (i + 1) * 4 << "]: Running = " << aComp.isRunning() << " Pos = " << obj.pos.x << " " << obj.pos.y << " " << obj.pos.z << std::endl;
    //}

    std::cout << AnimationComponent::runningQueue.size() << std::endl;

    #include <QDateTime>
    auto startTime = QDateTime::currentMSecsSinceEpoch();

    do // TODO in production remove this to do and just put this 
    {
        for(int i = AnimationComponent::runningQueue.size() - 1; i >= 0; i--) 
        {
            AnimationComponent::runningQueue[i]->tick(10); // tickRate
            
            if(!AnimationComponent::runningQueue[i]->isRunning())
                AnimationComponent::runningQueue.erase(AnimationComponent::runningQueue.begin() + i);
        }

    } while (AnimationComponent::runningQueue.size() > 0);

    auto endTime = QDateTime::currentMSecsSinceEpoch();

    std::cout << endTime - startTime << std::endl;

    //TODO implement a vector of void(*)(unsigned int) to store all the function that need to get called in the tick thread

    std::cout << "[Obj 1]: Running = " << aComp.isRunning() << " Pos = " << obj.pos.x << " " << obj.pos.y << " " << obj.pos.z << std::endl;
    std::cout << "[Obj 2]: Running = " << aComp2.isRunning() << " Pos = " << obj2.pos.x << " " << obj2.pos.y << " " << obj2.pos.z << std::endl;
    std::cout << "[Obj 3]: Running = " << aComp3.isRunning() << " Pos = " << obj3.pos.x << " " << obj3.pos.y << " " << obj3.pos.z << std::endl;
    std::cout << "[Obj 4]: Running = " << aComp4.isRunning() << " Pos = " << obj4.pos.x << " " << obj4.pos.y << " " << obj4.pos.z << std::endl;
    std::cout << "[Obj 5]: Running = " << aComp5.isRunning() << " Pos = " << obj5.pos.x << " " << obj5.pos.y << " " << obj5.pos.z << std::endl;


    return 0;

    /*
    {
        Geometry2D abstractGeometry;
        abstractGeometry.points.push_back(constant::Vector2D(9 ,  3));
        abstractGeometry.points.push_back(constant::Vector2D(11,  1));
        abstractGeometry.points.push_back(constant::Vector2D(10, -2));
        abstractGeometry.points.push_back(constant::Vector2D(9 , -1));
        abstractGeometry.points.push_back(constant::Vector2D(8 ,  1));

        abstractGeometry.trianglesIndices.push_back(TriangleIndices(0, 1, 4));
        abstractGeometry.trianglesIndices.push_back(TriangleIndices(1, 3, 4));
        abstractGeometry.trianglesIndices.push_back(TriangleIndices(1, 2, 3));

        Collidable2D* collideList[5];
        collideList[0] = new Rectangle2D(0, 0, 10, 5);
        collideList[1] = new Rectangle2D(4, 5, 5, 5);
        {
            collideList[2] = new Triangle2D(constant::Vector2D(0, 0), constant::Vector2D(5, 5), constant::Vector2D(10, 0));
            collideList[3] = new Triangle2D(constant::Vector2D(7, 2), constant::Vector2D(7, 6), constant::Vector2D(11, 2));
            collideList[4] = new AbstractShape2D(abstractGeometry);
        }

        if(collideList[0]->collideWith(collideList[1]))
            std::cout << "Collision 0 and 1" << std::endl;

        if(collideList[0]->collideWith(collideList[2]))
            std::cout << "Collision 0 and 2" << std::endl;
        
        if(collideList[0]->collideWith(collideList[3]))
            std::cout << "Collision 0 and 3" << std::endl;
        
        if(collideList[0]->collideWith(collideList[4]))
            std::cout << "Collision 0 and 4" << std::endl;

        if(collideList[1]->collideWith(collideList[2]))
            std::cout << "Collision 1 and 2" << std::endl;

        if(collideList[1]->collideWith(collideList[3]))
            std::cout << "Collision 1 and 3" << std::endl;

        if(collideList[1]->collideWith(collideList[4]))
            std::cout << "Collision 1 and 4" << std::endl;

        if(collideList[2]->collideWith(collideList[3]))
            std::cout << "Collision 2 and 3" << std::endl;

        if(collideList[2]->collideWith(collideList[4]))
            std::cout << "Collision 2 and 4" << std::endl;

        if(collideList[3]->collideWith(collideList[4]))
            std::cout << "Collision 3 and 4" << std::endl;

        for(int i = 0; i < 5; i++)
            delete collideList[i];

        auto centroid = Triangle2D::getCentroid(constant::Vector2D(0, 0), constant::Vector2D(5, 5), constant::Vector2D(10, 0));
        std::cout << centroid.x << " " << centroid.y << std::endl;
        auto centroid2 = Triangle2D::getCentroid(constant::Vector2D(7, 2), constant::Vector2D(7, 6), constant::Vector2D(11, 2));
        std::cout << centroid2.x << " " << centroid2.y << std::endl;
    }


    std::cout << "Succefully destroyed" << std::endl;

    return 0;
    */

    //std::vector< std::function<void()> > tickFunction; 

    //auto l = lCreate();
    //l(10);

/*
    Object obj1(10,10,1,1);
    Object obj2(20,10,5,1);

    Part particle[100];
    ParticleSubData *particleSubData[100];

    for(int i = 0; i < 100; i++)
    {
        particle[i].x = i;
        particle[i].y = i;

        particleSubData[i] = new ParticleMoveSubData(2, 2);
    }

    ParticleComponent pComponent;
    pComponent.count = 100;
    pComponent.particleList = particle;
    pComponent.particleSubDataList = particleSubData;
    pComponent.onTick = [&]() { 
        for(int i = 0; i < pComponent.count; i++) 
        {
            ParticleMoveSubData *pMoveData = static_cast<ParticleMoveSubData*>(pComponent.particleSubDataList[i]);
            pComponent.particleList[i].x += pMoveData->xVelocity;
            pComponent.particleList[i].y += pMoveData->yVelocity;
        }};

    //for(int i = 0; i < 100; i++)
    //    std::cout << pComponent.particleList[i].x << std::endl;

    pComponent.onTick();

    //for(int i = 0; i < 100; i++)
    //    std::cout << pComponent.particleList[i].x << std::endl;


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

    std::cout << sizeof(Numerical) << std::endl;
    std::cout << sizeof(Numerical::Op) << std::endl;
    std::cout << sizeof(NumericalInt) << std::endl;
    std::cout << sizeof(table["Width"]) << std::endl;

    auto ref2 = table["Width"];
    ref2.value->print();

    {
        auto ratio = table["Width"] / table["Height"];
        ratio.value->print();
        table["Ratio"] = ratio;
    }
    
    auto ref3 = table["Ratio"];
    ref3.value->print();

    auto ref4 = table["Width"] * table["WidthDelta"];
    ref4.value->print();

    auto ref5 = table["Width"] - table["Height"];
    ref5.value->print();
*/

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

    //tickFunction.emplace_back(obj1.onTick);
    //tickFunction.emplace_back(obj2.onTick);
    //
    //for(int i = 0; i < 10; i++)
    //    for(auto f : tickFunction)
    //        f();

    //obj1.onTick();
    //std::cout << obj1.x << std::endl;
    //std::cout << obj2.x << std::endl;

/*
    BigInt aaaa(3);
    BigInt bbb(23);

    auto op = &aaaa.operator+=;

    std::cout << op << std::endl;

    (aaaa.*op)(5);

    std::cout << aaaa << std::endl;

    (aaaa.*op)(bbb);

    std::cout << bbb << std::endl;

    float f = 15.5f;
    int i = 2;

    auto node0 = createNode(obj1, nullptr);
    auto node1 = createNode(f, node0);
    auto node2 = createNode(i, node1);

    std::cout << node2->data * node2->next->data << std::endl; 

    return 0;
*/

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

    BigInt firstNb("10050000");
    BigInt secondNb(5);

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

    BigInt n1(100050030045);
    BigInt n2(-2345100);
    BigInt n3(10090450);
    BigInt n4(n1);

    BigInt n5(1502);
    BigInt n6(8939);

    std::cout << n5 + n6 << std::endl;

    auto n7 = n6;
    n7 = n7 + n7;
    std::cout << n7 << std::endl;

    std::cout << n1 << std::endl << n2 << std::endl << n3 << std::endl << n4 << std::endl;

    BigInt num = "-10234";
    BigInt num2 = "50234";
    BigInt num3 = "234";
    BigInt num4 = "0000530000";
    auto num5 = num4;

    std::cout << num << ", " << num2 << ", " << num3 << ", " << num4 << ", " << num5 << ", " << num + num2 << std::endl;
    std::cout << num4 + num2 << std::endl;
    std::cout << num4 - num2 << std::endl;
    std::cout << num2 - num4 << std::endl;

    BigInt temp1(1);
    BigInt temp2(1);

    BigInt result;

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