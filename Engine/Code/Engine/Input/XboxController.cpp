#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" )

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/XboxController.hpp"


//---------------------------------------------------------------------------------------------------------
XboxController::XboxController( int controllerID )
	: m_controllerID( controllerID )
{
	for( int buttonStateIndex = 0; buttonStateIndex < NUM_XBOX_BUTTONS; ++buttonStateIndex )
	{
		m_buttonStates[buttonStateIndex] = KeyButtonState();
	}
}


//---------------------------------------------------------------------------------------------------------
XboxController::~XboxController()
{
}


//---------------------------------------------------------------------------------------------------------
const KeyButtonState XboxController::GetButtonState( XboxButtonID buttonID ) const
{
	return m_buttonStates[ buttonID ];
}


//---------------------------------------------------------------------------------------------------------
void XboxController::Update()
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );

	//XINPUT_VIBRATION vibrationInfo;
	//memset( &)
	//vibrationLevel
	//XInputSetState to set vibrations

	if( errorStatus == ERROR_SUCCESS )
	{
		m_isConnected = true;
		XINPUT_GAMEPAD& gamepad = xboxControllerState.Gamepad;
		UpdateJoyStick( m_leftJoyStick, gamepad.sThumbLX, gamepad.sThumbLY );
		UpdateJoyStick( m_rightJoyStick, gamepad.sThumbRX, gamepad.sThumbRY );
		UpdateTrigger( m_leftTriggerValue, gamepad.bLeftTrigger );
		UpdateTrigger( m_rightTriggerValue, gamepad.bRightTrigger );
		UpdateButton( XBOX_BUTTON_ID_A,				gamepad.wButtons, XINPUT_GAMEPAD_A );
		UpdateButton( XBOX_BUTTON_ID_B,				gamepad.wButtons, XINPUT_GAMEPAD_B );
		UpdateButton( XBOX_BUTTON_ID_X,				gamepad.wButtons, XINPUT_GAMEPAD_X );
		UpdateButton( XBOX_BUTTON_ID_Y,				gamepad.wButtons, XINPUT_GAMEPAD_Y );
		UpdateButton( XBOX_BUTTON_ID_BACK,			gamepad.wButtons, XINPUT_GAMEPAD_BACK );
		UpdateButton( XBOX_BUTTON_ID_START,			gamepad.wButtons, XINPUT_GAMEPAD_START );
		UpdateButton( XBOX_BUTTON_ID_LSHOULDER,		gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
		UpdateButton( XBOX_BUTTON_ID_RSHOULDER,		gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
		UpdateButton( XBOX_BUTTON_ID_LTHUMB,		gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB );
		UpdateButton( XBOX_BUTTON_ID_RTHUMB,		gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB );
		UpdateButton( XBOX_BUTTON_ID_DPAD_RIGHT,	gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
		UpdateButton( XBOX_BUTTON_ID_DPAD_UP,		gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButton( XBOX_BUTTON_ID_DPAD_LEFT,		gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButton( XBOX_BUTTON_ID_DPAD_DOWN,		gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
	}
	else if( errorStatus == ERROR_DEVICE_NOT_CONNECTED )
	{
		m_isConnected = false;
		Reset();
	}
}


//---------------------------------------------------------------------------------------------------------
void XboxController::Reset()
{
	m_leftTriggerValue	= 0.f;
	m_rightTriggerValue	= 0.f;

	for( int buttonStateIndex = 0; buttonStateIndex < NUM_XBOX_BUTTONS; ++buttonStateIndex )
	{
		m_buttonStates[ buttonStateIndex ] = KeyButtonState();
	}
}


//---------------------------------------------------------------------------------------------------------
void XboxController::UpdateTrigger( float& triggerValue, unsigned char rawValue )
{
	float correctedValue = RangeMapFloat( 0.f, 255.f, 0.f, 1.f, static_cast< float >( rawValue ) );
	triggerValue = correctedValue;
}


//---------------------------------------------------------------------------------------------------------
void XboxController::UpdateJoyStick( AnalogJoyStick& joysitck, short rawX, short rawY )
{
	float normalizedX = RangeMapFloat( -32768.f, 32767.f, -1.f, 1.f, static_cast< float >( rawX ) );
	float normalizedY = RangeMapFloat( -32768.f, 32767.f, -1.f, 1.f, static_cast< float >( rawY ) );
	joysitck.UpdatePosition( normalizedX, normalizedY );
}


//---------------------------------------------------------------------------------------------------------
void XboxController::UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag )
{
	m_buttonStates[ buttonID ].UpdateStatus( (buttonFlags & buttonFlag) == buttonFlag );
}

