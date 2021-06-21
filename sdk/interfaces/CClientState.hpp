#pragma once

#include <cstdint>

#include "..\math\Vector.hpp"
#include "..\misc\bf_write.h"

#define GenDefineVFunc(...) ( this, __VA_ARGS__ ); }
#define VFUNC( index, func, sig ) auto func { return call_virtual< sig >( this, index ) GenDefineVFunc

class CClockDriftMgr {
public:
	float m_ClockOffsets[ 17 ];   //0x0000
	uint32_t m_iCurClockOffset; //0x0044
	uint32_t m_nServerTick;     //0x0048
	uint32_t m_nClientTick;     //0x004C
}; //Size: 0x0050

class INetChannel {
public:
	char pad_0x0000[ 0x18 ]; //0x0000
	__int32 m_nOutSequenceNr; //0x0018 
	__int32 m_nInSequenceNr; //0x001C 
	__int32 m_nOutSequenceNrAck; //0x0020 
	__int32 m_nOutReliableState; //0x0024 
	__int32 m_nInReliableState; //0x0028 
	__int32 m_nChokedPackets; //0x002C 

	VFUNC( 49, Transmit( bool onlyreliable ), bool( __thiscall* )( void*, bool ) )( onlyreliable )
};//Size=0x4294

// padding macro, please use, counts pads in class automaticly
#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define PAD( size ) uint8_t MACRO_CONCAT( _pad, __COUNTER__ )[ size ];
class CClientState {
private:
	PAD(0x9C);                                // 0x0000

public:
	INetChannel* m_net_channel;				// 0x009C

private:
	PAD(0x70);                                // 0x00A0

public:
	int				m_next_message_time;		// 0x0110

public:
	float           m_net_cmd_time;             // 0x0114
	uint32_t        m_server_count;             // 0x0118
private:
	PAD(0x4C);								// 0x011C

public:
	int             m_unk;                      // 0x0168
	int             m_server_tick;              // 0x016C
	int             m_client_tick;              // 0x0170
	int             m_delta_tick;               // 0x0174

private:
	PAD(0x4B30);                              // 0x0178

public:
	float           m_frame_time;               // 0x4CA8
	int             m_last_outgoing_command;    // 0x4CAC
	int             m_choked_commands;          // 0x4CB0
	int             m_last_command_ack;         // 0x4CB4
	PAD(0x134);                               // 0x4CB8
	void*			m_events;					// 0x4DEC

	enum indices : size_t {
		TEMPENTITIES = 36,
	};
};