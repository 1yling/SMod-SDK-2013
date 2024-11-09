//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1

ConVar hud_disable_health("hud_disable_health", "0");

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(CHudHealth, CHudNumericDisplay);

public:
	CHudHealth(const char *pElementName);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();
	void MsgFunc_Damage(bf_read &msg);

	virtual void Paint(void);
	virtual void ApplySchemeSettings(IScheme *scheme);
private:
	// old variables
	int		m_iHealth;

	int		m_bitsDamage;

	CHudTexture *m_pHealthIcon;

	CPanelAnimationVarAliasType(float, icon_xpos, "icon_xpos", "5", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_ypos, "icon_ypos", "-3", "proportional_float");

	CPanelAnimationVar(Color, m_LowHealthColor, "LowHealthColor", "255 0 0 0");

	float icon_tall;
	float icon_wide;

};

DECLARE_HUDELEMENT(CHudHealth);
DECLARE_HUD_MESSAGE(CHudHealth, Damage);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth(const char *pElementName) : CHudElement(pElementName), CHudNumericDisplay(NULL, "HudHealth")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE(CHudHealth, Damage);
	Reset();
}

void CHudHealth::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	if (!m_pHealthIcon)
	{
		m_pHealthIcon = gHUD.GetIcon("item_healthkit");
	}

	if (m_pHealthIcon)
	{

		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float)m_pHealthIcon->Height();
		icon_wide = (scale) * (float)m_pHealthIcon->Width();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	m_iHealth = INIT_HEALTH;
	m_bitsDamage = 0;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_HEALTH");

	if (tempString)
	{
		//SetLabelText(tempString);
	}
	else
	{
		//SetLabelText(L"HEALTH");
	}
	SetDisplayValue(m_iHealth);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		// Never below zero
		newHealth = MAX(local->GetHealth(), 0);
	}

	// Only update the fade if we've changed health
	if (newHealth == m_iHealth)
	{
		return;
	}

	m_iHealth = newHealth;

	if (m_iHealth >= 20)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
	}
	else if (m_iHealth > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}

	SetDisplayValue(m_iHealth);
}

void CHudHealth::Paint(void)
{
	if (hud_disable_health.GetBool())
		return;

	if (m_pHealthIcon)
	{
		m_pHealthIcon->DrawSelf(icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor());
	}

	//draw the health icon
	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_Damage(bf_read &msg)
{

	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if (damageTaken > 0 || armor > 0)
	{
		if (damageTaken > 0)
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
		}
	}
}