#include "input.h"

//Helper function 
template <typename Instance>
static inline bool CheckReleased(const Instance &instance)
{
	return instance.state == Input::InputState::KEYRELEASED || instance.state == Input::InputState::MOUSERELEASE;
}

//Input Class
Input::Input() : QObject()
{

}

Input::InputState Input::registerKeyInput(Qt::Key key, Input::InputState state)
{
	auto instance = KeyInstance{key, state};
	auto it = findInputPos(instance, keyContainer);

	switch(state)
	{
		case Input::InputState::KEYPRESSED:
			if (it == -1)
			{
				keyContainer.push_back(instance);
				return Input::InputState::INPUTREGISTERED;
			}
			else
				return Input::InputState::INPUTERROR;
		
		break;

		case Input::InputState::KEYRELEASED:
			if (it != -1)
			{
				keyContainer.at(it).state = Input::InputState::KEYRELEASED;
				return Input::InputState::INPUTREGISTERED;
			}
			else
				return Input::InputState::INPUTERROR;
		break;

		default:
			return Input::InputState::INPUTERROR;
		break;
	}
}

Input::InputState Input::registerMouseInput(Qt::MouseButton button, Input::InputState state)
{
	auto it = findInputPos(button, buttonContainer);
	switch(state)
	{
		case Input::InputState::MOUSEPRESS:
		{
			if (it == -1)
			{
				buttonContainer.push_back(Input::ButtonInstance{button, Input::InputState::MOUSEPRESS});
				return Input::InputState::INPUTREGISTERED;
			}
			else
				return Input::InputState::INPUTERROR;
		}
		break;

		case Input::InputState::MOUSERELEASE:
		{
			if (it != -1)
			{
				buttonContainer.at(it).state = Input::InputState::MOUSERELEASE;
				return Input::InputState::INPUTREGISTERED;
			}
			else
				return Input::InputState::INPUTERROR;
		}
		break;

		default:
		{
			return Input::InputState::INPUTERROR;
		}
		break;
	}
}

Input::InputState Input::registerMouseMove(QPoint mousePos, QPoint mouseDelta)
{
	this->mousePos = mousePos;
	this->mouseDelta += mouseDelta;

	return Input::InputState::INPUTREGISTERED;
}

Input::InputState Input::keyState(Qt::Key key)
{
	auto it = findInputPos(key, keyContainer);
	return (it != -1) ? keyContainer.at(it).state : Input::InputState::INPUTERROR;
}

bool Input::isKeyPressed(Qt::Key key)
{
	return keyState(key) == Input::InputState::KEYPRESSED;
}

bool Input::isKeyReleased(Qt::Key key)
{
	return keyState(key) == Input::InputState::KEYRELEASED;
}

Input::InputState Input::buttonState(Qt::MouseButton button)
{
	auto it = findInputPos(button, buttonContainer);
	return (it != -1) ? buttonContainer.at(it).state : Input::InputState::INPUTERROR;
}

bool Input::isButtonPressed(Qt::MouseButton button)
{
	return buttonState(button) == Input::InputState::MOUSEPRESS;
}

bool Input::isButtonReleased(Qt::MouseButton button)
{
	return buttonState(button) == Input::InputState::MOUSERELEASE;
}

QPoint Input::getMousePos()
{
	return this->mousePos;
}

QPoint Input::getMouseDelta()
{
	return this->mouseDelta;
}

void Input::updateInput(double deltaTime)
{
	this->mouseDelta.setX(0);
	this->mouseDelta.setY(0);

	// Remove old data
	auto removeKey = std::remove_if(keyContainer.begin(), keyContainer.end(), &CheckReleased<KeyInstance>);
	keyContainer.erase(removeKey, keyContainer.end());

	auto removeButton = std::remove_if(buttonContainer.begin(), buttonContainer.end(), &CheckReleased<ButtonInstance>);
	buttonContainer.erase(removeButton, buttonContainer.end());
}

void Input::reset()
{
	std::vector<Input::KeyInstance>().swap(keyContainer);
	std::vector<Input::ButtonInstance>().swap(buttonContainer);
}

template <typename Container, typename Value>
int Input::findInputPos(Value value, Container container)
{
	for (long long unsigned int i = 0; i < container.size(); i++)
		if(value == container.at(i).value)
			return i;

	return -1;
}



