#include "Camera.h"

//Camera constructor initializer, sets the position and coordinates of the camera.
//no params, 
//return void
Camera::Camera()
{
	_cameraType = AIRCRAFT;

	_pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	_right = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	_up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	_look = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
}


//Camera copy constructor,
//param camera type - copies the camera type and sets its init positions
//return void
Camera::Camera(CameraType cameraType)
{
	_cameraType = cameraType;

	_pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	_right = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	_up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	_look = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
}

Camera::~Camera()
{

}

//gets the position of the camera,
// param d3dxvector3 pointer to pos, sets that pos to this camera pos.
//return void;
void Camera::getPosition(D3DXVECTOR3* pos)
{
	*pos = _pos;
}


//Sets the position of this camera by parameter pos
//param d3dxvector3 - position passed in to set this camera
//return void;
void Camera::setPosition(D3DXVECTOR3* pos)
{
	_pos = *pos;
}


//gets the right portion of the camera
//param d3dxvector3 vector of the right section
//return void;
void Camera::getRight(D3DXVECTOR3* right)
{
	*right = _right;
}

//gets the up portion of the camera
//param d3dxvector 3 vector of the up portion
//return void;
void Camera::getUp(D3DXVECTOR3* up)
{
	*up = _up;
}

//gets the look vector of the camera
//param d3dxvector3 : look vector
//return void;
void Camera::getLook(D3DXVECTOR3* look)
{
	*look = _look;
}

//sets the look vector of the camera
//param d3dxvector 3 look;
//return void;
void Camera::setLook(D3DXVECTOR3* look)
{
	_look = *look;
}

//moves the camera based on type. if 'land camera' restricted to xz plane.
//param float units - how much in units to move the camera dir
//return void;
void Camera::walk(float units)
{
	// move only on xz plane for land object

	if (_cameraType == LANDOBJECT)
		_pos += D3DXVECTOR3(_look.x, 0.0f, _look.z) * units;

	if (_cameraType == AIRCRAFT)
		_pos += _look * units;
}

//strafe the camera, restricted to xz plane depending on camera type.
//param float units - amount in units to strafe the camera
//return void
void Camera::strafe(float units)
{
	// move only on xz plane for land object
	if (_cameraType == LANDOBJECT)
		_pos += D3DXVECTOR3(_right.x, 0.0f, _right.z) * units;

	if (_cameraType == AIRCRAFT)
		_pos += _right * units;
}

//moves the camera vertically (y-axis) based on camera type
//param float units - how much to move the camera by.
//return void;
void Camera::fly(float units)
{
	// move only on y-axis for land object
	if (_cameraType == LANDOBJECT)
		_pos.y += units;

	if (_cameraType == AIRCRAFT)
		_pos += _up * units;
}

//changes the pitch angle of the camera
//param float angle - angle to multiply matrix by arbitrary angle
//return void;
void Camera::pitch(float angle)
{
	D3DXMATRIX T;
	D3DXMatrixRotationAxis(&T, &_right, angle);

	// rotate _up and _look around _right vector
	D3DXVec3TransformCoord(&_up, &_up, &T);
	D3DXVec3TransformCoord(&_look, &_look, &T);
}

void Camera::yaw(float angle)
{
	D3DXMATRIX T;

	// rotate around world y (0, 1, 0) always for land object
	if (_cameraType == LANDOBJECT)
		D3DXMatrixRotationY(&T, angle);

	// rotate around own up vector for aircraft
	if (_cameraType == AIRCRAFT)
		D3DXMatrixRotationAxis(&T, &_up, angle);

	// rotate _right and _look around _up or y-axis
	D3DXVec3TransformCoord(&_right, &_right, &T);
	D3DXVec3TransformCoord(&_look, &_look, &T);
}

void Camera::roll(float angle)
{
	// only roll for aircraft type
	if (_cameraType == AIRCRAFT)
	{
		D3DXMATRIX T;
		D3DXMatrixRotationAxis(&T, &_look, angle);

		// rotate _up and _right around _look vector
		D3DXVec3TransformCoord(&_right, &_right, &T);
		D3DXVec3TransformCoord(&_up, &_up, &T);
	}
}

void Camera::getViewMatrix(D3DXMATRIX* V)
{
	// Keep camera's axes orthogonal to eachother
	D3DXVec3Normalize(&_look, &_look);

	D3DXVec3Cross(&_up, &_look, &_right);
	D3DXVec3Normalize(&_up, &_up);

	D3DXVec3Cross(&_right, &_up, &_look);
	D3DXVec3Normalize(&_right, &_right);

	// Build the view matrix:
	float x = -D3DXVec3Dot(&_right, &_pos);
	float y = -D3DXVec3Dot(&_up, &_pos);
	float z = -D3DXVec3Dot(&_look, &_pos);

	(*V)(0, 0) = _right.x; (*V)(0, 1) = _up.x; (*V)(0, 2) = _look.x; (*V)(0, 3) = 0.0f;
	(*V)(1, 0) = _right.y; (*V)(1, 1) = _up.y; (*V)(1, 2) = _look.y; (*V)(1, 3) = 0.0f;
	(*V)(2, 0) = _right.z; (*V)(2, 1) = _up.z; (*V)(2, 2) = _look.z; (*V)(2, 3) = 0.0f;
	(*V)(3, 0) = x;        (*V)(3, 1) = y;     (*V)(3, 2) = z;       (*V)(3, 3) = 1.0f;
}

void Camera::setCameraType(CameraType cameraType)
{
	_cameraType = cameraType;
}
