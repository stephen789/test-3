/*
 * 
 * @file os_floaterbuildingprefs.h
 * @author simms
 *
 * ----------------------------------------------------------------------------
 * "The DILLIGAFF license" (Do I Look Like I Give A Flying Fuck):
 * Copyright (c) 2012, Simms, Inc.
 * 
 * If you can throw me some shrapnel for beer and coffee https://flattr.com/donation/give/to/simms
 * ----------------------------------------------------------------------------
 */

#include "llviewerprecompiledheaders.h"
#include "os_floaterbuildingprefs.h"
#include "llcolorswatch.h"
#include "llcombobox.h"
#include "llradiogroup.h"
#include "lltexturectrl.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llpanellogin.h"
#include "lltrans.h"

LLFloaterBuildPrefs::LLFloaterBuildPrefs(const LLSD& seed)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_buildprefs.xml");

	//Build -------------------------------------------------------------------------------
	getChild<LLUICtrl>("next_owner_copy")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onCommitCheckBox, this, _1, _2));
	getChild<LLUICtrl>("script_next_owner_copy")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onCommitCheckBox, this, _1, _2));
	getChild<LLUICtrl>("material")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onCommitComboBox, this, _1, _2));
	getChild<LLUICtrl>("combobox shininess")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onCommitComboBox, this, _1, _2));
	getChild<LLTextureCtrl>("texture control")->setDefaultImageAssetID(LLUUID(gSavedSettings.getString("EmeraldBuildPrefs_Texture")));
	getChild<LLUICtrl>("texture control")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onCommitTexturePicker, this, _1));

	mSpoofBuildPrim = getChild<LLCheckBoxCtrl>("spoof_buildprim");
	getChild<LLUICtrl>("spoof_buildprim")->setValue((BOOL)gSavedPerAccountSettings.getBOOL("UseOSBuildSpoofItem"));
	mSpoofBuildPrim->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onCheckSpoofPrim, this, _2));

	requires<LLButton>("OK");
	requires<LLButton>("Cancel");
	requires<LLButton>("Apply");

	getChild<LLUICtrl>("Apply")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onBtnApply, this));
	getChild<LLUICtrl>("Cancel")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onBtnCancel, this));
	getChild<LLUICtrl>("OK")->setCommitCallback(boost::bind(&LLFloaterBuildPrefs::onBtnOK, this));

	refreshValues();
	refresh();
}

LLFloaterBuildPrefs::~LLFloaterBuildPrefs()
{
}

void LLFloaterBuildPrefs::onBtnOK()
{
	// commit any outstanding text entry
	if (hasFocus())
	{
		LLUICtrl* cur_focus = dynamic_cast<LLUICtrl*>(gFocusMgr.getKeyboardFocus());
		if (cur_focus->acceptsTextInput())
		{
			cur_focus->onCommit();
		}
	}

	if (canClose())
	{
		apply();
		close(false);

		gSavedSettings.saveToFile(gSavedSettings.getString("ClientSettingsFile"), TRUE);
	}
	else
	{
		// Show beep, pop up dialog, etc.
		LL_INFOS()  << "Can't close preferences!" << LL_ENDL;
	}

	LLPanelLogin::updateLocationSelectorsVisibility();
}

void LLFloaterBuildPrefs::onBtnApply()
{
	if (hasFocus())
	{
		LLUICtrl* cur_focus = dynamic_cast<LLUICtrl*>(gFocusMgr.getKeyboardFocus());
		if (cur_focus && cur_focus->acceptsTextInput())
		{
			cur_focus->onCommit();
		}
	}
	apply();

	LLPanelLogin::updateLocationSelectorsVisibility();
}

// static 
void LLFloaterBuildPrefs::onBtnCancel()
{
	if (hasFocus())
	{
		LLUICtrl* cur_focus = dynamic_cast<LLUICtrl*>(gFocusMgr.getKeyboardFocus());
		if (cur_focus->acceptsTextInput())
		{
			cur_focus->onCommit();
		}
	}
	close(); // side effect will also cancel any unsaved changes.
}

void LLFloaterBuildPrefs::onCommitCheckBox(LLUICtrl* ctrl, const LLSD& value)
{
	const std::string name = ctrl->getName();
	bool enabled = value.asBoolean();
	if (name == "next_owner_copy")
	{
		if (!enabled) gSavedSettings.setBOOL("ObjectsNextOwnerTransfer", true);
	}
	else if (name == "script_next_owner_copy")
	{
		if (!enabled) gSavedSettings.setBOOL("ScriptsNextOwnerTransfer", true);
	}
}

void LLFloaterBuildPrefs::onCommitComboBox(LLUICtrl* ctrl, const LLSD& value)
{
	gSavedSettings.setString(ctrl->getControlName(), value.asString());
}

void LLFloaterBuildPrefs::onCommitTexturePicker(LLUICtrl* ctrl)
{
	LLTextureCtrl*	image_ctrl = static_cast<LLTextureCtrl*>(ctrl);
	if (image_ctrl)	gSavedSettings.setString("EmeraldBuildPrefs_Texture", image_ctrl->getImageAssetID().asString());
}

//<os>
void LLFloaterBuildPrefs::onCheckSpoofPrim(const LLSD& value)
{
	if (value.asBoolean())
	{
		gSavedPerAccountSettings.setBOOL("UseOSBuildSpoofItem", TRUE);
	}
	else
	{
		gSavedPerAccountSettings.setBOOL("UseOSBuildSpoofItem", FALSE);
	}
}
// </os>

void LLFloaterBuildPrefs::refreshValues()
{
	//Build -------------------------------------------------------------------------------
	mAlpha = gSavedSettings.getF32("EmeraldBuildPrefs_Alpha");
	mColor = gSavedSettings.getColor4("EmeraldBuildPrefs_Color");
	mFullBright = gSavedSettings.getBOOL("EmeraldBuildPrefs_FullBright");
	mGlow = gSavedSettings.getF32("EmeraldBuildPrefs_Glow");
	mItem = gSavedPerAccountSettings.getString("EmeraldBuildPrefs_Item");
	mMaterial = gSavedSettings.getString("BuildPrefs_Material");
	mNextCopy = gSavedSettings.getBOOL("ObjectsNextOwnerCopy");
	mNextMod = gSavedSettings.getBOOL("ObjectsNextOwnerModify");
	mNextTrans = gSavedSettings.getBOOL("ObjectsNextOwnerTransfer");
	mScriptNextCopy = gSavedSettings.getBOOL("ScriptsNextOwnerCopy");
	mScriptNextMod = gSavedSettings.getBOOL("ScriptsNextOwnerModify");
	mScriptNextTrans = gSavedSettings.getBOOL("ScriptsNextOwnerTransfer");
	mShiny = gSavedSettings.getString("EmeraldBuildPrefs_Shiny");
	mTemporary = gSavedSettings.getBOOL("EmeraldBuildPrefs_Temporary");
	mTexture = gSavedSettings.getString("EmeraldBuildPrefs_Texture");
	mPhantom = gSavedSettings.getBOOL("EmeraldBuildPrefs_Phantom");
	mPhysical = gSavedSettings.getBOOL("EmeraldBuildPrefs_Physical");
	mXsize = gSavedSettings.getF32("BuildPrefs_Xsize");
	mYsize = gSavedSettings.getF32("BuildPrefs_Ysize");
	mZsize = gSavedSettings.getF32("BuildPrefs_Zsize");
}

void LLFloaterBuildPrefs::refresh()
{
	//Build -------------------------------------------------------------------------------
	childSetValue("alpha", mAlpha);
	getChild<LLColorSwatchCtrl>("colorswatch")->setOriginal(mColor);
	childSetValue("EmFBToggle", mFullBright);
	childSetValue("glow", mGlow);
	childSetValue("material", mMaterial);
	childSetValue("next_owner_copy", mNextCopy);
	childSetValue("next_owner_modify", mNextMod);
	childSetValue("next_owner_transfer", mNextTrans);
	childSetValue("EmPhantomToggle", mPhantom);
	childSetValue("EmPhysicalToggle", mPhysical);
	childSetValue("combobox shininess", mShiny);
	childSetValue("EmTemporaryToggle", mTemporary);
	childSetValue("texture control", mTexture);
	childSetValue("X size", mXsize);
	childSetValue("Y size", mYsize);
	childSetValue("Z size", mZsize);
}

void LLFloaterBuildPrefs::cancel()
{
	//Build -------------------------------------------------------------------------------
	gSavedSettings.setF32("EmeraldBuildPrefs_Alpha", mAlpha);
	gSavedSettings.setColor4("EmeraldBuildPrefs_Color", mColor);
	gSavedSettings.setBOOL("EmeraldBuildPrefs_FullBright", mFullBright);
	gSavedSettings.setF32("EmeraldBuildPrefs_Glow", mGlow);
	gSavedPerAccountSettings.setString("EmeraldBuildPrefs_Item", mItem);
	gSavedSettings.setString("BuildPrefs_Material", mMaterial);
	gSavedSettings.setBOOL("ObjectsNextOwnerCopy", mNextCopy);
	gSavedSettings.setBOOL("ObjectsNextOwnerModify", mNextMod);
	gSavedSettings.setBOOL("ObjectsNextOwnerTransfer", mNextTrans);
	gSavedSettings.setBOOL("ScriptsNextOwnerCopy", mScriptNextCopy);
	gSavedSettings.setBOOL("ScriptsNextOwnerModify", mScriptNextMod);
	gSavedSettings.setBOOL("ScriptsNextOwnerTransfer", mScriptNextTrans);
	gSavedSettings.setBOOL("EmeraldBuildPrefs_Phantom", mPhantom);
	gSavedSettings.setBOOL("EmeraldBuildPrefs_Physical", mPhysical);
	gSavedSettings.setString("EmeraldBuildPrefs_Shiny", mShiny);
	gSavedSettings.setBOOL("EmeraldBuildPrefs_Temporary", mTemporary);
	gSavedSettings.setString("EmeraldBuildPrefs_Texture", mTexture);
	gSavedSettings.setF32("BuildPrefs_Xsize", mXsize);
	gSavedSettings.setF32("BuildPrefs_Ysize", mYsize);
	gSavedSettings.setF32("BuildPrefs_Zsize", mZsize);
}

void LLFloaterBuildPrefs::apply()
{
	refreshValues();
	refresh();
}