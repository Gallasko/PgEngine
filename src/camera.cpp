#include "camera.h"

Camera::Camera(QVector3D position, QVector3D up, float yaw, float pitch) : QObject(), Front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(0.5f), MouseSensitivity(0.005f), Zoom(0.5f)
{
    init(position, up, yaw, pitch); 
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : QObject(), Front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(0.5f), MouseSensitivity(0.005f), Zoom(0.5f)
{
    QVector3D position = QVector3D(posX, posY, posZ);
    QVector3D up = QVector3D(upX, upY, upZ);

    init(position, up, yaw, pitch); 
}

void Camera::init(QVector3D position, QVector3D up, float yaw, float pitch)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    Zoom = 30.0f;
    updateCameraVectors();

    initializeOpenGLFunctions(); 
}

void Camera::setSensitivity(float sensitivity)
{
    MouseSensitivity = sensitivity;
}

void Camera::setPos(float x, float y, float z)
{
    Position.setX(x);
    Position.setY(y);
    Position.setZ(z);
}

QMatrix4x4 Camera::GetViewMatrix()
{
    viewMatrix.setToIdentity();
    viewMatrix.lookAt(Position, Position + Front, Up);
    return viewMatrix;
}

void Camera::ProcessKeyboard(constant::Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == constant::Camera_Movement::FORWARD)
        Position += Up * velocity;
    if (direction == constant::Camera_Movement::BACKWARD)
        Position -= Up * velocity;
    if (direction == constant::Camera_Movement::LEFT)
        Position -= Right * velocity;
    if (direction == constant::Camera_Movement::RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, Input *inputHandler, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    if(inputHandler->isButtonPressed(Qt::RightButton))
    {
        //TODO check this value and correct them 
        Position += Up * yoffset / 2.0f;
        Position += Right * xoffset / 2.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= yoffset;

    if (Zoom < 10.0f)
        Zoom = 10.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f; 
}

void Camera::updateKeyboard(Input *inputHandler, double deltaTime...)
{
    if(inputHandler->isKeyPressed(Qt::Key_A))
        ProcessKeyboard(constant::Camera_Movement::LEFT, deltaTime);

    if(inputHandler->isKeyPressed(Qt::Key_W))
        ProcessKeyboard(constant::Camera_Movement::FORWARD, deltaTime);

    if(inputHandler->isKeyPressed(Qt::Key_S))
        ProcessKeyboard(constant::Camera_Movement::BACKWARD, deltaTime);

    if(inputHandler->isKeyPressed(Qt::Key_D))
        ProcessKeyboard(constant::Camera_Movement::RIGHT, deltaTime);
}

void Camera::updateMouse(Input *inputHandler, double deltaTime...)
{
    auto mouseDelta = inputHandler->getMouseDelta();
    float xOffset = mouseDelta.x();
    float yOffset = mouseDelta.y();

    ProcessMouseMovement(xOffset, yOffset, inputHandler);
}

void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    QVector3D front;
    front.setX(cos(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
    front.setY(sin(qDegreesToRadians(Pitch)));
    front.setZ(sin(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
    Front = front.normalized();
    // also re-calculate the Right and Up vector
    Right = QVector3D::crossProduct(Front, WorldUp).normalized();  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up    = QVector3D::crossProduct(Right, Front).normalized();
}
