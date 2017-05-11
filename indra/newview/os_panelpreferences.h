#ifndef OS_PANELPREFERENCES_H
#define OS_PANELPREFERENCES_H

#include "llpanel.h"

class OSPanelPreferences : public LLPanel
{
public:
	OSPanelPreferences();
	~OSPanelPreferences();
	
	void apply();
    void cancel();
    void refresh();
    void refreshValues();
	
private:
	static void onCustomBeam(void* data);
	static void onCustomBeamColor(void* data);
	static void onBeamDelete(void* data);
	static void onBeamColorDelete(void* data);
	static void onComboBoxCommit(LLUICtrl* ctrl, void* userdata);
	static void beamUpdateCall(LLUICtrl* ctrl, void* userdata);
};

#endif // #ifndef
