#pragma once

#define OFFSET_ENTITYLIST		    0x641a328 //cl_entitylist
#define OFFSET_LOCAL_ENT			0x26abef8 //LocalPlayer  
#define OFFSET_LEVELNAME            0x1e56864 //
#define OFFSET_INATTACK				0x3f330a8 //+attack

#define OFFSET_NAME_INDEX			0x38
#define OFFSET_TEAM					0x0324 //m_iTeamNum
#define OFFSET_HEALTH				0x0314 //m_iHealth
#define OFFSET_SHIELD				0x0190  //m_shieldHealth
#define OFFSET_SHIELDMAX			0x194 ////m_shieldHealthMax
#define OFFSET_NAME					0x0471 //[RecvTable.DT_BaseEntity] m_iName
#define OFFSET_VISIBLE_TIME         0x1a74 //lastVisibleTime

#define OFFSET_NAMELIST				0x8e999b0 //NameList

#define OFFSET_LIFE_STATE			0x0688 //m_lifeState
#define OFFSET_BLEED_OUT_STATE		0x27c8 //m_bleedoutState
#define OFFSET_studioHdr 0x1000 //studioHdr

#define OFFSET_CURRENT_WEAPON		0x19d4 //m_latestPrimaryWeapons
#define OFFSET_CURRENT_WEAPONID		0x1890 //m_weaponNameIndex
#define OFFSET_ORIGIN				0x16c  //[DataMap.CBaseViewModel]m_vecAbsOrigin
#define OFFSET_BONES				0xdb8 + 0x48 ////m_nForceBone
#define OFFSET_AIMPUNCH				0x2500 //m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
#define OFFSET_CAMERAPOS			0x1fac //CPlayer!camera_origin
#define OFFSET_VIEWANGLES			0x25e8 //m_ammoPoolCapacity

#define OFFSET_MATRIX				0x11a390  //ViewMatrix
#define OFFSET_RENDER				0x3f30768 //ViewRender
#define OFFSET_ITEMID				0x15e4  //m_customScriptInt
#define m_currentFramePlayertimeBase 0x2150 //m_currentFramePlayer.timeBase
#define OFFSET_SIGNIFIER			0x0468		// m_iSignifierName
#define OFFSET_ABS_VELOCITY			0x0160		// C_Player : m_vecAbsVelocity