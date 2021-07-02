#ifndef CAMERA_H
#define CAMERA_H

#include <QtMath>
#include <QVector3D>
#include <QMatrix4x4>

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

#include "constant.h"
#include "Input/input.h"
#include "Engine/basesystem.h"

class Camera : public QObject, protected QOpenGLFunctions, public Base
{
    Q_OBJECT
public:
    // camera Attributes
    QVector3D Position;
    QVector3D Front;
    QVector3D Up;
    QVector3D Right;
    QVector3D WorldUp;

    // euler Angles
    float Yaw;
    float Pitch;
    
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);

    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    //Init the camera positions and VAOs
    void init(QVector3D position, QVector3D up, float yaw, float pitch);

    void setSensitivity(float sensitivity);

    void setPos(float x, float y, float z);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    QMatrix4x4 GetViewMatrix();

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(constant::Camera_Movement direction, float deltaTime);

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, Input *inputHandler, GLboolean constrainPitch = true);

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

public slots:
    void updateKeyboard(Input *inputHandler, double deltaTime);
    void updateMouse(Input *inputHandler, double deltaTime);

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();

    QMatrix4x4 viewMatrix;
};

#endif