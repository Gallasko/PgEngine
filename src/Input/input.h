#ifndef INPUT_H
#define INPUT_H

#include <Qt>
#include <QObject>
#include <QPoint>

#include <vector>

namespace pg
{
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

	private:
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

	public:
		Input() : QObject() {}

		Input::InputState registerKeyInput(const Qt::Key& key, const Input::InputState& state);
		Input::InputState registerMouseInput(const Qt::MouseButton& button, const Input::InputState& state);
		Input::InputState registerMouseMove(const QPoint& mousePos, const QPoint& mouseDelta);

		Input::InputState keyState(const Qt::Key& key) const;
		bool isKeyPressed(const Qt::Key& key) const;
		bool isKeyReleased(const Qt::Key& key) const;

		Input::InputState buttonState(const Qt::MouseButton& button) const;
		bool isButtonPressed(const Qt::MouseButton& button) const;
		bool isButtonReleased(const Qt::MouseButton& button) const;

		const QPoint& getMousePos() const;
		const QPoint& getMouseDelta() const;

		void updateInput(double deltaTime);

		void reset();

	private:
		std::vector<Input::KeyInstance> keyContainer;
		std::vector<Input::ButtonInstance> buttonContainer;
		QPoint mousePos;
		QPoint mouseDelta;
		
		double updateTime;

		template <typename Container, typename Value>
		int findInputPos(const Value& value, const Container& container) const;

	};
}

#endif //INPUT_H