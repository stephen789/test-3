#include "llviewerprecompiledheaders.h"

#include "os_panelpreferences.h"

#include "llpanel.h"
#include "lluictrl.h"
#include "llcombobox.h"
#include "llcheckboxctrl.h"
#include "lllineeditor.h"
#include "lluictrlfactory.h"
#include "llslider.h"
#include "llsliderctrl.h"
#include "os_beamcolormapfloater.h"
#include "os_beammapfloater.h"
#include "os_beammaps.h"
//#include "os_beamscolors.h"

OSPanelPreferences::OSPanelPreferences()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "os_panel_preferences.xml");
	
	LLSliderCtrl* mShapeScaleSlider = getChild<LLSliderCtrl>("BeamShapeScale");
				  mShapeScaleSlider->setCommitCallback(boost::bind(&OSPanelPreferences::beamUpdateCall, this, _1));

	LLSliderCtrl* mBeamsPerSecondSlider = getChild<LLSliderCtrl>("MaxBeamsPerSecond");
				  mBeamsPerSecondSlider->setCommitCallback(boost::bind(&OSPanelPreferences::beamUpdateCall, this, _1));

	getChild<LLComboBox>("BeamShape_combo")->setCommitCallback(boost::bind(&OSPanelPreferences::onComboBoxCommit, this, _1));

	getChild<LLComboBox>("BeamColor_combo")->setCommitCallback(boost::bind(&OSPanelPreferences::onComboBoxCommit, this, _1));

	getChild<LLButton>("BeamColor_new")->setClickedCallback(onCustomBeamColor, this);
	getChild<LLButton>("BeamColor_refresh")->setClickedCallback(boost::bind(&OSPanelPreferences::refresh,this));
	getChild<LLButton>("BeamColor_delete")->setClickedCallback(onBeamColorDelete,this);
			
	getChild<LLButton>("custom_beam_btn")->setClickedCallback(onCustomBeam, this);
	getChild<LLButton>("refresh_beams")->setClickedCallback(boost::bind(&OSPanelPreferences::refresh,this));
	getChild<LLButton>("delete_beam")->setClickedCallback(onBeamDelete,this);
	
	refreshValues();
    refresh();
}

OSPanelPreferences::~OSPanelPreferences()
{
}

// Store current settings for cancel
void OSPanelPreferences::refreshValues()
{

}

// Update controls based on current settings
void OSPanelPreferences::refresh()
{
	LLComboBox* comboBox = getChild<LLComboBox>("BeamShape_combo");
	
	if(comboBox != NULL) 
	{
		comboBox->removeall();
		comboBox->add("===OFF===");
		std::vector<std::string> names = gLggBeamMaps.getFileNames();
		for(int i=0; i<(int)names.size(); i++) 
		{
			comboBox->add(names[i]);
		}
		comboBox->setSimple(gSavedSettings.getString("BeamShape"));
	}

	comboBox = getChild<LLComboBox>("BeamColor_combo");
	if(comboBox != NULL) 
	{
		comboBox->removeall();
		comboBox->add("===OFF===");
		std::vector<std::string> names = gLggBeamMaps.getColorsFileNames();
		for(int i=0; i<(int)names.size(); i++) 
		{
			comboBox->add(names[i]);
		}
		comboBox->setSimple(gSavedSettings.getString("BeamColorFile"));
	}

}

// Reset settings to local copy
void OSPanelPreferences::cancel()
{

}

// Update local copy so cancel has no effect
void OSPanelPreferences::apply()
{
    //refreshValues();
    refresh();
}

void OSPanelPreferences::onCustomBeam(void* data)
{
	//LLPrefsAscentVan* self =(LLPrefsAscentVan*)data;
	LggBeamMap::show(true, data);

}
void OSPanelPreferences::onCustomBeamColor(void* data)
{
	LggBeamColorMap::show(true,data);
}

void OSPanelPreferences::beamUpdateCall(LLUICtrl* crtl, void* userdata)
{
	gLggBeamMaps.forceUpdate();
}

void OSPanelPreferences::onComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;
	if(box)
	{
		gSavedSettings.setString(box->getControlName(),box->getValue().asString());
	}
	
}
void OSPanelPreferences::onBeamDelete(void* data)
{
	OSPanelPreferences* self = (OSPanelPreferences*)data;
	
	LLComboBox* comboBox = self->getChild<LLComboBox>("PhoenixBeamShape_combo");

	if(comboBox != NULL) 
	{
		std::string filename = comboBox->getValue().asString()+".xml";
		std::string path_name1(gDirUtilp->getExpandedFilename( LL_PATH_APP_SETTINGS , "beams", filename));
		std::string path_name2(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "beams", filename));
		
		if(gDirUtilp->fileExists(path_name1))
		{
			LLFile::remove(path_name1);
			gSavedSettings.setString("OSBeamShape","===OFF===");
		}
		if(gDirUtilp->fileExists(path_name2))
		{
			LLFile::remove(path_name2);
			gSavedSettings.setString("OSBeamShape","===OFF===");
		}
	}
	self->refresh();
}
void OSPanelPreferences::onBeamColorDelete(void* data)
{
	OSPanelPreferences* self = (OSPanelPreferences*)data;

	LLComboBox* comboBox = self->getChild<LLComboBox>("BeamColor_combo");

	if(comboBox != NULL) 
	{
		std::string filename = comboBox->getValue().asString()+".xml";
		std::string path_name1(gDirUtilp->getExpandedFilename( LL_PATH_APP_SETTINGS , "beamsColors", filename));
		std::string path_name2(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "beamsColors", filename));

		if(gDirUtilp->fileExists(path_name1))
		{
			LLFile::remove(path_name1);
			gSavedSettings.setString("OSBeamColorFile","===OFF===");
		}
		if(gDirUtilp->fileExists(path_name2))
		{
			LLFile::remove(path_name2);
			gSavedSettings.setString("OSBeamColorFile","===OFF===");
		}
	}
	self->refresh();
}
