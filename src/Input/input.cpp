#include "input.h"

#include <algorithm>

#include "../logger.h"

namespace pg
{
	namespace
	{
		const char * DOM = "Input";

		//Helper function 
		template <typename Instance>
		static inline bool CheckReleased(const Instance& instance)
		{
			return instance.state == Input::InputState::KEYRELEASED || instance.state == Input::InputState::MOUSERELEASE;
		}
	}

	Input::InputState Input::registerKeyInput(const Qt::Key& key, const Input::InputState& state)
	{
		LOG_THIS_MEMBER(DOM);

		const auto instance = KeyInstance{key, state};
		const auto it = findInputPos(instance, keyContainer);

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

	Input::InputState Input::registerMouseInput(const Qt::MouseButton& button, const Input::InputState& state)
	{
		LOG_THIS_MEMBER(DOM);

		const auto it = findInputPos(button, buttonContainer);
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

	Input::InputState Input::registerMouseMove(const QPoint& mousePos, const QPoint& mouseDelta)
	{
		LOG_THIS_MEMBER(DOM);

		this->mousePos = mousePos;
		this->mouseDelta += mouseDelta;

		return Input::InputState::INPUTREGISTERED;
	}

	Input::InputState Input::keyState(const Qt::Key& key) const
	{
		LOG_THIS_MEMBER(DOM);

		const auto it = findInputPos(key, keyContainer);
		return (it != -1) ? keyContainer.at(it).state : Input::InputState::INPUTERROR;
	}

	bool Input::isKeyPressed(const Qt::Key& key) const
	{
		LOG_THIS_MEMBER(DOM);

		return keyState(key) == Input::InputState::KEYPRESSED;
	}

	bool Input::isKeyReleased(const Qt::Key& key) const
	{
		LOG_THIS_MEMBER(DOM);

		return keyState(key) == Input::InputState::KEYRELEASED;
	}

	Input::InputState Input::buttonState(const Qt::MouseButton& button) const
	{
		LOG_THIS_MEMBER(DOM);

		const auto it = findInputPos(button, buttonContainer);
		return (it != -1) ? buttonContainer.at(it).state : Input::InputState::INPUTERROR;
	}

	bool Input::isButtonPressed(const Qt::MouseButton& button) const
	{
		LOG_THIS_MEMBER(DOM);

		return buttonState(button) == Input::InputState::MOUSEPRESS;
	}

	bool Input::isButtonReleased(const Qt::MouseButton& button) const
	{
		LOG_THIS_MEMBER(DOM);

		return buttonState(button) == Input::InputState::MOUSERELEASE;
	}

	const QPoint& Input::getMousePos() const
	{
		LOG_THIS_MEMBER(DOM);

		return this->mousePos;
	}

	const QPoint& Input::getMouseDelta() const
	{
		LOG_THIS_MEMBER(DOM);

		return this->mouseDelta;
	}

	void Input::updateInput(double)
	{
		this->mouseDelta.setX(0);
		this->mouseDelta.setY(0);

		// Remove old data
		const auto removeKey = std::remove_if(keyContainer.begin(), keyContainer.end(), &CheckReleased<KeyInstance>);
		keyContainer.erase(removeKey, keyContainer.end());

		const auto removeButton = std::remove_if(buttonContainer.begin(), buttonContainer.end(), &CheckReleased<ButtonInstance>);
		buttonContainer.erase(removeButton, buttonContainer.end());
	}

	void Input::reset()
	{
		std::vector<Input::KeyInstance>().swap(keyContainer);
		std::vector<Input::ButtonInstance>().swap(buttonContainer);
	}

	template <typename Container, typename Value>
	int Input::findInputPos(const Value& value, const Container& container) const
	{
		for (long long unsigned int i = 0; i < container.size(); i++)
			if(value == container.at(i).value)
				return i;

		return -1;
	}

}