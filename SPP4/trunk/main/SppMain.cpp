// SppMain.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "SppMain.h"


CSppMain::CSppMain() :m_PropoStarted(false) {}
CSppMain::~CSppMain() {}


bool CSppMain::Start()
{
	// Start only once
	if (m_PropoStarted)
		return false;
	m_PropoStarted = true;

	// Modulation type: PPM/PCM(JR) ....
	struct Modulations *  Modulation = GetModulation(0);


	return true;
}