#ifndef INPUT_H
#define INPUT_H

#include <Qt>
#include <QObject>
#include <QPoint>

#include <vector>
#include <algorithm>

class Input : public QObject
{
	Q_OBJECT
public:
	enum class InputState
	{
		INPUTERROR,
		INPUTREGISTERED,
		KEYPRESSED,
		KEYRELEASED,
		MOUSEPRESS,
		MOUSERELEASE
	};

	template <typename T>
	struct InputInstance
	{
		T value;
		InputState state;

		inline bool operator==(const InputInstance &rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator==(const T &rhs) const
		{
			return this->value == rhs;
		}

	};

	typedef InputInstance<Qt::Key> KeyInstance;
	typedef InputInstance<Qt::MouseButton> ButtonInstance;

	Input();

	Input::InputState registerKeyInput(Qt::Key key, Input::InputState state);
	Input::InputState registerMouseInput(Qt::MouseButton button, Input::InputState state);
	Input::InputState registerMouseMove(QPoint mousePos, QPoint mouseDelta);

	Input::InputState keyState(Qt::Key key);
	bool isKeyPressed(Qt::Key key);
	bool isKeyReleased(Qt::Key key);

	Input::InputState buttonState(Qt::MouseButton button);
	bool isButtonPressed(Qt::MouseButton button);
	bool isButtonReleased(Qt::MouseButton button);

	QPoint getMousePos();
	QPoint getMouseDelta();

	void updateInput(double deltaTime);

	void reset();

signals:
	void updatedKeyInput(Input *inputHandler, double deltaTime);
	void updatedMouseInput(Input *inputHandler, double deltaTime);

private:
	std::vector<Input::KeyInstance> keyContainer;
	std::vector<Input::ButtonInstance> buttonContainer;
	QPoint mousePos;
	QPoint mouseDelta;
	
	double updateTime;

	template <typename Container, typename Value>
	int findInputPos(Value value, Container container);

};

#endif //INPUT_H